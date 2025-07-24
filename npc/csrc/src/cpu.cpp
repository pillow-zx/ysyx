#include "cpu.h"

bool npc_STATE = true;
Vysyx_25060173_core *core = new Vysyx_25060173_core;

static uint32_t temp_pc = 0x80000000;

static void reset() {
    for (int i = 0; i < 10; i++) {
        core->clk = 0;
        core->reset = 0;
        core->eval();
        // tfp->dump(sim_time++);
        core->clk = 1;
        core->eval();
        // tfp->dump(sim_time++);

        core->clk = 0;
        core->reset = 1;
        core->eval();
        // tfp->dump(sim_time++);
        core->clk = 1;
        core->eval();
        // tfp->dump(sim_time++);
    }
}

static void cpu_exec_once(std::vector<uint32_t> &insts) {
    // Calculate instruction index based on current PC
    uint32_t pc_index = (core->now_pc - temp_pc) / 4;
    
    if (pc_index >= insts.size()) {
        npc_STATE = false;
        return;
    }
    
    core->clk = 0;
    core->inst = insts[pc_index];  // Get instruction based on current PC
    core->eval();

    core->clk = 1;
    core->eval();
}

static void say_pc() {
    std::cout << "PC: " << std::hex << core->now_pc << std::dec << std::endl;
}

void cpu_exec(int n, std::vector<uint32_t> &insts) {
    reset();
    if (n < 0) {
        // Continue execution until stopped
        while (npc_STATE) {
            cpu_exec_once(insts);
            say_pc();
        }
    } else {
        // Execute n instructions
        for (int i = 0; i < n && npc_STATE; i++) {
            cpu_exec_once(insts);
            say_pc();
        }
    }

    if (!npc_STATE) {
        std::cout << "Final PC: " << std::hex << core->now_pc << std::dec << std::endl;
        return;
    }
}
