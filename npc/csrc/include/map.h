#pragma once

#include <device.h>

typedef void (*io_callback_t)(uint32_t, int, bool);
uint8_t *new_space(int size); // 分配新的IO空间

typedef struct {
    const char *name; // 设备名称
    uint32_t low;
    uint32_t high;
    void *space;
    io_callback_t callback; // IO回调函数
} IOMap;

static inline bool map_inside(IOMap *map, uint32_t addr) {
    return (addr >= map->low && addr <= map->high);
}

static inline int find_mapid_by_addr(IOMap *maps, int size, uint32_t addr) {
    for (int i = 0; i < size; i++) {
        if (map_inside(maps + i, addr)) {
            return i;
        }
    }
    return -1;
}


void add_mmio_map(const char *name, uint32_t addr, void *space, uint32_t len, io_callback_t callback);

void map_write(uint32_t addr, int len, uint32_t data, IOMap *map);
uint32_t map_read(uint32_t addr, int len, IOMap *map);
