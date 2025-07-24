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

#include <dlfcn.h>             // 动态库加载相关函数(dlopen, dlsym等)
#include <capstone/capstone.h> // Capstone反汇编引擎头文件
#include <common.h>            // NEMU通用头文件

// 动态加载的Capstone库函数指针
// cs_disasm_dl: 反汇编函数指针，将机器码转换为汇编指令
static size_t (*cs_disasm_dl)(csh handle, const uint8_t *code, size_t code_size, uint64_t address, size_t count,
                              cs_insn **insn);
// cs_free_dl: 释放反汇编结果内存的函数指针
static void (*cs_free_dl)(cs_insn *insn, size_t count);

// Capstone反汇编引擎句柄，用于标识一个反汇编会话
static csh handle;

/**
 * 初始化反汇编器
 * 功能：动态加载Capstone反汇编库，并根据当前ISA架构配置反汇编引擎
 */
void init_disasm() {
    void *dl_handle;
    // 动态加载Capstone反汇编库(.so动态链接库文件)
    // RTLD_LAZY: 延迟加载，只有在实际调用符号时才解析
    dl_handle = dlopen("tools/capstone/repo/libcapstone.so.5", RTLD_LAZY);
    assert(dl_handle); // 确保库加载成功

    // 从动态库中获取cs_open函数的地址
    // cs_open用于初始化Capstone反汇编引擎
    cs_err (*cs_open_dl)(cs_arch arch, cs_mode mode, csh *handle) = NULL;
    cs_open_dl = dlsym(dl_handle, "cs_open");
    assert(cs_open_dl); // 确保函数符号找到

    // 从动态库中获取cs_disasm函数的地址(反汇编核心函数)
    cs_disasm_dl = dlsym(dl_handle, "cs_disasm");
    assert(cs_disasm_dl);

    // 从动态库中获取cs_free函数的地址(内存释放函数)
    cs_free_dl = dlsym(dl_handle, "cs_free");
    assert(cs_free_dl);

    // 根据编译时配置的ISA架构选择对应的Capstone架构
    // MUXDEF是NEMU的宏，根据CONFIG_ISA_xxx配置选择对应值
    cs_arch arch = MUXDEF(CONFIG_ISA_x86, CS_ARCH_X86,                                             // x86架构
                          MUXDEF(CONFIG_ISA_mips32, CS_ARCH_MIPS,                                  // MIPS32架构
                                 MUXDEF(CONFIG_ISA_riscv, CS_ARCH_RISCV,                           // RISC-V架构
                                        MUXDEF(CONFIG_ISA_loongarch32r, CS_ARCH_LOONGARCH, -1)))); // LoongArch架构

    // 根据ISA架构和位数配置选择对应的Capstone模式
    cs_mode mode =
        MUXDEF(CONFIG_ISA_x86, CS_MODE_32,               // x86 32位模式
               MUXDEF(CONFIG_ISA_mips32, CS_MODE_MIPS32, // MIPS 32位模式
                      MUXDEF(CONFIG_ISA_riscv,
                             MUXDEF(CONFIG_ISA64, CS_MODE_RISCV64, CS_MODE_RISCV32) |
                                 CS_MODE_RISCVC,                                          // RISC-V模式(支持压缩指令)
                             MUXDEF(CONFIG_ISA_loongarch32r, CS_MODE_LOONGARCH32, -1)))); // LoongArch 32位模式

    // 使用指定的架构和模式初始化Capstone引擎
    int ret = cs_open_dl(arch, mode, &handle);
    assert(ret == CS_ERR_OK); // 确保初始化成功

#ifdef CONFIG_ISA_x86
    // 如果是x86架构，需要设置汇编语法为AT&T语法
    // (AT&T语法: movl %eax, %ebx vs Intel语法: mov ebx, eax)
    cs_err (*cs_option_dl)(csh handle, cs_opt_type type, size_t value) = NULL;
    cs_option_dl = dlsym(dl_handle, "cs_option");
    assert(cs_option_dl);

    ret = cs_option_dl(handle, CS_OPT_SYNTAX, CS_OPT_SYNTAX_ATT);
    assert(ret == CS_ERR_OK);
#endif
}

/**
 * 反汇编函数
 * 功能：将一段机器码反汇编为人类可读的汇编指令
 *
 * @param str    输出缓冲区，用于存储反汇编后的汇编指令字符串
 * @param size   输出缓冲区的大小
 * @param pc     指令的程序计数器地址(虚拟地址)
 * @param code   指向机器码字节序列的指针
 * @param nbyte  机器码的字节数
 */
void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte) {
    cs_insn *insn; // Capstone指令结构体指针，用于存储反汇编结果

    // 调用Capstone的反汇编函数
    // handle: 之前初始化的Capstone引擎句柄
    // code: 要反汇编的机器码字节序列
    // nbyte: 机器码字节数
    // pc: 指令地址(用于地址相关指令的正确显示)
    // 0: 反汇编指令数量(0表示尽可能多)
    // &insn: 输出参数，返回反汇编结果
    size_t count = cs_disasm_dl(handle, code, nbyte, pc, 0, &insn);
    assert(count == 1); // 确保成功反汇编了一条指令

    // 将指令助记符(如add, mov, jmp等)复制到输出缓冲区
    int ret = snprintf(str, size, "%s", insn->mnemonic);

    // 如果指令有操作数(如寄存器、立即数等)，则添加到输出中
    if (insn->op_str[0] != '\0') {
        // 格式: "助记符\t操作数" (用制表符分隔)
        // 例如: "add\t%eax, %ebx" 或 "addi\tx1, x2, 100"
        snprintf(str + ret, size - ret, "\t%s", insn->op_str);
    }

    // 释放Capstone分配的内存
    cs_free_dl(insn, count);
}
