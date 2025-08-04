/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#ifndef __DEVICE_MAP_H__
#define __DEVICE_MAP_H__

#include <cpu/difftest.h>

typedef void (*io_callback_t)(uint32_t, int, bool);
uint8_t *new_space(int size);


/* IOMap 是一个初始化结构体的宏，用于设置 I/O 映射的属性。
 * 它将名称、地址范围（低地址和高地址）、地址空间以及回调函数绑定到对应的字段中，以便在 I/O 操作中使用。
 */
typedef struct {
    const char *name; // 端口名
    // we treat ioaddr_t as paddr_t here
    paddr_t low;            // 内存起始地址
    paddr_t high;           // 内存结束地址
    void *space;            // 映射的内存空间
    io_callback_t callback; // I/O 操作回调函数, 就是lambda表达式
} IOMap;

static inline bool map_inside(IOMap *map, paddr_t addr) {
    return (addr >= map->low && addr <= map->high);
}

static inline int find_mapid_by_addr(IOMap *maps, int size, paddr_t addr) {
    int i;
    for (i = 0; i < size; i++) {
        if (map_inside(maps + i, addr)) {
            difftest_skip_ref();
            return i;
        }
    }
    return -1;
}

void add_pio_map(const char *name, ioaddr_t addr, void *space, uint32_t len, io_callback_t callback);
void add_mmio_map(const char *name, paddr_t addr, void *space, uint32_t len, io_callback_t callback);

word_t map_read(paddr_t addr, int len, IOMap *map);
void map_write(paddr_t addr, int len, word_t data, IOMap *map);

#endif
