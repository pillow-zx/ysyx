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

#ifndef __ISA_RISCV_H__
#define __ISA_RISCV_H__

#include <common.h>

// 定义cpu结构体 (CPU_state)
typedef struct {
    word_t mepc;     // 存储异常发生时的pc
    word_t mcause;   // 存储异常原因
    word_t mstatus;  // 存储机器状态寄存器, 由于本项目不涉及特权级切换, 暂时不会使用，但为了difftest, 初始化为0x1800
    word_t mtvec;    // 存储异常处理程序的入口地址
} CSR;

/*
 *CSR寄存器地址十进制功能描述
 *mstatus  0x300  768 机器状态寄存器
 *mtvec    0x305  773 机器陷阱向量基地址寄存器
 *mepc     0x341  833 机器异常程序计数器
 *mcause   0x342  834 机器陷阱原因寄存器
 */
 
// CSR寄存器地址定义
#define CSR_MSTATUS 0x300
#define CSR_MTVEC   0x305
#define CSR_MEPC    0x341
#define CSR_MCAUSE  0x342


typedef struct {
    word_t  gpr[MUXDEF(CONFIG_RVE, 16, 32)];  // 通用寄存器
    vaddr_t pc;                               // pc寄存器
    CSR     csr;
} MUXDEF(CONFIG_RV64, riscv64_CPU_state, riscv32_CPU_state);

// decode  riscv32_ISADecodeInfo
typedef struct {
    uint32_t inst;
} MUXDEF(CONFIG_RV64, riscv64_ISADecodeInfo, riscv32_ISADecodeInfo);

#define isa_mmu_check(vaddr, len, type) (MMU_DIRECT)

#endif
