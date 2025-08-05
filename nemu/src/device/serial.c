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

#include <utils.h>
#include <device/map.h>

/* http://en.wikibooks.org/wiki/Serial_Programming/8250_UART_Programming */
// NOTE: this is compatible to 16550

#define CH_OFFSET 0 // 串口设备的 I/O 映射偏移地址

static uint8_t *serial_base = NULL; // 串口设备的 I/O 映射基地址


// 将字符输出到串口设备
// 在 AM 模式下使用 putch 函数输出字符，在其他模式下使用 putc 函数输出字符到 stderr
/* 没有运行时环境就代表没有程序正在运行, 如果没有程序运行就代表没有程序在使用串口,
 * 此时一旦串口的状态寄存器变化就代表出现了 bug , 所以输出到 stderr */
static void serial_putc(char ch) {
    // 通过一个选择器来实现对不同环境的适配
    MUXDEF(CONFIG_TARGET_AM, putch(ch), putc(ch, stderr));
}

/* 串口 I/O 设备回调函数 */
static void serial_io_handler(uint32_t offset, int len, bool is_write) {
    assert(len == 1);
    switch (offset) {
        /* We bind the serial port with the host stderr in NEMU. */
        // 这里将串口设备的 I/O 映射到 stderr 输出
        case CH_OFFSET:
            if (is_write)
                serial_putc(serial_base[0]);
            else
                panic("do not support read");
            break;
        default: panic("do not support offset = %d", offset);
    }
}

/* 为串口设备分配 I/O 映射空间, 通过端口硬映射和内存映射来让程序可以通过 I/O 指令访问 */
void init_serial() {
    serial_base = new_space(8); // 为串口设备分配 8 字节的端口 I/O 映射空间, 地址从0x3F8开始
#ifdef CONFIG_HAS_PORT_IO
    add_pio_map("serial", CONFIG_SERIAL_PORT, serial_base, 8, serial_io_handler);
#else
    /* 为串口设备分配 8 字节的内存 I/O 映射空间, 地址从0xa00003F8开始 */
    add_mmio_map("serial", CONFIG_SERIAL_MMIO, serial_base, 8, serial_io_handler);
#endif
}
