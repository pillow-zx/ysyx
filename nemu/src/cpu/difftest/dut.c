/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <dlfcn.h>

#include <isa.h>
#include <cpu/cpu.h>
#include <memory/paddr.h>
#include <utils.h>
#include <difftest-def.h>

void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *dut, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(uint64_t NO) = NULL;

#ifdef CONFIG_DIFFTEST

static bool is_skip_ref = false;
static int skip_dut_nr_inst = 0;

// this is used to let ref skip instructions which
// can not produce consistent behavior with NEMU
void difftest_skip_ref() {
    is_skip_ref = true;
    // If such an instruction is one of the instruction packing in QEMU
    // (see below), we end the process of catching up with QEMU's pc to
    // keep the consistent behavior in our best.
    // Note that this is still not perfect: if the packed instructions
    // already write some memory, and the incoming instruction in NEMU
    // will load that memory, we will encounter false negative. But such
    // situation is infrequent.
    skip_dut_nr_inst = 0;
}

// this is used to deal with instruction packing in QEMU.
// Sometimes letting QEMU step once will execute multiple instructions.
// We should skip checking until NEMU's pc catches up with QEMU's pc.
// The semantic is
//   Let REF run `nr_ref` instructions first.
//   We expect that DUT will catch up with REF within `nr_dut` instructions.
void difftest_skip_dut(int nr_ref, int nr_dut) {
    skip_dut_nr_inst += nr_dut;

    while (nr_ref-- > 0) {
        ref_difftest_exec(1);
    }
}

// 在DUT host memory的`buf`和REF guest memory的`addr`之间拷贝`n`字节,
// `direction`指定拷贝的方向, `DIFFTEST_TO_DUT`表示往DUT拷贝, `DIFFTEST_TO_REF`表示往REF拷贝
// void difftest_memcpy(paddr_t addr, void *buf, size_t n, bool direction);
// `direction`为`DIFFTEST_TO_DUT`时, 获取REF的寄存器状态到`dut`;
// `direction`为`DIFFTEST_TO_REF`时, 设置REF的寄存器状态为`dut`;
// void difftest_regcpy(void *dut, bool direction);
// 让REF执行`n`条指令
// void difftest_exec(uint64_t n);
// 初始化REF的DiffTest功能
// void difftest_init();

// 为了使程序方便扩展，具备通用性，可以采用插件形式。
// 采用异步事件驱动模型，保证主程序逻辑不变，将各个业务已动态链接库的形式加载进来，这就是所谓的插件。
// 在NEMU中，使用动态链接库的方式来实现差分测试（difftest）。
// 动态链接库允许在运行时加载和链接代码，这样可以在不修改主程序的情况下添加或更新功能。
// 动态链接库的使用使得NEMU能够灵活地集成不同的测试工具和功能模块，而不需要重新编译整个程序。
// 动态链接库的加载和使用通常涉及到以下几个步骤：
// 1. 使用 `dlopen` 函数加载动态链接库文件。
// 2. 使用 `dlsym` 函数获取动态链接库中函数的地址。
// 3. 使用获取的函数指针调用动态链接库中的函数。
// 4. 使用 `dlclose` 函数关闭动态链接库。
// 5. 在程序结束时，确保释放动态链接库资源。
void init_difftest(char *ref_so_file, long img_size, int port) {
    assert(ref_so_file != NULL); // 检查传入的动态库

    void *handle;
    // dlopen以指定模式打开指定的动态连接库文件，并返回一个句柄给调用进程
    // RTLD_LAZY 暂缓决定，等有需要时再解出符号
    // RTLD_NOW 立即决定，返回前解除所有未决定的符号。
    handle = dlopen(ref_so_file, RTLD_LAZY); // 打开动态链接库
    assert(handle);

    // dlsym通过句柄和连接符名称获取函数名或者变量名
    ref_difftest_memcpy = dlsym(handle, "difftest_memcpy"); // 获取动态链接库中的函数地址
    assert(ref_difftest_memcpy);

    ref_difftest_regcpy = dlsym(handle, "difftest_regcpy"); // 复制寄存器状态
    assert(ref_difftest_regcpy);

    ref_difftest_exec = dlsym(handle, "difftest_exec"); // 执行指令
    assert(ref_difftest_exec);

    ref_difftest_raise_intr = dlsym(handle, "difftest_raise_intr"); // 处理中断
    assert(ref_difftest_raise_intr);

    void (*ref_difftest_init)(int) = dlsym(handle, "difftest_init"); // 初始化差分测试
    assert(ref_difftest_init);

    Log("Differential testing: %s", ANSI_FMT("ON", ANSI_FG_GREEN));
    Log("The result of every instruction will be compared with %s. "
        "This will help you a lot for debugging, but also significantly reduce the performance. "
        "If it is not necessary, you can turn it off in menuconfig.",
        ref_so_file);

    ref_difftest_init(port);
    // 将DUT的guset memory复制到REF中
    ref_difftest_memcpy(RESET_VECTOR, guest_to_host(RESET_VECTOR), img_size, DIFFTEST_TO_REF);
    ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
}

static void checkregs(CPU_state *ref, vaddr_t pc) {
    if (!isa_difftest_checkregs(ref, pc)) {
        nemu_state.state = NEMU_ABORT;
        nemu_state.halt_pc = pc;
        isa_reg_display();
    }
}

// 当nemu执行到某个指令时，调用difftest_step函数进行差分测试
void difftest_step(vaddr_t pc, vaddr_t npc) {
    // 创建一个CPU_state结构体来保存DUT的寄存器状态
    CPU_state ref_r;

    if (skip_dut_nr_inst > 0) {
        ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);
        if (ref_r.pc == npc) {
            skip_dut_nr_inst = 0;
            checkregs(&ref_r, npc);
            return;
        }
        skip_dut_nr_inst--;
        if (skip_dut_nr_inst == 0)
            panic("can not catch up with ref.pc = " FMT_WORD " at pc = " FMT_WORD, ref_r.pc, pc);
        return;
    }

    // 是否跳过对REF的检查
    if (is_skip_ref) {
        // to skip the checking of an instruction, just copy the reg state to reference design
        ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
        is_skip_ref = false;
        return;
    }

    // 执行DUT的指令
    ref_difftest_exec(1);
    // 复制DUT的寄存器状态到REF
    ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);

    checkregs(&ref_r, pc);
}
#else
void init_difftest(char *ref_so_file, long img_size, int port) {}
#endif
