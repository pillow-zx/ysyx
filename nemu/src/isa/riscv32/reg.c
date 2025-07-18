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
#include "local-include/reg.h"
#include "common.h"

extern CPU_state cpu;

const char *regs[] = {"$0", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
                      "a1", "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
                      "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

// 打印寄存器的值
void isa_reg_display() {
    bool success = false;
    for (int i = 0; i < ARRLEN(regs); i++) { // ARRLEN(regs)计算regs数组的长度
        // printf("%s: 0x%08x\n", regs[i], gpr(i));
        word_t value = isa_reg_str2val(regs[i], &success); // 获取寄存器的值
        printf("%-16s0x%-16x%d\n", regs[i], value,
               value); // 打印寄存器名称、值（十六进制）和值（十进制）
    }
    printf("pc:0x%-16x%d\n", cpu.pc, cpu.pc); // 打印程序计数器pc的值
}

// 获取寄存器的值
word_t isa_reg_str2val(const char *s, bool *success) {
    for (int i = 0; i < 32; i++) {     // 遍历所有寄存器
        if (strcmp(s, regs[i]) == 0) { // 如果寄存器名称匹配
            *success = true;
            return gpr(i); // 使用gpr函数获取寄存器的值
        }
    }
    *success = false;
    return 0;
}
