#include <Vcore.h>


static CORE_NAME dut;

static void single_cycle() {
    dut.clk = 0; dut.eval();
    dut.clk = 1; dut.eval();
}

static void reset(int n) {
    dut.rst = 1;
    while (n-- > 0) single_cycle();
    dut.rst = 0;
}

int main() {
    reset(10);

    while(1) {
        single_cycle();
    }

    return 0;
}