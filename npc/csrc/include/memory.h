#pragma once

#include <tools.h>

#define CONFIG_MSIZE 0x8000000  // 128MB instead of 2GB

std::array<uint32_t, CONFIG_MSIZE / 4> &write_pmem();

uint32_t read_pmem(uint32_t addr);

// 新增：字节级访问函数声明
uint8_t read_pmem_byte(uint32_t addr);
void write_pmem_byte(uint32_t addr, uint8_t value);

// 新增：半字级访问函数声明
uint16_t read_pmem_halfword(uint32_t addr);
void write_pmem_halfword(uint32_t addr, uint16_t value);

void init_memory();