#include <am.h>
#include <nemu.h>

void __am_timer_init() {}

#define RTC_ADDR_LOW RTC_ADDR
#define RTC_ADDR_HIGH (RTC_ADDR + 4)

/* 获取系统运行时间 */
void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
    uint32_t low, high1, high2;

    do {
        high1 = inl(RTC_ADDR_HIGH);
        low = inl(RTC_ADDR_LOW);
        high2 = inl(RTC_ADDR_HIGH);
    } while (high1 != high2);  // 确保读取的高32位一致, 避免读取到不一致的值

    uptime->us = ((uint64_t)high1 << 32) | low;  // 将高32位和低32位合并为64位
}
  

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
    rtc->second = 0;
    rtc->minute = 0;
    rtc->hour = 0;
    rtc->day = 0;
    rtc->month = 0;
    rtc->year = 1900;
}
