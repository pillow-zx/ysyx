#include <memory.h>
#include <macro.h>
#include <array>


static std::array<uint8_t, CONFIG_MSIZE> pmem __attribute__((aligned(4096)));

static bool in_pmem(uint32_t addr) {
    return addr >= DEFAULT_MEM_START && addr < DEFAULT_MEM_START + CONFIG_MSIZE;
}


std::array<uint32_t, CONFIG_MSIZE / 4> &write_pmem() {
    return reinterpret_cast<std::array<uint32_t, CONFIG_MSIZE / 4>&>(pmem);
}

uint32_t read_pmem(uint32_t addr) {
    ASSERT_WITH_MSG_0(in_pmem(addr), "READ_PMEM: Address out of bounds: " + std::to_string(addr));
    uint32_t offset = addr - DEFAULT_MEM_START;
    uint32_t *pmem_word_ptr = reinterpret_cast<uint32_t*>(pmem.data());
    uint32_t word_indx = offset / sizeof(uint32_t);
    ASSERT_WITH_MSG_0(word_indx < CONFIG_MSIZE / sizeof(uint32_t), "Word index out of bounds: " + std::to_string(word_indx));
    return pmem_word_ptr[word_indx];
}

// 新增：字节级读取函数
uint8_t read_pmem_byte(uint32_t addr) {
    ASSERT_WITH_MSG_0(in_pmem(addr), "READ_PMEM_BYTE: Address out of bounds: " + std::to_string(addr));
    uint32_t offset = addr - DEFAULT_MEM_START;
    return pmem[offset];
}

// 新增：字节级写入函数
void write_pmem_byte(uint32_t addr, uint8_t value) {
    ASSERT_WITH_MSG_0(in_pmem(addr), "WRITE_PMEM_BYTE: Address out of bounds: " + std::to_string(addr));
    uint32_t offset = addr - DEFAULT_MEM_START;
    pmem[offset] = value;
}

// 新增：半字级读取函数
uint16_t read_pmem_halfword(uint32_t addr) {
    ASSERT_WITH_MSG_0(in_pmem(addr), "READ_PMEM_HALFWORD: Address out of bounds: " + std::to_string(addr));
    ASSERT_WITH_MSG_0((addr & 1) == 0, "HALFWORD address must be aligned: " + std::to_string(addr));
    uint32_t offset = addr - DEFAULT_MEM_START;
    uint16_t *pmem_halfword_ptr = reinterpret_cast<uint16_t*>(pmem.data());
    uint32_t halfword_indx = offset / sizeof(uint16_t);
    return pmem_halfword_ptr[halfword_indx];
}

// 新增：半字级写入函数
void write_pmem_halfword(uint32_t addr, uint16_t value) {
    ASSERT_WITH_MSG_0(in_pmem(addr), "WRITE_PMEM_HALFWORD: Address out of bounds: " + std::to_string(addr));
    ASSERT_WITH_MSG_0((addr & 1) == 0, "HALFWORD address must be aligned: " + std::to_string(addr));
    uint32_t offset = addr - DEFAULT_MEM_START;
    uint16_t *pmem_halfword_ptr = reinterpret_cast<uint16_t*>(pmem.data());
    uint32_t halfword_indx = offset / sizeof(uint16_t);
    pmem_halfword_ptr[halfword_indx] = value;
}

void init_memory() {
    pmem.fill(0);
    PRINT_GREEN_0(std::string("Memory initialized with size: ") + std::to_string(CONFIG_MSIZE) + " bytes");
}