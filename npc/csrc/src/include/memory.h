#pragma once

#include <cpu.h>

class Memory {
  private:
    std::vector<uint8_t> memory; // Memory storage

    bool load_program(const std::string &filename) {
        /* Open the file */
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return false;
        }

        /* Get the file size */
        file.seekg(0, std::ios::end);
        size_t file_size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (file_size > MEMORY_SIZE) {
            std::cerr << "File size exceeds memory size." << std::endl;
            return false;
        }

        /* Read the file into memory */
        file.read(reinterpret_cast<char *>(memory.data()), file_size);
        return true;
    }

  public:
    Memory() {
        memory.resize(MEMORY_SIZE, 0); // Initialize memory with zeros
    }

    std::vector<uint8_t> get_memory() {
        return memory;
    }

    bool init(const std::string &filename) {
        if (!load_program(filename)) {
            return false;
        }
        return true;
    }

    uint32_t get_inst(uint32_t pc) {
        if (pc >= MEMORY_START && pc < MEMORY_START + MEMORY_SIZE) {
            return *reinterpret_cast<uint32_t *>(&memory[pc - MEMORY_START]);
        }
        return 0;
    }

    uint32_t read_data(uint32_t addr, uint32_t size) {
        uint32_t offset = addr - MEMORY_START;

        if (offset >= MEMORY_SIZE)
            return 0;

        uint32_t data = 0;
        for (int i = 0; i < size; ++i) {
            data |= (memory[offset + i] << (i * 8));
        }
        return data;
    }

    void write_data(uint32_t addr, uint32_t size, uint32_t data) {
        uint32_t offset = addr - MEMORY_START;

        if (offset >= MEMORY_SIZE)
            return;

        for (int i = 0; i < size; ++i) {
            memory[offset + i] = (data >> (i * 8)) & 0xFF;
        }
    }
};

extern Memory memory; // Global memory instance
