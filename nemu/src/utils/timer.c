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
#include MUXDEF(CONFIG_TIMER_GETTIMEOFDAY, <sys/time.h>, <time.h>)

IFDEF(CONFIG_TIMER_CLOCK_GETTIME,
    static_assert(CLOCKS_PER_SEC == 1000000, "CLOCKS_PER_SEC != 1000000"));
IFDEF(CONFIG_TIMER_CLOCK_GETTIME,
    static_assert(sizeof(clock_t) == 8, "sizeof(clock_t) != 8"));

static uint64_t boot_time = 0;

/* 获取当前时间 */
static uint64_t get_time_internal() {
#if defined(CONFIG_TARGET_AM)
    uint64_t us = io_read(AM_TIMER_UPTIME).us;
#elif defined(CONFIG_TIMER_GETTIMEOFDAY)
    struct timeval now;
    gettimeofday(&now, NULL);
    uint64_t us = now.tv_sec * 1000000 + now.tv_usec;
#else
    /* 对struct timespec的详细说明见：file:///home/waysorry/user/NemuNote/timespec.md */
    struct timespec now;
    /* 对clock_gettime的详细说明见: file:///home/waysorry/user/NemuNote/clock_gettime.md */
    clock_gettime(CLOCK_MONOTONIC_COARSE, &now); // 从低精度但高效的单调时钟获取当前时间并存储在now中
    /* 秒数乘以1000000转换为微秒，纳秒除以1000转换为微秒，将二者相加得到总微秒数 */
    uint64_t us = now.tv_sec * 1000000 + now.tv_nsec / 1000;
#endif
    return us; //返回当前时间
}

/* 获取时间差 */
uint64_t get_time() {
    if (boot_time == 0) boot_time = get_time_internal();
    uint64_t now = get_time_internal();
    return now - boot_time;
}

void init_rand() {
    // 根据当前时间初始化随机数种子
    srand(get_time_internal());
}
