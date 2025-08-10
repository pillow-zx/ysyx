#pragma once

#include <memory.h>

extern "C" uint32_t get_inst(uint32_t pc);

extern "C" uint32_t pmem_read(uint32_t addr, uint32_t size);

extern "C" void pmem_write(uint32_t addr, uint32_t size, uint32_t data);

extern "C" void inst_ebreak();