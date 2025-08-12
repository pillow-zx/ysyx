#pragma once

/* 内存 */
#define MEMORY_START  0x80000000
#define MEMORY_SIZE   0x8000000 // 128MB

/* CPU */
#define NPC_BITS      32
#define CPU_REGISTERS 16

/* 设备 */
#define IO_SPACE_MAX  (32 * 1024 * 1024) // 32MB
#define PAGE_SHIFT    12
#define PAGE_SIZE     (1 << PAGE_SHIFT) // 4KB
#define PAGE_MASK     (~(PAGE_SIZE - 1))
#define SERIAL_MMIO   0xa00003f8
#define RTC_MMIO      0xa0000048
#define MMIO_START    0xa0000000
#define MMIO_END      (MMIO_START + IO_SPACE_MAX - 1)
#define TIMER_HZ      60 // 60Hz
