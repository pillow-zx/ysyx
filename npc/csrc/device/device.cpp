#include <device.h>
#include <map.h>

void init_map();
void init_serial();
void init_timer();


void init_device() {
    init_map();
    init_serial();
    init_timer();
}