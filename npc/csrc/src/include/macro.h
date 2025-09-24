#pragma once

#include <stdio.h>

/* 内存 */
#define MEMORY_START       0x80000000
#define MEMORY_SIZE        0x8000000  // 128MB

/* CPU */
#define NPC_BITS           32
#define CPU_REGISTERS      16

/* 设备 */
#define IO_SPACE_MAX       (32 * 1024 * 1024)  // 32MB
#define PAGE_SHIFT         12
#define PAGE_SIZE          (1 << PAGE_SHIFT)  // 4KB
#define PAGE_MASK          (~(PAGE_SIZE - 1))
#define SERIAL_MMIO        0xa00003f8
#define RTC_MMIO           0xa0000048
#define MMIO_START         0xa0000000
#define MMIO_END           (MMIO_START + IO_SPACE_MAX - 1)
#define TIMER_HZ           60  // 60Hz

#define ANSI_FG_BLACK      "\33[1;30m"  // 黑色
#define ANSI_FG_RED        "\33[1;31m"  // 红色
#define ANSI_FG_GREEN      "\33[1;32m"  // 绿色
#define ANSI_FG_YELLOW     "\33[1;33m"  // 黄色
#define ANSI_FG_BLUE       "\33[1;34m"  // 蓝色
#define ANSI_FG_MAGENTA    "\33[1;35m"  // 紫色
#define ANSI_FG_CYAN       "\33[1;36m"  // 青色
#define ANSI_FG_WHITE      "\33[1;37m"  // 白色
#define ANSI_BG_BLACK      "\33[1;40m"  // 黑色背景
#define ANSI_BG_RED        "\33[1;41m"  // 红色背景
#define ANSI_BG_GREEN      "\33[1;42m"  // 绿色背景
#define ANSI_BG_YELLOW     "\33[1;43m"  // 黄色背景
#define ANSI_BG_BLUE       "\33[1;44m"  // 蓝色背景
#define ANSI_BG_MAGENTA    "\33[1;45m"  // 紫色背景
#define ANSI_BG_CYAN       "\33[1;46m"  // 青色背景
#define ANSI_BG_WHITE      "\33[1;47m"  // 白色背景
#define ANSI_NONE          "\33[0m"     // 关闭所有属性

#define ANSI_FMT(str, fmt) fmt str ANSI_NONE  // ANSI_FMT(str, fmt) 是一个宏定义，用于格式化字符串的输出


#define _Log(...)           \
    do {                    \
        printf(__VA_ARGS__); \
    } while (0)

#define Log(format, ...) _Log(ANSI_FMT("[%s:%d %s] " format, ANSI_FG_BLUE) "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)
