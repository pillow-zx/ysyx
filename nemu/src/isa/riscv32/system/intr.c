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

// 根据RISC-V特权架构规范，ecall指令会根据执行时的特权级别产生不同的异常原因：
// 用户模式执行ecall：
// mcause = 8 (Environment call from U-mode)
// 监督者模式执行ecall：
// mcause = 9 (Environment call from S-mode)
// 机器模式执行ecall：
// mcause = 11 (Environment call from M-mode)

// 函数会被操作系统，用于处理中断或异常，
word_t isa_raise_intr(word_t NO, vaddr_t epc) {
    /* TODO: Trigger an interrupt/exception with ``NO''.
     * Then return the address of the interrupt/exception vector.
     */
    cpu.csr.mcause = NO;  // 设置异常原因寄存器
    cpu.csr.mepc = epc;   // 保存异常发生时的pc
    // 处理mstatus寄存器
    word_t mstatus = cpu.csr.mstatus;
    word_t mie = (mstatus >> 3) & 1;
    mstatus = (mstatus & ~(1 << 7)) | (mie << 7);  // MIE -> MPIE
    mstatus &= ~(1 << 3);  // 清除MIE
    cpu.csr.mstatus = mstatus;
    return cpu.csr.mtvec; // 返回异常处理程序的入口地址
}

word_t isa_query_intr() {
    return INTR_EMPTY;
}
