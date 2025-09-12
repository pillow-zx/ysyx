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
/*
 *八个控制状态寄存器（CSR）是机器模式下异常处理的必要部分：
 *- mtvec（Machine Trap Vector）它保存发生异常时处理器需要跳转到的地址。
 *- mepc（Machine Exception PC）它指向发生异常的指令。
 *- mcause（Machine Exception Cause）它指示发生异常的种类。
 *- mie（Machine Interrupt Enable）它指出处理器目前能处理和必须忽略的中断。
 *- mip（Machine Interrupt Pending）它列出目前正准备处理的中断。
 *- mtval（Machine Trap Value）它保存了陷入（trap）的附加信息：地址例外中出错的地址、发生非法指令例外的指令本身，对于其他异常，它的值为 0。
 *- mscratch（Machine Scratch）它暂时存放一个字大小的数据。
 *- mstatus（Machine Status）它保存全局中断使能，以及许多其他的状态。
 */


/*
| 位     | 名称        | 中文含义                                                                        |
| -----  | ---------   | --------------------------------------------------------------------------- |
| 31     | SD          | 状态位，用于指示浮点寄存器状态（FS）或扩展状态（XS）是否“脏”，即是否被修改过                                   |
| 30-29  | XS          | 扩展状态位，用于保存自定义扩展的状态信息                                                        |
| 28-27  | FS          | 浮点状态位，表示浮点寄存器的状态（干净/脏/初始化等）                                                 |
| 23     | MPRV        | 内存访问特权，如果置位，load/store 指令使用 MPP 指定的特权等级访问内存                                 |
| 22     | SUM         | 允许 Supervisor 模式访问用户模式内存                                                    |
| 21     | MXR         | 允许读取可执行内存（Make eXecutable Readable）                                         |
| 7      | TVM         | Trap Virtual Memory，用于控制是否允许 S-mode 执行虚拟内存相关指令                              |
| 6      | TW          | Timeout Wait，控制时间限制相关陷阱                                                     |
| 5      | TSR         | Trap SRET，控制 SRET 指令陷阱                                                      |
| 3      | MIE         | Machine Interrupt Enable，当前 Machine 模式中断使能标志                                |
| 7      | MPIE        | Machine Previous Interrupt Enable，保存上一次中断使能状态                               |
| 11-12  | SPP / SIE   | Supervisor Previous Privilege / Supervisor Interrupt Enable，保存 S 模式的特权和中断状态 |
| 0-1    | MPP         | Machine Previous Privilege，保存上一次特权等级（User / Supervisor / Machine）           |
*/


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

// 定义cpu结构体 (CPU_state)
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
