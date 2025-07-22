#include <iostream>
#include <vector>
#include <verilated.h>
#include <fstream>
#include "verilated_vcd_c.h"
#include "Vysyx_25060173_core.h"

static unsigned int run = 1;
static unsigned int sim_time = 0;
static uint32_t temp_pc = 0x80000000;  // Initial PC value
static uint32_t times = 0;

#define MAX_TIMES 10000
#define SUCCESS_MSG(x) std::cout << "\033[32m" << x << "\033[0m" << std::endl
#define FAIL_MSG(x) std::cout << "\033[31m" << x << "\033[0m" << std::endl


static void npc_break() {
    if (times >= MAX_TIMES) {
        FAIL_MSG("Simulation exceeded maximum cycles, stopping execution.");
        run = 0;
    }
}

static void cpu_exec_once(Vysyx_25060173_core *core, std::vector<uint32_t> &insts, VerilatedVcdC *tfp) {
    uint32_t pc_count = (core->now_pc - 0x80000000) / 4;  // Use current PC, not next PC

    if (pc_count >= insts.size()) {
        run = 0;
        return;
    }

    core->clk = 0;
    core->inst = insts[pc_count];  // Get instruction based on current PC
    core->eval();
    tfp->dump(sim_time++);

    core->clk = 1;
    core->eval();
    tfp->dump(sim_time++);
}

static void reset(Vysyx_25060173_core *core, int n, VerilatedVcdC *tfp) {
    for (int i = 0; i < n; i++) {
        core->clk = 0;
        core->reset = 0;
        core->eval();
        tfp->dump(sim_time++);
        core->clk = 1;
        core->eval();
        tfp->dump(sim_time++);

        core->clk = 0;
        core->reset = 1;
        core->eval();
        tfp->dump(sim_time++);
        core->clk = 1;
        core->eval();
        tfp->dump(sim_time++);
    }
}

extern "C" void ebreak_handler() {
    if (run) {
        std::cout << "EBREAK encountered, stopping execution." << std::endl;
        run = 0;
    }
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    Vysyx_25060173_core *core = new Vysyx_25060173_core;

    Verilated::traceEverOn(true);
    VerilatedVcdC *tfp = new VerilatedVcdC;
    // core->trace(tfp, 99);
    // tfp->open("trace.vcd");

    std::cout << argv[0] << " is running..." << std::endl;
    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    uint32_t count = static_cast<uint32_t>(size / sizeof(uint32_t));
    std::vector<uint32_t> insts(count);

    if (!file.read(reinterpret_cast<char *>(insts.data()), count * sizeof(uint32_t))) {
        throw std::runtime_error("Failed to read file");
    }

    file.close();

    reset(core, 10, tfp);

    while (run) {
        single_cycle(core, insts, tfp);
        times++;
        npc_break();
        // std::cout << "Next PC: 0x" << std::hex << core->next_pc << std::endl;
        std::cout << "Now  PC: 0x" << std::hex << core->now_pc << "  Next PC: 0x" << std::hex << core->next_pc << std::endl;
    }

    if (times < MAX_TIMES) SUCCESS_MSG("Simulation finished successfully.");

    tfp->close();
    delete tfp;
    delete core;
    return 0;
}
