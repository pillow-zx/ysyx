#include "dpi-c.h"
#include "cpu.h"
#include <memory.h>
#include <iostream>

extern "C" void ebreak_handler() {
    if (npc_STATE) {
        std::cout << "EBREAK instruction encountered at PC: " << std::hex << core->now_pc << std::dec << std::endl;
        npc_STATE = false; // Set the state to false to stop execution
    }
}

extern "C" int pmem_read(int addr) {
    if (!npc_STATE) {
        std::cerr << "Error: Attempt to read memory while NPC is not in a valid state." << std::endl;
        exit(1);
    }
    if (addr < 0 || addr >= CONFIG_MSIZE) {
        std::cerr << "Error: Address out of bounds: " << addr << std::endl;
        exit(1);
    }
    return read_pmem(addr);
}

extern "C" void pmem_write(int addr, int value) {
    if (!npc_STATE) {
        std::cerr << "Error: Attempt to write memory while NPC is not in a valid state." << std::endl;
        exit(1);
    }
    if (addr < 0 || addr >= CONFIG_MSIZE) {
        std::cerr << "Error: Address out of bounds: " << addr << std::endl;
        exit(1);
    }
    if (value < 0 || value > 0xFFFFFFFF) {
        std::cerr << "Error: Value out of bounds: " << value << std::endl;
        exit(1);
    }
    write_pmem()[addr / 4] = value; // Assuming addr is word-aligned
}