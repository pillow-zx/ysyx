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

#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>


#if defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
// pmem 是一个大小为 CONFIG_MSIZE（大小为128MB） 的静态数组，代表物理内存
// PG_ALIGN 是一个属性修饰符，确保数组按页边界对齐（ 4KB 对齐）
// = {} 将数组初始化为全零
/* 定义一个128MB大小的物理内存空间，并规定按页边界对齐，大小为4kb, 将数组内元素全部初始化为0 */
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
// 当这个数组在内存中分配时：
// 内存布局示意：
// 0x0000 +-----------------+
//        |     其他数据     |
// 0x1000 +-----------------+ <- pmem（4KB对齐）
//        |                 |
//        |     物理内存     |
//        |    (128MB)     |
//        |                 |
//        +-----------------+
#endif

/* 这两个函数实现了 NEMU 中客户机物理地址和主机虚拟地址之间的转换，是虚拟机内存模拟的核心组件。*/
// 原式为： uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
uint8_t *guest_to_host(paddr_t paddr) {
    return pmem + (paddr - CONFIG_MBASE);
}
// 将客户机（模拟的系统）物理地址转换为主机（运行 NEMU 的系统）虚拟地址。
// paddr_t paddr：客户机物理地址
// uint8_t*：对应的主机虚拟地址指针
// 主机地址 = pmem起始地址 + (客户机地址 - CONFIG_MBASE)
paddr_t host_to_guest(uint8_t *haddr) {
    return haddr - pmem + CONFIG_MBASE;
}
// 将主机虚拟地址转换为客户机物理地址。
// uint8_t* haddr：主机虚拟地址指针
// paddr_t：对应的客户机物理地址
// 客户机地址 = 主机地址 - pmem起始地址 + CONFIG_MBASE

static word_t pmem_read(paddr_t addr, int len) {
    word_t ret = host_read(guest_to_host(addr), len);
    return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
    host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
    panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD, addr,
          PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

/* 初始化内存 */
void init_mem() {
#if defined(CONFIG_PMEM_MALLOC)
    pmem = malloc(CONFIG_MSIZE);
    assert(pmem);
#endif
    IFDEF(CONFIG_MEM_RANDOM, memset(pmem, rand(), CONFIG_MSIZE));
    Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

extern char iringbuf[64][128];
extern int iringbuf_head;

#ifdef CONFIG_ITRACE
static void iringbuf_show() {
    printf("\033[34m%s\033[0m\n", iringbuf[iringbuf_head - 1]);
    for (int i = 0; i < iringbuf_head; i++) {
        printf("\033[34m%s\033[0m\n", iringbuf[i]);
    }
}
#endif

#ifdef CONFIG_MTRACE
static void mtrace(char *str, paddr_t addr, int len) {
    printf("\033[1;34m[memory trace] %s = " FMT_PADDR ", len = %d\033[0m\n", str, addr, len);
}
#endif

/* 读取物理地址的数据 */
word_t paddr_read(paddr_t addr, int len) {
    /* 首先检查地址是否在物理内存范围内（使用 likely 提示编译器这是常见情况） */
    /* 如果是，调用 pmem_read，如果不是且启用了设备模拟，尝试从 MMIO（内存映射 I/O）读取，如果以上都不是，报告越界错误*/
    if (likely(in_pmem(addr))) {
        IFDEF(CONFIG_MTRACE, mtrace("paddr_read", addr, len));
        return pmem_read(addr, len);
    }
    IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
    IFDEF(CONFIG_ITRACE, iringbuf_show());
    out_of_bound(addr);
    return 0;
}

/* 向物理地址写入数据 */
void paddr_write(paddr_t addr, int len, word_t data) {
    /* 首先检查地址是否在物理内存范围内 */
    /* 如果是，调用 pmem_write,如果不是且启用了设备模拟，尝试写入 MMIO,如果以上都不是，报告越界错误*/
    if (likely(in_pmem(addr))) {
        IFDEF(CONFIG_MTRACE, mtrace("paddr_write", addr, len));
        pmem_write(addr, len, data);
        return;
    }
    IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
    IFDEF(CONFIG_ITRACE, iringbuf_show());
    out_of_bound(addr);
}
