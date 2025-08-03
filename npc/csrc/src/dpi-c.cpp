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
    // 对于字节访问，直接读取字节并返回
    uint8_t byte_value = read_pmem_byte(addrs);
    std::cout << "Reading value " << std::hex << (int)byte_value << " from address: " << addrs << std::dec << std::endl;
    return (int)byte_value;
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

    // 检查是否为字节写入（值在0-255范围内）
    if (values <= 0xFF) {
        // 字节写入
        write_pmem_byte(addrs, (uint8_t)values);
        std::cout << "Wrote value: " << std::hex << values << " to address: " << addrs << std::dec << std::endl;
        std::cout << "Read value: " << std::hex << (int)read_pmem_byte(addrs) << std::dec << std::endl;
    } else {
        // 字写入（4字节）
        if (values > 0xFFFFFFFF) {
            std::cerr << "Error: Value out of bounds: " << values << std::endl;
            exit(1);
        }
        write_pmem()[(addrs - DEFAULT_MEM_START) / 4] = values;
        std::cout << "Wrote value: " << std::hex << values << " to address: " << addrs << std::dec << std::endl;
        std::cout << "Read value: " << std::hex << read_pmem(addrs) << std::dec << std::endl;
    }
}

// 新增：专用的字节读取函数
extern "C" int pmem_read_byte(int addr) {
    if (!npc_STATE) {
        std::cerr << "Error: Attempt to read memory while NPC is not in a valid state." << std::endl;
        exit(1);
    }
    uint32_t addrs = addr;
    if (addrs < DEFAULT_MEM_START || addrs >= DEFAULT_MEM_START + CONFIG_MSIZE) {
        std::cerr << "Error: Address out of bounds: " << addrs << std::endl;
        exit(1);
    }
    uint8_t byte_value = read_pmem_byte(addrs);
    std::cout << "Reading byte value " << std::hex << (int)byte_value << " from address: " << addrs << std::dec
              << std::endl;
    return (int)byte_value;
}

// 新增：专用的字节写入函数
extern "C" void pmem_write_byte(int addr, int value) {
    if (!npc_STATE) {
        std::cerr << "Error: Attempt to write memory while NPC is not in a valid state." << std::endl;
        exit(1);
    }
    uint32_t addrs = (uint32_t)addr;
    uint8_t byte_value = (uint8_t)(value & 0xFF);
    if (addrs < DEFAULT_MEM_START || addrs >= DEFAULT_MEM_START + CONFIG_MSIZE) {
        std::cerr << "Error: Address out of bounds: " << addrs << std::endl;
        exit(1);
    }
    write_pmem_byte(addrs, byte_value);
    std::cout << "Wrote byte value: " << std::hex << (int)byte_value << " to address: " << addrs << std::dec
              << std::endl;
}

// 新增：专用的字读取函数
extern "C" int pmem_read_word(int addr) {
    if (!npc_STATE) {
        std::cerr << "Error: Attempt to read memory while NPC is not in a valid state." << std::endl;
        exit(1);
    }
    uint32_t addrs = addr;
    if (addrs < DEFAULT_MEM_START || addrs >= DEFAULT_MEM_START + CONFIG_MSIZE) {
        std::cerr << "Error: Address out of bounds: " << addrs << std::endl;
        exit(1);
    }
    uint32_t word_value = read_pmem(addrs);
    std::cout << "Reading word value " << std::hex << word_value << " from address: " << addrs << std::dec << std::endl;
    return (int)word_value;
}

// 新增：专用的字写入函数
extern "C" void pmem_write_word(int addr, int value) {
    if (!npc_STATE) {
        std::cerr << "Error: Attempt to write memory while NPC is not in a valid state." << std::endl;
        exit(1);
    }
    uint32_t values = (uint32_t)value;
    uint32_t addrs = (uint32_t)addr;
    if (addrs < DEFAULT_MEM_START || addrs >= DEFAULT_MEM_START + CONFIG_MSIZE) {
        std::cerr << "Error: Address out of bounds: " << addrs << std::endl;
        exit(1);
    }
    write_pmem()[(addrs - DEFAULT_MEM_START) / 4] = values;
    std::cout << "Wrote word value: " << std::hex << values << " to address: " << addrs << std::dec << std::endl;
}

// 新增：专用的半字读取函数
extern "C" int pmem_read_halfword(int addr) {
    if (!npc_STATE) {
        std::cerr << "Error: Attempt to read memory while NPC is not in a valid state." << std::endl;
        exit(1);
    }
    uint32_t addrs = addr;
    if (addrs < DEFAULT_MEM_START || addrs >= DEFAULT_MEM_START + CONFIG_MSIZE) {
        std::cerr << "Error: Address out of bounds: " << addrs << std::endl;
        exit(1);
    }
    uint16_t halfword_value = read_pmem_halfword(addrs);
    std::cout << "Reading halfword value " << std::hex << halfword_value << " from address: " << addrs << std::dec
              << std::endl;
    return (int)halfword_value;
}

// 新增：专用的半字写入函数
extern "C" void pmem_write_halfword(int addr, int value) {
    if (!npc_STATE) {
        std::cerr << "Error: Attempt to write memory while NPC is not in a valid state." << std::endl;
        exit(1);
    }
    uint16_t values = (uint16_t)(value & 0xFFFF);
    uint32_t addrs = (uint32_t)addr;
    if (addrs < DEFAULT_MEM_START || addrs >= DEFAULT_MEM_START + CONFIG_MSIZE) {
        std::cerr << "Error: Address out of bounds: " << addrs << std::endl;
        exit(1);
    }
    write_pmem_halfword(addrs, values);
    std::cout << "Wrote halfword value: " << std::hex << values << " to address: " << addrs << std::dec << std::endl;
}