#include <dpi-c.h>
#include <device.h>
#include <cpu.h>

auto is_mmio_addr = [](uint32_t addr) -> bool { return addr >= MMIO_START && addr <= MMIO_END; };

extern "C" uint32_t get_inst(uint32_t pc) {
    return memory.get_inst(pc);
}

extern "C" uint32_t pmem_read(uint32_t addr, uint32_t size) {
    if (is_mmio_addr(addr)) {
        switch (size) {
            case 1: return mmio_read(addr, 1) & 0xff; break;
            case 2: return mmio_read(addr, 2) & 0xffff; break;
            case 4: return mmio_read(addr, 4); break;
        }
    }
    return memory.read_data(addr, size);
}

extern "C" void pmem_write(uint32_t addr, uint32_t size, uint32_t data) {
    if (is_mmio_addr(addr)) {
        switch (size) {
            case 1: mmio_write(addr, data & 0xff, 1); break;
            case 2: mmio_write(addr, data & 0xffff, 2); break;
            case 4: mmio_write(addr, data, 4); break;
        }
    } else {
        memory.write_data(addr, size, data);
    }
}

extern "C" void inst_ebreak() {
    npc_state.set_state(EXIT);
}
