#include <ctime>
#include <device.h>

static uint32_t *rtc_port_base = nullptr;

static uint64_t boot_time = 0;

static uint64_t get_time_internal() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

uint64_t get_time() {
    if (boot_time == 0)
        boot_time = get_time_internal();
    uint64_t now = get_time_internal();
    return now - boot_time;
}

auto rtc_io_callback = [](uint32_t offset, int len, bool is_write) {
    uint32_t offsets = offset - RTC_MMIO;
    assert(offsets == 0 || offsets == 4);
    if (!is_write && offsets == 4) {
        uint64_t us = get_time();
        rtc_port_base[0] = (uint32_t)us;
        rtc_port_base[1] = us >> 32;
    }
};

static void timer_intr() {}

void init_timer() {
    rtc_port_base = (uint32_t *)new_space(8);
    add_mmio_map("rtc", RTC_MMIO, rtc_port_base, 8, rtc_io_callback);
    add_alarm_handle(timer_intr);
}
