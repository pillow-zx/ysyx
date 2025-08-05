#ifndef NEMU_H__
#define NEMU_H__

#include <klib-macros.h>

#include ISA_H // the macro `ISA_H` is defined in CFLAGS
               // it will be expanded as "x86/x86.h", "mips/mips32.h", ...

#if defined(__ISA_X86__)
# define nemu_trap(code) asm volatile ("int3" : :"a"(code))
#elif defined(__ISA_MIPS32__)
# define nemu_trap(code) asm volatile ("move $v0, %0; sdbbp" : :"r"(code))
#elif defined(__riscv)
# define nemu_trap(code) asm volatile("mv a0, %0; ebreak" : :"r"(code))
#elif defined(__ISA_LOONGARCH32R__)
# define nemu_trap(code) asm volatile("move $a0, %0; break 0" : :"r"(code))
#else
# error unsupported ISA __ISA__
#endif

#if defined(__ARCH_X86_NEMU)
# define DEVICE_BASE 0x0
#else
# define DEVICE_BASE 0xa0000000 // 设备基地址
#endif

#define MMIO_BASE 0xa0000000    // 内存映射I/O基地址

#define SERIAL_PORT     (DEVICE_BASE + 0x00003f8)   // 串口地址
#define KBD_ADDR        (DEVICE_BASE + 0x0000060)   // 键盘地址
#define RTC_ADDR        (DEVICE_BASE + 0x0000048)   // 实时时钟地址
#define VGACTL_ADDR     (DEVICE_BASE + 0x0000100)   // VGA控制器地址
#define AUDIO_ADDR      (DEVICE_BASE + 0x0000200)   // 音频设备地址
#define DISK_ADDR       (DEVICE_BASE + 0x0000300)   // 磁盘设备地址
#define FB_ADDR         (MMIO_BASE   + 0x1000000)   // 帧缓冲区地址
#define AUDIO_SBUF_ADDR (MMIO_BASE   + 0x1200000)   // 音频设备双缓冲区地址

extern char _pmem_start;    // 物理内存起始地址 0x80000000
#define PMEM_SIZE (128 * 1024 * 1024)
#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)
/* 物理内存地址空间 */
#define NEMU_PADDR_SPACE \
  RANGE(&_pmem_start, PMEM_END), \
  RANGE(FB_ADDR, FB_ADDR + 0x200000), \
  RANGE(MMIO_BASE, MMIO_BASE + 0x1000) /* serial, rtc, screen, keyboard */

typedef uintptr_t PTE;  // 页表项类型

#define PGSIZE    4096  // 页大小

#endif
