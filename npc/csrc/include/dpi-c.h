#pragma once

extern "C" void ebreak_handler();

extern "C" int pmem_read(int addr);

extern "C" void pmem_write(int addr, int value);

extern "C" unsigned int inst_read(unsigned int addr);

extern "C" int pmem_read_byte(int addr);

extern "C" void pmem_write_byte(int addr, int value);

extern "C" int pmem_read_halfword(int addr);

extern "C" void pmem_write_halfword(int addr, int value);

extern "C" int pmem_read_word(int addr);

extern "C" void pmem_write_word(int addr, int value);
