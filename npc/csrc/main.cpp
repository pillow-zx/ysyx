#include <iostream>
#include <vector>
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vysyx_25060173_core.h"

static unsigned int RUN = 1;
static unsigned int pc_counter = 0;

static const std::vector<unsigned int> test_data = {
    0b00000000000000001000000100010011, // addi x1, x0, 1
    0b00000000000100000000000001110011, // ebreak
};

static void single_cycle(Vysyx_25060173_core* core) {
    core->clk = 0;
    core->inst = test_data[pc_counter]; // Fetch instruction based on PC
    core->eval();
    core->clk = 1;
    core->eval();
    
    // Only advance pc_counter if we're still running and not at the last instruction
    if (RUN && pc_counter < test_data.size() - 1) {
        pc_counter++;
    }
}

static void reset(Vysyx_25060173_core* core, int n) {
    core->reset = 0;  // reset is active low in the module
    while (n-- > 0) {
        single_cycle(core);
    }
    core->reset = 1;  // release reset (inactive high)
}

extern "C" void ebreak_handler() {
    if (RUN) {  // Only print once
        std::cout << "EBREAK instruction encountered. Stopping simulation." << std::endl;
        RUN = 0;  // Stop the simulation
    }
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vysyx_25060173_core* core = new Vysyx_25060173_core;

    reset(core, 10);

    while (RUN) {
        single_cycle(core);
    }

    delete core;
    return 0;
}