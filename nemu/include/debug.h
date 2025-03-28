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

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <common.h>
#include <stdio.h>
#include <utils.h>

#define Log(format, ...) \
    _Log(ANSI_FMT("[%s:%d %s] " format, ANSI_FG_BLUE) "\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)

/* 预处理器从外到内处理宏定义 */
// 使用 MUXDEF 宏根据 CONFIG_TARGET_AM 是否定义来选择两种不同的错误输出方式：
// 如果 CONFIG_TARGET_AM 已定义：使用 printf 直接输出到标准输出
// 如果 CONFIG_TARGET_AM 未定义：先刷新标准输出缓冲区，然后使用 fprintf 输出到标准错误

// 如果 CONFIG_TARGET_AM 未定义，则声明一个外部变量 log_fp 并刷新它。这用于在非抽象机环境下确保日志及时写入文件。

// assert 宏的功能是检查一个表达式是否为真（非零）：
// 如果表达式为真（非零）：程序继续正常执行
// 如果表达式为假（零）：
// 打印一条错误信息到标准错误流（stderr）
// 调用 abort() 函数终止程序执行
// 错误信息通常包含失败的表达式、源文件名和行号
#define Assert(cond, format, ...) \
  do { \
    if (!(cond)) { \
    MUXDEF(CONFIG_TARGET_AM, \
        printf(ANSI_FMT(format, ANSI_FG_RED) "\n", ## __VA_ARGS__), \
        (fflush(stdout), fprintf(stderr, ANSI_FMT(format, ANSI_FG_RED) "\n", ##  __VA_ARGS__))); \
    IFNDEF(CONFIG_TARGET_AM, extern FILE* log_fp; fflush(log_fp)); \
    extern void assert_fail_msg(); \
    assert_fail_msg(); \
    assert(cond); \
    } \
  } while (0)

#define panic(format, ...) Assert(0, format, ## __VA_ARGS__)

#define TODO() panic("please implement me")

#endif
