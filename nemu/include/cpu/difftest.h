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

#ifndef __CPU_DIFFTEST_H__
#define __CPU_DIFFTEST_H__

#include <common.h>
#include <difftest-def.h>

#ifdef CONFIG_DIFFTEST
void difftest_skip_ref();
void difftest_skip_dut(int nr_ref, int nr_dut);
void difftest_set_patch(void (*fn)(void* arg), void* arg);
void difftest_step(vaddr_t pc, vaddr_t npc);
void difftest_detach();
void difftest_attach();
#else
// static: 限制函数作用域在当前文件内
// inline: 建议编译器内联展开此函数
static inline void difftest_skip_ref() {}       // 跳过参考实现的检查
static inline void difftest_skip_dut(int nr_ref, int nr_dut) {} // 跳过被测单元的检查
static inline void difftest_set_patch(void (*fn)(void* arg), void* arg) {}   // 设置差分测试补丁
// vaddr_t pc: 当前指令的虚拟地址
// vaddr_t npc: 下一条指令的虚拟地址
static inline void difftest_step(vaddr_t pc, vaddr_t npc) {}
static inline void difftest_detach() {}     // 分离差分测试
static inline void difftest_attach() {}      // 附加差分测试
#endif

extern void (*ref_difftest_memcpy)(paddr_t addr, void* buf, size_t n, bool direction);
extern void (*ref_difftest_regcpy)(void* dut, bool direction);
extern void (*ref_difftest_exec)(uint64_t n);
extern void (*ref_difftest_raise_intr)(uint64_t NO);

static inline bool difftest_check_reg(const char* name, vaddr_t pc, word_t ref, word_t dut) {
    if (ref != dut) {
        Log("%s is different after executing instruction at pc = " FMT_WORD
            ", right = " FMT_WORD ", wrong = " FMT_WORD ", diff = " FMT_WORD,
            name, pc, ref, dut, ref ^ dut);
        return false;
    }
    return true;
}

#endif
