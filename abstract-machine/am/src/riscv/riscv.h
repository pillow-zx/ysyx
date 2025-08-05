#ifndef RISCV_H__
#define RISCV_H__

#include <stdint.h>

/* I/O port operations */
static inline uint8_t  inb(uintptr_t addr) { return *(volatile uint8_t  *)addr; }   // 读取8位数据
static inline uint16_t inw(uintptr_t addr) { return *(volatile uint16_t *)addr; }   // 读取16位数据
static inline uint32_t inl(uintptr_t addr) { return *(volatile uint32_t *)addr; }   // 读取32位数据

/* Memory-mapped I/O operations */
static inline void outb(uintptr_t addr, uint8_t  data) { *(volatile uint8_t  *)addr = data; }   // 写入8位数据
static inline void outw(uintptr_t addr, uint16_t data) { *(volatile uint16_t *)addr = data; }   // 写入16位数据
static inline void outl(uintptr_t addr, uint32_t data) { *(volatile uint32_t *)addr = data; }   // 写入32位数据

#define PTE_V 0x01  // Valid bit    // 有效位
#define PTE_R 0x02  // Readable bit  // 可读位
#define PTE_W 0x04  // Writable bit  // 可写位
#define PTE_X 0x08  // Executable bit// 可执行位
#define PTE_U 0x10  // User-accessible bit // 用户可访问位
#define PTE_A 0x40  // Accessed bit // 已访问位
#define PTE_D 0x80  // Dirty bit

enum { MODE_U, MODE_S, MODE_M = 3 };
#define MSTATUS_MXR  (1 << 19)
#define MSTATUS_SUM  (1 << 18)

#if __riscv_xlen == 64
#define MSTATUS_SXL  (2ull << 34)
#define MSTATUS_UXL  (2ull << 32)
#else
#define MSTATUS_SXL  0
#define MSTATUS_UXL  0
#endif

#endif
