#pragma once

extern "C" void ebreak_handler();

extern "C" int pmem_read(int addr);

extern "C" void pmem_write(int addr, int value);

extern "C" unsigned int inst_read(unsigned int addr);
