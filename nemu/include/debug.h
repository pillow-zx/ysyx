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

// LOG的输出为 蓝色的format + 程序文件名 + 行号 + 函数名
#define Log(format, ...) \
    _Log(ANSI_FMT("[%s:%d %s] " format, ANSI_FG_BLUE) "\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__)

// __LINE__  ：当前程序行的行号，表示为十进制整型常量
// __FILE__  ：当前源文件名，表示字符串型常量
// __DATE__ ：转换的日历日期，表示为Mmm dd yyyy 形式的字符串常量，Mmm是由asctime产生的。
// __TIME__  ：转换的时间，表示"hh:mm:ss"形式的字符串型常量，是有asctime产生的。（asctime貌似是指的一个函数）
// __STDC__ ：编辑器为ISO兼容实现时位十进制整型常量
// __STDC_VERSION__  ：如何实现复合C89整部1，则这个宏的值为19940SL；如果实现符合C99，则这个宏的值为199901L；否则数值是未定义
// __STDC_EOBTED__ ：(C99)实现为宿主实现时为1,实现为独立实现为0
// __STDC_IEC_559__ ：(C99)浮点数实现复合IBC 60559标准时定义为1，否者数值是未定义
// __STDC_IEC_559_COMPLEX__： (C99)复数运算实现复合IBC 60559标准时定义为1，否者数值是未定义
// __STDC_ISO_10646__ ：(C99)定义为长整型常量，yyyymmL表示wchar_t值复合ISO 10646标准及其指定年月的修订补充，否则数值未定义


/* 预处理器从外到内处理宏定义 */
// 使用 MUXDEF 宏根据 CONFIG_TARGET_AM 是否定义来选择两种不同的错误输出方式：
// 如果 CONFIG_TARGET_AM 已定义：使用 printf 直接输出fotmat到标准输出
// 如果 CONFIG_TARGET_AM 未定义：先刷新标准输出缓冲区，然后使用 fprintf 输出到标准错误

// 如果 CONFIG_TARGET_AM 未定义，则声明一个外部变量 log_fp 并刷新它。这用于在非抽象机环境下确保日志及时写入文件。

// assert 宏的功能是检查一个表达式是否为真（非零）：
// 如果表达式为真（非零）：程序继续正常执行
// 如果表达式为假（零）：
// 打印一条错误信息到标准错误流（stderr）
// 调用 abort() 函数终止程序执行
// 错误信息通常包含失败的表达式、源文件名和行号

// MUXDEF 宏用于根据条件选择不同的宏定义
// Assert 宏的在CONFIG_TARGET_AM下的定义时直接输出红色的信息，否则输出红色的错误信息到标准错误流
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

// panic 宏的功能是直接调用 Assert 宏，传入一个条件值为0（假），表示程序发生了错误
#define panic(format, ...) Assert(0, format, ## __VA_ARGS__)

#define TODO() panic("please implement me")

#endif
