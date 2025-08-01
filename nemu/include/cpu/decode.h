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

#ifndef __CPU_DECODE_H__
#define __CPU_DECODE_H__

#include <isa.h>

typedef struct Decode {
    vaddr_t pc;                             // program counter   程序计数器
    vaddr_t snpc;                           // static next pc   静态下一个指令地址
    vaddr_t dnpc;                           // dynamic next pc  动态下一个指令地址
    ISADecodeInfo isa;                      // ISA解码信息
    IFDEF(CONFIG_ITRACE, char logbuf[128]); //  指令跟踪日志缓冲区
} Decode;

// --- pattern matching mechanism ---
// 将模式字符串中的0和1抽取到整型变量key中
// mask表示key的掩码
// shift则表示opcode距离最低位的比特数量, 用于帮助编译器进行优化
__attribute__((always_inline)) static inline void pattern_decode(const char *str, int len, uint64_t *key,
                                                                 uint64_t *mask, uint64_t *shift) {
    uint64_t __key = 0, __mask = 0, __shift = 0;
    // 在代码中通过goto语句来跳出循环
#define macro(i)                                                                                                       \
    if ((i) >= len)                                                                                                    \
        goto finish;                                                                                                   \
    else {                                                                                                             \
        char c = str[i];                                                                                               \
        if (c != ' ') {                                                                                                \
            Assert(c == '0' || c == '1' || c == '?', "invalid character '%c' in pattern string", c);                   \
            __key = (__key << 1) | (c == '1' ? 1 : 0);                                                                 \
            __mask = (__mask << 1) | (c == '?' ? 0 : 1);                                                               \
            __shift = (c == '?' ? __shift + 1 : 0);                                                                    \
        }                                                                                                              \
    }
    /* 由于__shift在判断过程中并没有进行移位操作,
     * 所以虽然在处理过程中遇到?__shift会增加,
     * 但一旦遇到非?字符, __shift会被重置为0.
     * 这意味着在处理过程中, 只有尾部连续的?会影响__shift的值.
     * 例如:
     *   str = "1001 ? 1"
     *   处理到最后一个字符时, __shift会被重置为0
     *   因此最终的__shift值为0, 而不是1.
    */

    // 通过宏展开的方式, 递归地处理字符串
    // 这种方式能够在编译时展开, 避免了运行时的循环开销
#define macro2(i)                                                                                                      \
    macro(i);                                                                                                          \
    macro((i) + 1)
#define macro4(i)                                                                                                      \
    macro2(i);                                                                                                         \
    macro2((i) + 2)
#define macro8(i)                                                                                                      \
    macro4(i);                                                                                                         \
    macro4((i) + 4)
#define macro16(i)                                                                                                     \
    macro8(i);                                                                                                         \
    macro8((i) + 8)
#define macro32(i)                                                                                                     \
    macro16(i);                                                                                                        \
    macro16((i) + 16)
#define macro64(i)                                                                                                     \
    macro32(i);                                                                                                        \
    macro32((i) + 32)
    macro64(0);
    panic("pattern too long");
#undef macro
finish:
    *key = __key >> __shift;
    *mask = __mask >> __shift;
    *shift = __shift;
}

__attribute__((always_inline)) static inline void pattern_decode_hex(const char *str, int len, uint64_t *key,
                                                                     uint64_t *mask, uint64_t *shift) {
    uint64_t __key = 0, __mask = 0, __shift = 0;
#define macro(i)                                                                                                       \
    if ((i) >= len)                                                                                                    \
        goto finish;                                                                                                   \
    else {                                                                                                             \
        char c = str[i];                                                                                               \
        if (c != ' ') {                                                                                                \
            Assert((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || c == '?',                                       \
                   "invalid character '%c' in pattern string", c);                                                     \
            __key = (__key << 4) | (c == '?' ? 0 : (c >= '0' && c <= '9') ? c - '0' : c - 'a' + 10);                   \
            __mask = (__mask << 4) | (c == '?' ? 0 : 0xf);                                                             \
            __shift = (c == '?' ? __shift + 4 : 0);                                                                    \
        }                                                                                                              \
    }

    macro16(0);
    panic("pattern too long");
#undef macro
finish:
    *key = __key >> __shift;
    *mask = __mask >> __shift;
    *shift = __shift;
}

// --- pattern matching wrappers for decode ---
// 使用do while(0)来包裹宏定义, 以确保宏的使用不会引起语法错误,并确保至少执行一次
#define INSTPAT(pattern, ...)                                                                                          \
    do {                                                                                                               \
        uint64_t key, mask, shift;                                                                                     \
        pattern_decode(pattern, STRLEN(pattern), &key, &mask, &shift);                                                 \
        if ((((uint64_t)INSTPAT_INST(s) >> shift) & mask) == key) {                                                    \
            INSTPAT_MATCH(s, ##__VA_ARGS__);                                                                           \
            goto *(__instpat_end);                                                                                     \
        }                                                                                                              \
    } while (0)

#define INSTPAT_START(name)                                                                                            \
    {                                                                                                                  \
        const void *__instpat_end = &&concat(__instpat_end_, name);
#define INSTPAT_END(name)                                                                                              \
    concat(__instpat_end_, name) :;                                                                                    \
    }

#endif
