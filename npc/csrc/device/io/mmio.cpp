#include <device.h>
#include <map.h>

#define NR_MAP 16
static IOMap maps[NR_MAP] = {};
static int nr_maps = 0;

static IOMap *fetch_mmio_map(uint32_t addr) {
    int mapid = find_mapid_by_addr(maps, nr_maps, addr);
    return (mapid >= 0) ? &maps[mapid] : nullptr;
}

static void report_mmio_overlap(uint32_t addr, IOMap *map) {
    if (map != nullptr) {
        fprintf(stderr, "MMIO address 0x%08x overlaps with map %s [0x%08x - 0x%08x]\n", addr, map->name, map->low,
                map->high);
    } else {
        fprintf(stderr, "MMIO address 0x%08x is not mapped\n", addr);
    }
}

void add_mmio_map(const char *name, uint32_t addr, void *space, uint32_t len, io_callback_t callback) {
    assert(nr_maps < NR_MAP && "MMIO map limit reached");
    uint32_t left = addr, right = addr + len - 1;
    for (int i = 0; i < nr_maps; i++) {
        if (maps[i].low <= right && maps[i].high >= left) {
            report_mmio_overlap(addr, &maps[i]);
            return;
        }
    }
    maps[nr_maps] = (IOMap){.name = name, .low = left, .high = right, .space = space, .callback = callback};
    std::cout << "Added MMIO map: " << name << " [0x" << std::hex << left << " - 0x" << right << "]\n";
    nr_maps++;
}

uint32_t mmio_read(uint32_t addr, int len) {
    assert(len >= 1 && len <= 4 && "Invalid read length");
    IOMap *map = fetch_mmio_map(addr);
    if (map == nullptr) {
        report_mmio_overlap(addr, nullptr);
        return 0;
    }
    return map_read(addr, len, map);
}

void mmio_write(uint32_t addr, int len, uint32_t data) {
    assert(len >= 1 && len <= 4 && "Invalid write length");
    IOMap *map = fetch_mmio_map(addr);
    if (map == nullptr) {
        report_mmio_overlap(addr, nullptr);
        return;
    }
    map_write(addr, len, data, map);
}