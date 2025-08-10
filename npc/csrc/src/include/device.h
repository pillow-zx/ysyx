#pragma once

#include <macro.h>
#include <stdint.h>
#include <stddef.h>
#include <cassert>
#include <functional>
#include <iostream>

struct IOMap {
    const char *name;
    uint32_t low;
    uint32_t high;
    void *space;
    std::function<void(uint32_t, int, bool)> callback;
};

static inline bool map_inside(IOMap *map, uint32_t addr) {
    return map != nullptr && addr >= map->low && addr < map->high;
}

static inline int find_mapid_by_addr(IOMap *maps, int size, uint32_t addr) {
    // std::cout << "Searching for map with address: " << std::hex << addr << std::endl;
    // std::cout << "Total maps: " << size << std::endl;
    for (int i = 0; i < size; i++) {
        if (map_inside(maps + i, addr)) {
            return i;
        }
    }
    return -1;
}

typedef void (*alarm_handler_t)();
void add_alarm_handle(alarm_handler_t func);

extern uint8_t *io_space;
extern uint8_t *p_space;

extern std::function<uint8_t *(int)> new_space;

void add_mmio_map(const char *name, uint32_t addr, void *space, uint32_t len, std::function<void(uint32_t, int, bool)> callback);

uint32_t mmio_read(uint32_t addr, int len);
void mmio_write(uint32_t addr, uint32_t data, int len);

void init_map();

uint32_t map_read(IOMap *map, uint32_t addr, int len);
void map_write(IOMap *map, uint32_t addr, uint32_t data, int len);

void init_serial();

void init_timer();

void init_alarm();

void init_device();