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
#include "debug.h"

// 根据RISC-V特权架构规范，ecall指令会根据执行时的特权级别产生不同的异常原因：
// 用户模式执行ecall：
// mcause = 8 (Environment call from U-mode)
// 监督者模式执行ecall：
// mcause = 9 (Environment call from S-mode)
// 机器模式执行ecall：
// mcause = 11 (Environment call from M-mode)

// CSR访问函数
word_t csr_read(word_t csr) {
    switch (csr) {
        case CSR_MSTATUS: return cpu.csr.mstatus;
        case CSR_MTVEC: return cpu.csr.mtvec;
        case CSR_MEPC: return cpu.csr.mepc;
        case CSR_MCAUSE: return cpu.csr.mcause;
        default: printf("Error: Unsupported CSR read at 0x%x\n", csr); assert(0);
    }
}

void csr_write(word_t csr, word_t val) {
    switch (csr) {
        case CSR_MSTATUS: cpu.csr.mstatus = val; break;
        case CSR_MTVEC: cpu.csr.mtvec = val; break;
        case CSR_MEPC: cpu.csr.mepc = val; break;
        case CSR_MCAUSE: cpu.csr.mcause = val; break;
        default: printf("Error: Unsupported CSR write at 0x%x\n", csr); assert(0);
    }
}


// RISC-V异常代码定义
#define CAUSE_MISALIGNED_FETCH    0
#define CAUSE_FETCH_ACCESS        1
#define CAUSE_ILLEGAL_INSTRUCTION 2
#define CAUSE_BREAKPOINT          3
#define CAUSE_MISALIGNED_LOAD     4
#define CAUSE_LOAD_ACCESS         5
#define CAUSE_MISALIGNED_STORE    6
#define CAUSE_STORE_ACCESS        7
#define CAUSE_USER_ECALL          8
#define CAUSE_SUPERVISOR_ECALL    9
#define CAUSE_MACHINE_ECALL       11
#define CAUSE_FETCH_PAGE_FAULT    12
#define CAUSE_LOAD_PAGE_FAULT     13
#define CAUSE_STORE_PAGE_FAULT    15

static void etrace_start(word_t NO, vaddr_t epc) {
    switch (NO) {
        case CAUSE_MACHINE_ECALL: {
            Log("[ETRACE] MACHINE_ECALL -> pc = 0x%08x mcause: 0x%08x, mstatus: 0x%08x, mtvec: 0x%08x, mepc: 0x%08x", epc, csr_read(CSR_MCAUSE), csr_read(CSR_MSTATUS),
                csr_read(CSR_MTVEC), csr_read(CSR_MEPC));
        }
        default: {
            Log("[ETRACE] UNSUPPORTED EXCEPTION NO: %d -> pc = 0x%08x mcause: 0x%08x, mstatus: 0x%08x, mtvec: 0x%08x, mepc: 0x%08x", NO, epc, csr_read(CSR_MCAUSE),
                csr_read(CSR_MSTATUS), csr_read(CSR_MTVEC), csr_read(CSR_MEPC));
        }
    }
}


// 函数会被操作系统，用于处理中断或异常，
word_t isa_raise_intr(word_t NO, vaddr_t epc) {
    /* TODO: Trigger an interrupt/exception with ``NO''.
     * Then return the address of the interrupt/exception vector.
     */
    etrace_start(NO, epc);
    cpu.csr.mcause = NO;  // 设置异常原因寄存器
    cpu.csr.mepc = epc;   // 保存异常发生时的pc
    // 处理mstatus寄存器
    word_t mstatus = cpu.csr.mstatus;
    word_t mie = (mstatus >> 3) & 1;
    mstatus = (mstatus & ~(1 << 7)) | (mie << 7);  // MIE -> MPIE
    mstatus &= ~(1 << 3);                          // 清除MIE
    cpu.csr.mstatus = mstatus;
    return cpu.csr.mtvec;  // 返回异常处理程序的入口地址
}

word_t isa_query_intr() { return INTR_EMPTY; }
