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

#include <common.h>

/* extern表示变量在其他源文件中定义，当前仅为引用 */
/* 前缀 g_ 是一种命名约定，帮助开发人员立即识别出这是一个全局变量 */
extern uint64_t g_nr_guest_inst; // 引用全局变量g_nr_guest_inst，变量表示客户机指令数，这个变量用于跟踪和记录模拟器执行的指令数量

#ifndef CONFIG_TARGET_AM
FILE* log_fp = NULL;

/* 初始化日志文件 */
void init_log(const char* log_file) {                   // 传入日志文件名和路径
    // stdin：标准输入流（键盘输入）
    // stdout：标准输出流（屏幕输出）
    // stderr：标准错误流（错误消息输出）
    log_fp = stdout;                                    // 默认输出到标准输出
    if (log_file != NULL) {
        FILE* fp = fopen(log_file, "w");                // 以写入模式打开文件
        Assert(fp, "Can not open '%s'", log_file);      // 检查文件是否成功打开
        log_fp = fp;
    }
    Log("Log is written to %s", log_file ? log_file : "stdout");
}

bool log_enable() {
    return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) &&
        (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}
#endif
