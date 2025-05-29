#include <nvboard.h>
#include <Vtop.h>
#include "verilated.h"
#include "verilated_vcd_c.h"

void nvboard_bind_all_pins(Vtop *top);

static void single_cycle(Vtop *top) {
    top->clk = 0;
    top->eval();

    top->clk = 1;
    top->eval();
}

int main(int argc, char **argv) {
    // Verilated::commandArgs(argc, argv);
    // Verilated::traceEverOn(true);
    Vtop *top = new Vtop;

    // VerilatedVcdC *tfp = new VerilatedVcdC;
    // top->trace(tfp, 99);
    // tfp->open("wave.vcd");

    nvboard_bind_all_pins(top);
    nvboard_init();

    // uint64_t cycle = 0;
    while (true) {
        nvboard_update();
        // top->eval();
        single_cycle(top);
        // tfp->dump(cycle);
        // cycle++;
    }

    nvboard_quit();
    delete top;
    // tfp->close();
    // delete tfp;
    return 0;
}
