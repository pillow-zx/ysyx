#include <memory.h>
#include <macro.h>
#include <array>


static std::array<uint8_t, CONFIG_MSIZE> pmem __attribute__((aligned(4096)));

static bool in_pmem(uint32_t addr) {
    return addr < CONFIG_MSIZE;
}


std::array<uint32_t, CONFIG_MSIZE / 4> &write_pmem() {
    return reinterpret_cast<std::array<uint32_t, CONFIG_MSIZE / 4>&>(pmem);
}

uint32_t read_pmem(uint32_t addr) {
    ASSERT_WITH_MSG_0(in_pmem(addr), "Address out of bounds: " + std::to_string(addr));
    uint32_t *pmem_word_ptr = reinterpret_cast<uint32_t*>(pmem.data());
    uint32_t word_indx = addr / sizeof(uint32_t);
    ASSERT_WITH_MSG_0(word_indx < CONFIG_MSIZE / sizeof(uint32_t), "Word index out of bounds: " + std::to_string(word_indx));
    return pmem_word_ptr[word_indx];
}

void init_memory() {
    pmem.fill(0);
    PRINT_GREEN_0(std::string("Memory initialized with size: ") + std::to_string(CONFIG_MSIZE) + " bytes");
}