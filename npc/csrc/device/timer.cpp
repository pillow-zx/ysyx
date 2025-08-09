#include <device.h>
#include <cpu.h>
#include <alarm.h>
#include <map.h>

static uint32_t *rtc_port_base = nullptr;

static void rtc_io_handler(uint32_t offset, int len, bool is_write) {
    assert(offset == 0  || len == 4); // RTC typically uses 4-byte access
    // RTC IO handler implementation
    if (!is_write && offset == 4) {
        uint64_t us = get_time();
        rtc_port_base[0] = (uint32_t)us;
        rtc_port_base[1] = us >> 32;
    }
}

static void timer_intr() {
    if (npc_STATE) {
        extern void dev_raise_intr();
        dev_raise_intr();
    }
}


void init_timer() {
    rtc_port_base = (uint32_t *)new_space(8);
    add_mmio_map("rtc", RTC_MMIO, rtc_port_base, 8, rtc_io_handler);
    add_alarm_handle(timer_intr);
}