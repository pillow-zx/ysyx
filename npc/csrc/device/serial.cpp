#include <device.h>
#include <map.h>

#define CH_OFFSET 0

static uint8_t *serial_base = nullptr;

static void serial_puts(char ch) {
    std::cout << ch;
}

static void serial_io_handler(uint32_t offset, int len, bool is_write) {
    assert(len == 1);
    switch (offset) {
        case CH_OFFSET: if (is_write) serial_puts(serial_base[0]); break;
        default: LOG_ERROR("Unknown serial offset: 0x%x", offset); break;
    }
}

void init_serial() {
    serial_base = (uint8_t *)new_space(8);
    add_mmio_map("serial", SERIAL_MMIO, serial_base, 8, serial_io_handler);
}