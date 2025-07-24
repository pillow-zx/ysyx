#include "cpu.h"

bool npc_STATE = true;
Vysyx_25060173_core *core = new Vysyx_25060173_core;

static uint32_t temp_pc = 0x80000000;

static const std::vector<std::string> regs = {"$0", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
                                              "a1", "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
                                              "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

void show_regs() {
    PRINT_MAGENTA_0("=================================");
    PRINT_MAGENTA_0("=======Register information=====");
    PRINT_MAGENTA_0("=================================");
    uint32_t count = 0;
    for (int i = 0; i < NPC_BITS; i++) {
        // 通过Verilator提供的公共接口访问寄存器文件
        uint32_t reg_value;
        reg_value = core->rootp->ysyx_25060173_core__DOT__u_ysyx_25060173_RegisterFile__DOT__regfile[i];
        std::string prompt = boost::str(boost::format("%s: 0x%08x") % regs.at(i) % reg_value);
        PRINT_BLUE_0(prompt);
        if (count == 8) {
            std::cout << std::endl;
            count = 0;
        } else {
            std::cout << "\t";
            count++;
        }
    }
    std::cout << std::endl;
    PRINT_MAGENTA_0("=================================");
    PRINT_MAGENTA_0("      Total registers: " << NPC_BITS);
    PRINT_MAGENTA_0("=================================");
}

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
    core->inst = insts[pc_index]; // Get instruction based on current PC
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
