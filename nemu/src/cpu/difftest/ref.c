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

// Reference implementation of memory copy for differential testing difftest的内存拷贝参考实现
__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n, bool direction) {
  assert(0);
}

// Reference implementation of register copy for differential testing
// difftest的寄存器拷贝参考实现
__EXPORT void difftest_regcpy(void *dut, bool direction) {
  assert(0);
}

// Reference implementation of instruction execution for differential testing
// difftest的指令执行参考实现
__EXPORT void difftest_exec(uint64_t n) {
  assert(0);
}

__EXPORT void difftest_raise_intr(word_t NO) {
  assert(0);
}

// Reference implementation of initialization for differential testing
// difftest的初始化参考实现
// 该函数会在模拟器启动时被调用，用于初始化difftest相关
__EXPORT void difftest_init(int port) {
  void init_mem();
  init_mem();
  /* Perform ISA dependent initialization. */
  init_isa();
}
