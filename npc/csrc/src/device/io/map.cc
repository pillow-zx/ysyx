#include <device.h>
#include <stdexcept>
#include <iostream>

uint8_t *io_space = nullptr;
uint8_t *p_space = nullptr;

std::function<uint8_t *(int)> new_space = [](int size) -> uint8_t * {
    uint8_t *p = p_space;
    size = (size + PAGE_MASK) & ~PAGE_MASK; // Align to page size
    p_space += size;
    assert(p_space - io_space <= IO_SPACE_MAX);
    return p;
};

static void check_bound(IOMap *map, uint32_t addr) {
    if (map == nullptr) {
        throw std::runtime_error("IOMap is null");
    } else if (addr < map->low || addr >= map->high) {
        throw std::out_of_range("Address out of bounds");
    }
}

static void invoke_callback(std::function<void(uint32_t, int, bool)> callback, uint32_t addr, int len, bool is_write) {
    if (callback) {
        callback(addr, len, is_write);
    }
}

void init_map() {
    io_space = new uint8_t[IO_SPACE_MAX];
    assert(io_space != nullptr);
    p_space = io_space;
}

uint32_t map_read(IOMap *map, uint32_t addr, int len) {
    try {
        check_bound(map, addr);
    } catch (const std::out_of_range &e) {
        std::cerr << "Read error: " << e.what() << std::endl;
    }
    uint32_t offset = addr - map->low;
    invoke_callback(map->callback, addr, len, false);
    uint32_t ret = 0;
    switch (len) {
        case 1: ret = *(uint8_t *)((uint8_t *)map->space + offset); break;
        case 2: ret = *(uint16_t *)((uint8_t *)map->space + offset); break;
        case 4: ret = *(uint32_t *)((uint8_t *)map->space + offset); break;
    }
    return ret;
}

void map_write(IOMap *map, uint32_t addr, uint32_t data, int len) {
    try {
        check_bound(map, addr);
    } catch (const std::out_of_range &e) {
        std::cerr << "Write error: " << e.what() << std::endl;
    }
    uint32_t offset = addr - map->low;
    switch (len) {
        case 1: *(uint8_t *)((uint8_t *)map->space + offset) = data; break;
        case 2: *(uint16_t *)((uint8_t *)map->space + offset) = data; break;
        case 4: *(uint32_t *)((uint8_t *)map->space + offset) = data; break;
    }
    invoke_callback(map->callback, offset, len, true);
}
