#pragma once

extern "C" void ebreak_handler();

extern "C" int pmem_read(int addr);

extern "C" void pmem_write(int addr, int value);
