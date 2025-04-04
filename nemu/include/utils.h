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

#ifndef __UTILS_H__
#define __UTILS_H__

#include <common.h>

// ----------- state -----------
//          0            1          2         3           4
enum { NEMU_RUNNING, NEMU_STOP, NEMU_END, NEMU_ABORT, NEMU_QUIT };

typedef struct {
    int state;          // 表示 NEMU 模拟器的当前状态。
    vaddr_t halt_pc;    // 表示 NEMU 模拟器在暂停时的虚拟地址。
    uint32_t halt_ret;  // 表示 NEMU 模拟器在暂停时的返回值。
} NEMUState;

extern NEMUState nemu_state;

// ----------- timer -----------

uint64_t get_time();

// ----------- log -----------

#define ANSI_FG_BLACK   "\33[1;30m"     // 黑色
#define ANSI_FG_RED     "\33[1;31m"     // 红色
#define ANSI_FG_GREEN   "\33[1;32m"     // 绿色
#define ANSI_FG_YELLOW  "\33[1;33m"     // 黄色
#define ANSI_FG_BLUE    "\33[1;34m"     // 蓝色
#define ANSI_FG_MAGENTA "\33[1;35m"     // 紫色
#define ANSI_FG_CYAN    "\33[1;36m"     // 青色
#define ANSI_FG_WHITE   "\33[1;37m"     // 白色
#define ANSI_BG_BLACK   "\33[1;40m"     // 黑色背景
#define ANSI_BG_RED     "\33[1;41m"     // 红色背景
#define ANSI_BG_GREEN   "\33[1;42m"     // 绿色背景
#define ANSI_BG_YELLOW  "\33[1;43m"     // 黄色背景
#define ANSI_BG_BLUE    "\33[1;44m"     // 蓝色背景
#define ANSI_BG_MAGENTA "\33[1;45m"     // 紫色背景
#define ANSI_BG_CYAN    "\33[1;46m"     // 青色背景
#define ANSI_BG_WHITE   "\33[1;47m"     // 白色背景
#define ANSI_NONE       "\33[0m"        // 关闭所有属性

#define ANSI_FMT(str, fmt) fmt str ANSI_NONE // ANSI_FMT(str, fmt) 是一个宏定义，用于格式化字符串的输出

#define log_write(...) IFDEF(CONFIG_TARGET_NATIVE_ELF, \
  do { \
    extern FILE* log_fp; \
    extern bool log_enable(); \
    if (log_enable() && log_fp != NULL) { \
      fprintf(log_fp, __VA_ARGS__); \
      fflush(log_fp); \
    } \
  } while (0) \
)

#define _Log(...) \
  do { \
    printf(__VA_ARGS__); \
    log_write(__VA_ARGS__); \
  } while (0)


#endif
