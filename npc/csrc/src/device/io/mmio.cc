#include "device.h"
#include <iostream>

#define NR_MAPS 16
static IOMap maps[NR_MAPS] = {};
static int nr_maps = 0;

static IOMap *fetch_mmio_map(uint32_t addr) {
    int mapid = find_mapid_by_addr(maps, nr_maps, addr);
    return (mapid >= 0) ? &maps[mapid] : nullptr;
}

static void report_mmio_overlap(uint32_t addr, IOMap *map) {
    if (map != nullptr) {
        std::cerr << "MMIO address " << std::hex << addr << " overlaps with existing map: " << map->name << " ["
                  << std::hex << map->low << ", " << map->high << ")" << std::endl;
    } else {
        std::cerr << "MMIO address " << std::hex << addr << " does not match any existing map." << std::endl;
    }
}

void add_mmio_map(const char *name, uint32_t addr, void *space, uint32_t len,
                  std::function<void(uint32_t, int, bool)> callback) {
    assert(nr_maps < NR_MAPS);
    uint32_t left = addr, right = addr + len - 1;
    for (int i = 0; i < nr_maps; i++) {
        if (maps[i].low <= right && maps[i].high >= left) {
            report_mmio_overlap(addr, &maps[i]);
            return;
        }
    }
    maps[nr_maps++] = (IOMap){name, left, right, space, callback};
    std::cout << "Added MMIO map: " << name << " [" << std::hex << left << ", " << right << "]" << std::endl;
}

uint32_t mmio_read(uint32_t addr, int len) {
    IOMap *map = fetch_mmio_map(addr);
    if (map == nullptr) {
        std::cerr << "MMIO read error: address " << std::hex << addr << " not mapped." << std::endl;
        return 0;
    }
    return map_read(map, addr, len);
}

void mmio_write(uint32_t addr, uint32_t data, int len) {
    IOMap *map = fetch_mmio_map(addr);
    if (map == nullptr) {
        std::cerr << "MMIO write error: address " << std::hex << addr << " not mapped." << std::endl;
        return;
    }
    map_write(map, addr, data, len);
}
