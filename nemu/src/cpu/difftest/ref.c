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

// Reference implementation of memory copy for differential
// direction 指定copy的方向, difftest_to_dut 表示向dut拷贝, difftest_to_ref 表示向ref拷贝
__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n, bool direction) {
    if (direction) {
        memcpy(guest_to_host(addr), buf, n); // 从 buf 复制数据到物理地址 addr
        return;
    }
    memcpy(buf, guest_to_host(addr), n); // 从物理地址 addr 复制数据到 buf
}
// direction 为 difftest_to_dut 时, 获取ref的寄存器状态到 dut
// direction 为 difftest_to_ref 时, 设置ref的寄存器状态到 dut

// Reference implementation of register copy for differential testing
__EXPORT void difftest_regcpy(void *dut, bool direction) {
    RISCV_GPR_TYPE *regs = (RISCV_GPR_TYPE *)dut; // 将传入的指针转换为 uint32_t 类型的指针)
    if (direction) {
        for (int i = 0; i < RISCV_GPR_NUM; i++) {
            cpu.gpr[i] = regs[i]; // 将 DUT 的寄存器状态复制到 CPU 寄存器
        }
        return;
    }
    for (int i = 0; i < RISCV_GPR_NUM; i++) {
        regs[i] = cpu.gpr[i]; // 将 CPU 寄存器状态复制到 DUT 的寄存器
    }
}

#include <cpu/decode.h>
// Reference implementation of instruction execution for differential testing
// 让 ref 执行 n 条指令
__EXPORT void difftest_exec(uint64_t n, uint32_t pc, uint32_t dnpc, uint32_t inst) {
    Decode s;
    for (uint64_t i = 0; i < n; i++) {
        s.pc = pc;         // 设置当前指令的地址
        s.snpc = dnpc;     // 设置下一条指令的地址
        s.dnpc = dnpc; // 假设每条指令长度为4字节
        s.isa.inst = inst; // 设置当前指令
        isa_exec_once(&s); // 执行一次指令
    }
}

__EXPORT void difftest_raise_intr(word_t NO) {
    assert(0);
}

// Reference implementation of initialization for differential testing
// 初始化 ref 的difftest功能
__EXPORT void difftest_init(void *regs, void *pmems) {
    void init_mem();
    init_mem();
    /* Perform ISA dependent initialization. */
    init_isa();
    // copy the initial state of the CPU to the reference design
    difftest_regcpy(&regs, DIFFTEST_TO_REF);                             // 将CPU寄存器状态复制到REF
    difftest_memcpy(CONFIG_MBASE, pmems, CONFIG_MSIZE, DIFFTEST_TO_REF); // 将物理内存状态复制到REF
}
