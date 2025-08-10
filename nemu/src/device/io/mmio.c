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

// 模拟内存映射 I/O 设备
#include <device/map.h>
#include <memory/paddr.h>

#define NR_MAP 16

/* 每种不同的设备都有一个 I/O 映射,
 * 每一个设备的 I/O 映射都需要一个 IOMap 结构体,
 * 这里创建一个 IOMap 结构体数组来来存储所有设备的 I/O 映射 */
static IOMap maps[NR_MAP] = {}; // I/O 映射数组, 用于存储所有的 I/O 映射
static int nr_map = 0;          // 当前 I/O 映射数量及索引

static IOMap *fetch_mmio_map(paddr_t addr) {
    int mapid = find_mapid_by_addr(maps, nr_map, addr);
    return (mapid == -1 ? NULL : &maps[mapid]);
}

/* 报告 MMIO 区域重叠 */
static void report_mmio_overlap(const char *name1, paddr_t l1, paddr_t r1, const char *name2, paddr_t l2, paddr_t r2) {
    panic("MMIO region %s@[" FMT_PADDR ", " FMT_PADDR "] is overlapped "
          "with %s@[" FMT_PADDR ", " FMT_PADDR "]",
          name1, l1, r1, name2, l2, r2);
}

/* device interface */
/* 添加设备接口, 添加内存映射空间 */
void add_mmio_map(const char *name, paddr_t addr, void *space, uint32_t len, io_callback_t callback) {
    assert(nr_map < NR_MAP);
    paddr_t left = addr, right = addr + len - 1;
    // 检查地址范围是否与 nemu 的物理内存范围重叠
    if (in_pmem(left) || in_pmem(right)) {
        report_mmio_overlap(name, left, right, "pmem", PMEM_LEFT, PMEM_RIGHT);
    }
    // 检查地址范围是否与现有的 I/O 映射空间重叠
    for (int i = 0; i < nr_map; i++) {
        if (left <= maps[i].high && right >= maps[i].low) {
            report_mmio_overlap(name, left, right, maps[i].name, maps[i].low, maps[i].high);
        }
    }

    // 添加新的 I/O 映射空间
    maps[nr_map] = (IOMap){.name = name, .low = addr, .high = addr + len - 1, .space = space, .callback = callback};
    Log("Add mmio map '%s' at [" FMT_PADDR ", " FMT_PADDR "]", maps[nr_map].name, maps[nr_map].low, maps[nr_map].high);

    nr_map++;
}

word_t mmio_read(paddr_t addr, int len) {
    assert(len >= 1 && len <= 4);
    IOMap *map = fetch_mmio_map(addr);
    if (map == NULL) {
        panic("MMIO address " FMT_PADDR " is not mapped", addr);
    }
    return map_read(addr, len, map);
}

void mmio_write(paddr_t addr, int len, word_t data) {
    assert(len >= 1 && len <= 4);
    IOMap *map = fetch_mmio_map(addr);
    if (map == NULL) {
        panic("MMIO address " FMT_PADDR " is not mapped", addr);
    }
    map_write(addr, len, data, map);
}
