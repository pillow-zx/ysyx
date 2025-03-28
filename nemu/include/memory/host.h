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

#ifndef __MEMORY_HOST_H__
#define __MEMORY_HOST_H__

/* 此头文件定义了在主机内存中读取和写入数据的底层实用函数 */

#include <common.h>

/* 从主机内存中读取指定长度的数据，并将其作为 word_t 类型返回。*/
static inline word_t host_read(void* addr, int len) {
    // void *addr：要读取的内存地址，int len：要读取的字节数 (1, 2, 4, 或 8)
    switch (len) {
    case 1: return *(uint8_t*)addr;
    case 2: return *(uint16_t*)addr;
    case 4: return *(uint32_t*)addr;
    IFDEF(CONFIG_ISA64, case 8: return *(uint64_t*)addr);
    default: MUXDEF(CONFIG_RT_CHECK, assert(0), return 0);
    }
}

/* 将指定的数据值写入主机内存中的指定地址。 */
static inline void host_write(void* addr, int len, word_t data) {
    //void *addr：要写入的内存地址，int len：要写入的字节数 (1, 2, 4, 或 8)，word_t data：要写入的数据值
    switch (len) {
    case 1: *(uint8_t*)addr = data; return;
    case 2: *(uint16_t*)addr = data; return;
    case 4: *(uint32_t*)addr = data; return;
    IFDEF(CONFIG_ISA64, case 8: *(uint64_t*)addr = data; return);
    IFDEF(CONFIG_RT_CHECK, default: assert(0));
    }
}

// 这两个函数是 NEMU 内存模拟的底层实现：

// 1.类型安全的内存访问：

// 处理不同大小的内存访问 (1, 2, 4, 8 字节)
// 使用正确的类型进行读写，避免类型不匹配导致的问题
// 2.模拟体系结构的灵活性：

// 根据目标 ISA 的配置 (CONFIG_ISA64) 决定是否支持 64 位访问
// 可以轻松适应不同的模拟需求
// 3.与更高级别内存函数配合：

// 如前面看到的 pmem_read 和 pmem_write 函数会调用这些函数
// 形成了从客户机地址到主机内存的完整访问路径：
// 客户机地址 → guest_to_host → host_read/host_write → 实际内存访问

#endif
