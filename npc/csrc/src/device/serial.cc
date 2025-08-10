#include <iostream>
#include <device.h>

#define CH_OFFSET 0

static uint8_t *serial_base = nullptr;

static void serial_puts(char ch) {
    printf("%c", ch);
    fflush(stdout);
}

auto serial_io_callback = [](uint32_t offset, int len, bool is_write) {
    assert(len == 1);
    // std::cout << "Serial IO callback at offset: " << std::hex << offset << ", is_write: " << is_write << std::endl;
    switch (offset) {
        case CH_OFFSET:
            if (is_write) {
                serial_puts(*serial_base);
            }
            break;
        default: std::cout << "Unknown serial offset: " << std::hex << offset << std::endl; break;
    }
};

void init_serial() {
    serial_base = (uint8_t *)new_space(8);
    add_mmio_map("serial", SERIAL_MMIO, serial_base, 8, serial_io_callback);
}