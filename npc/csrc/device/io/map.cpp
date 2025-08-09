#include <device.h>
#include <map.h>

#define IO_SPACE_MAX (32 * 1024 * 1024)  // 32MB
#define PAGE_SHIFT   12                  // 4KB pages
#define PAGE_SIZE    (1ul << PAGE_SHIFT) // 4KB
#define PAGE_MASK    (PAGE_SIZE - 1)     // 0xFFF

static uint8_t *io_space = nullptr;
static uint8_t *p_space = nullptr;

uint8_t *new_space(int size) {
    uint8_t *p = p_space;
    size = (size + PAGE_MASK) & ~PAGE_MASK; // Align to page size
    p_space += size;
    assert(p_space - io_space <= IO_SPACE_MAX && "IO space overflow");
    return p;
}

static void check_bound(IOMap *map, uint32_t addr) {
    if (map == nullptr) {
        throw std::runtime_error("IOMap is null");
    } else {
        if (addr < map->low || addr > map->high) {
            throw std::out_of_range("Address out of bounds");
        }
    }
}

static void invoke_callback(io_callback_t callback, uint32_t addr, int len, bool is_write) {
    if (callback) {
        callback(addr, len, is_write);
    }
}

void init_map() {
    io_space = new uint8_t[IO_SPACE_MAX];
    assert(io_space != nullptr && "Failed to allocate IO space");
    p_space = io_space;
}

uint32_t map_read(uint32_t addr, int len, IOMap *map) {
    assert(len >= 1 && len <= 8 && "Invalid read length");
    check_bound(map, addr);
    uint32_t offset = addr - map->low;
    invoke_callback(map->callback, offset, len, false);
    uint32_t ret = 0;
    switch (len) {
        case 1: ret = *(uint8_t *)((uint8_t *)map->space + offset); break;
        case 2: ret = *(uint16_t *)((uint8_t *)map->space + offset); break;
        case 4: ret = *(uint32_t *)((uint8_t *)map->space + offset); break;
        default: throw std::invalid_argument("Invalid read length");
    }
    return ret;
}

void map_write(uint32_t addr, int len, uint32_t data, IOMap *map) {
    assert(len >= 1 && len <= 8 && "Invalid write length");
    check_bound(map, addr);
    uint32_t offset = addr - map->low;
    switch (len) {
        case 1: *(uint8_t *)((uint8_t *)map->space + offset) = data; break;
        case 2: *(uint16_t *)((uint8_t *)map->space + offset) = data; break;
        case 4: *(uint32_t *)((uint8_t *)map->space + offset) = data; break;
        default: throw std::invalid_argument("Invalid write length");
    }
    invoke_callback(map->callback, offset, len, true);
}