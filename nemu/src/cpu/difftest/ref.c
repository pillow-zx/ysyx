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
        memcpy(buf, guest_to_host(addr), n); // 从物理地址 addr 复制数据到 buf
        return;
    }
    memcpy(guest_to_host(addr), buf, n); // 从 buf 复制数据到物理地址 addr
}
// direction 为 difftest_to_dut 时, 获取ref的寄存器状态到 dut
// direction 为 difftest_to_ref 时, 设置ref的寄存器状态到 dut


// Reference implementation of register copy for differential testing
__EXPORT void difftest_regcpy(void *dut, bool direction) {
    CPU_state *temp_cpu = (CPU_state *)dut; // 将传入的指针转换为 CPU_state 类型
    if (direction) {
        for (int i = 0; i < ARRLEN(temp_cpu->gpr); i++) {
            temp_cpu->gpr[i] = cpu.gpr[i]; // 将 DUT 的寄存器值复制到 REF
        }
        return;
    }
    for (int i = 0; i < ARRLEN(temp_cpu->gpr); i++) {
        cpu.gpr[i] = temp_cpu->gpr[i]; // 将 REF 的寄存器值复制到 DUT
    }
}

#include "cpu/decode.h"
// Reference implementation of instruction execution for differential testing
// 让 ref 执行 n 条指令
__EXPORT void difftest_exec(uint64_t n) {
    Decode s;
    for (uint64_t i = 0; i < n; i++) {
        s.pc = cpu.pc; // 设置当前指令的地址
        s.snpc = cpu.pc; // 设置下一条指令的地址
        isa_exec_once(&s); // 执行一次指令
        cpu.pc = s.dnpc; // 更新PC为下一条指令的地址
    }
}

__EXPORT void difftest_raise_intr(word_t NO) {
    assert(0);
}

// Reference implementation of initialization for differential testing
// 初始化 ref 的difftest功能
__EXPORT void difftest_init(int port) {
    void init_mem();
    init_mem();
    /* Perform ISA dependent initialization. */
    init_isa();
}
