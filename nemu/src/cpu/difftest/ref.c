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

#include <isa.h>
#include <cpu/cpu.h>
#include <difftest-def.h>
#include <memory/paddr.h>
#include <cpu/decode.h>

// Reference implementation of memory copy for differential testing
// direction 指定copy的方向, DIFFTEST_TO_REF 表示向ref拷贝, DIFFTEST_TO_DUT 表示向dut拷贝
__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n, bool direction) {
    if (direction == DIFFTEST_TO_REF) {
        // 将buf的数据复制到ref的物理内存地址addr处
        memcpy(guest_to_host(addr), buf, n);
    } else {
        // 将ref的物理内存地址addr处的数据复制到buf中
        memcpy(buf, guest_to_host(addr), n);
    }
}

// Reference implementation of register copy for differential testing
// direction 为 DIFFTEST_TO_REF 时, 设置ref的寄存器状态为dut的寄存器状态
// direction 为 DIFFTEST_TO_DUT 时, 获取ref的寄存器状态到dut
__EXPORT void difftest_regcpy(void *dut, bool direction) {
    RISCV_GPR_TYPE *regs = (RISCV_GPR_TYPE *)dut;

    if (direction == DIFFTEST_TO_REF) {
        // 将DUT的寄存器状态复制到ref的CPU寄存器
        for (int i = 0; i < RISCV_GPR_NUM; i++) {
            cpu.gpr[i] = regs[i];
        }
        // 复制PC寄存器
        cpu.pc = regs[RISCV_GPR_NUM];
    } else {
        // 将ref的CPU寄存器状态复制到DUT
        for (int i = 0; i < RISCV_GPR_NUM; i++) {
            regs[i] = cpu.gpr[i];
        }
        // 复制PC寄存器
        regs[RISCV_GPR_NUM] = cpu.pc;
    }
}

// Reference implementation of instruction execution for differential testing
// 让ref执行n条指令，使用NEMU的运算模块
__EXPORT void difftest_exec(uint64_t n) {
    // 执行n条指令
    for (uint64_t i = 0; i < n; i++) {
        // 准备解码结构
        Decode s;
        s.pc = cpu.pc;   // 当前指令地址
        s.snpc = cpu.pc; // 静态下一条指令地址，初始化为当前PC
        s.dnpc = cpu.pc; // 动态下一条指令地址，初始化为当前PC

        // 从内存中取指令（RISC-V指令为4字节）
        s.isa.inst = paddr_read(cpu.pc, 4);

        // 执行一条指令，这会更新s.dnpc
        isa_exec_once(&s);

        // 更新CPU的PC寄存器为下一条指令的地址
        cpu.pc = s.dnpc;
    }
}

__EXPORT void difftest_raise_intr(word_t NO) {
    assert(0);
}

// Reference implementation of initialization for differential testing
// 初始化ref的difftest功能
__EXPORT void difftest_init(void *regs, void *pmems) {
    // 声明和调用初始化函数
    void init_mem();
    void init_isa();

    // 初始化内存子系统
    init_mem();

    // 执行ISA相关的初始化
    init_isa();

    // 复制初始的CPU状态到reference设计
    // 将传入的寄存器状态复制到ref
    difftest_regcpy(regs, DIFFTEST_TO_REF);

    // 将传入的物理内存状态复制到ref
    difftest_memcpy(CONFIG_MBASE, pmems, CONFIG_MSIZE, DIFFTEST_TO_REF);
}
