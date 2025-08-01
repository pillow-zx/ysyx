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

#ifndef __RISCV_REG_H__
#define __RISCV_REG_H__

#include <common.h>

/* 检测寄存器索引是否合法 */
static inline int check_reg_idx(int idx) {
    /* 如果定义了CONFIG_RVE，使用16个寄存器（RV32E指令集), 否则使用32个寄存器（标准RV32I指令集）*/
    /* assert验证idx必须大于等于0且必须小于最大寄存器数（16或32) */
    // 如果条件不满足，程序会触发断言失败
    // 如果检查通过，返回原始的索引值
    IFDEF(CONFIG_RT_CHECK, assert(idx >= 0 && idx < MUXDEF(CONFIG_RVE, 16, 32)));
    return idx;
}

#define gpr(idx) (cpu.gpr[check_reg_idx(idx)])

static inline const char* reg_name(int idx) {
    extern const char* regs[];
    return regs[check_reg_idx(idx)];
}

#endif
