#pragma once

#include <tools.h>

#define CONFIG_MSIZE 0x8000000  // 128MB instead of 2GB

std::array<uint32_t, CONFIG_MSIZE / 4> &write_pmem();

uint32_t read_pmem(uint32_t addr);

void init_memory();