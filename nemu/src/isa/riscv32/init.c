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
#include <memory/paddr.h>

// this is not consistent with uint8_t      虽然定义为 uint32_t 数组，但最终会通过 memcpy 复制到以 uint8_t 为单位的内存中
// but it is ok since we do not access the array directly   因为不直接访问数组，所以类型不一致不会造成问题
/* 这段代码定义了一个简单的 RISC-V 指令序列，作为 NEMU 模拟器的内置初始程序。*/
static const uint32_t img[] = {
  0x00000297,  // auipc t0,0
  0x00028823,  // sb  zero,16(t0)
  0x0102c503,  // lbu a0,16(t0)
  0x00100073,  // ebreak (used as nemu_trap)
  0xdeadbeef,  // some data
};
// 指令分析
// 第一条指令: auipc t0,0
// 机器码：0x00000297
// 功能：将当前 PC 值加上立即数（这里为0）存入 t0 寄存器
// t0 寄存器编号为 x5
// 第二条指令: sb zero,16(t0)
// 机器码：0x00028823
// 功能：将零寄存器（x0）的值存储到 t0+16 的内存地址
// 使用字节存储（sb = store byte）
// 第三条指令: lbu a0,16(t0)
// 机器码：0x0102c503
// 功能：从 t0+16 的地址加载一个字节到 a0 寄存器
// 无符号字节加载（lbu = load byte unsigned）
// 第四条指令: ebreak
// 机器码：0x00100073
// 功能：触发环境断点，这里用作 NEMU 陷阱
// 数据: 0xdeadbeef
// 不是指令，而是一个数据值
// 通常用作调试标记或填充值

// 程序功能分析
// 这个简单的程序：
// 获取当前 PC 值
// 在 PC+16 的位置存储一个零字节
// 从同一位置读取这个字节到 a0
// 触发 ebreak 结束程序

static void restart() {
    /* Set the initial program counter. */
    /* 初始化程序计数器 */
    cpu.pc = RESET_VECTOR;

    /* The zero register is always 0. */
    /* 把0号寄存器始终设置为0 */
    cpu.gpr[0] = 0;
}

void init_isa() {
    /* Load built-in image. */
    /* 读取程序 */
    /* 复制img中的内容到guest_to-host()函数返回的指针，大小为sizeof(img) */
    memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

    /* Initialize this virtual computer system. */
    /* 初始化寄存器 */
    restart();
}
