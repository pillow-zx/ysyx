#include "dpi-c.h"
#include "cpu.h"
#include "macro.h"
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
    uint32_t addrs = addr;
    // std::cout << "Reading memory at address: " << std::hex << addrs << std::dec << std::endl;
    if (addrs < DEFAULT_MEM_START || addrs >= DEFAULT_MEM_START + CONFIG_MSIZE) {
        std::cerr << "Error: Address out of bounds: " << addrs << std::endl;
        exit(1);
    }
    std::cout << "Reading value " << std::hex << read_pmem(addrs) << " from address: " << addrs << std::dec << std::endl;
    return read_pmem(addrs);
}

extern "C" unsigned int inst_read(unsigned int addr) {
    if (!npc_STATE) {
        std::cerr << "Error: Attempt to read instruction while NPC is not in a valid state." << std::endl;
        exit(1);
    }
    uint32_t addrs = addr;
    // std::cout << "Reading instruction at address: " << std::hex << addrs << std::dec << std::endl;
    if (addrs < DEFAULT_MEM_START || addrs >= DEFAULT_MEM_START + CONFIG_MSIZE) {
        std::cerr << "Error: Address out of bounds: " << addrs << std::endl;
        exit(1);
    }
    return read_pmem(addrs); // Assuming inst_read is similar to pmem_read
}

extern "C" void pmem_write(int addr, int value) {
    if (!npc_STATE) {
        std::cerr << "Error: Attempt to write memory while NPC is not in a valid state." << std::endl;
        exit(1);
    }
    uint32_t values = (uint32_t)value;
    uint32_t addrs = (uint32_t)addr;
    // std::cout << "Writing value: " << std::hex << values << " to address: " << addrs << std::dec << std::endl;
    if (addrs < DEFAULT_MEM_START || addrs >= DEFAULT_MEM_START + CONFIG_MSIZE) {
        std::cerr << "Error: Address out of bounds: " << addrs << std::endl;
        exit(1);
    }
    if (values > 0xFFFFFFFF) {
        std::cerr << "Error: Value out of bounds: " << values << std::endl;
        exit(1);
    }
    write_pmem()[(addrs - DEFAULT_MEM_START) / 4] = values; // Convert to offset from 0x80000000 base address
    std::cout << "Wrote value: " << std::hex << values << " to address: " << std::hex << addrs << std::dec << std::endl;
    std::cout << "Read value: " << std::hex << read_pmem(addrs) << std::dec << std::endl; // For debugging purposes
}