#include <device.h>

void init_device() {
    init_map();

    init_serial();

    init_timer();

    init_alarm();
}