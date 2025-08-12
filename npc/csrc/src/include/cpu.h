#pragma once

#include <difftest.h>

const std::vector<std::string> regs = {"$0", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
                                       "a1", "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
                                       "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

enum npc_Mode { RUNNING, STOP, EXIT };

class npc_State {
  private:
    npc_Mode npc_state;

  public:
    npc_State() : npc_state(RUNNING) {}

    void set_state(npc_Mode state) {
        npc_state = state;
    }

    npc_Mode get_state() const {
        return npc_state;
    }
};

extern npc_State npc_state; // Global NPC state

class Cpu {
  public:
    static void init() {
        // Initialize the CPU
        npc_state.set_state(RUNNING); // Set initial NPC state

        core->clk = 0;
        core->rst_n = 1;
        core->eval(); // Evaluate the model after reset

        core->clk = 1; // Set clock to high
        core->rst_n = 0;
        core->eval(); // Evaluate the model again after reset

        core->rst_n = 1; // Set reset to high
    }

    static void cpu_exec_once() {
        core->clk = 0; // Set clock to low
        core->eval();  // Evaluate the model

        MyTrace.itrace_handle(core->rootp->cpu__DOT__pc, core->rootp->cpu__DOT__inst);
        // MyTrace.ftrace_handle(core->rootp->cpu__DOT__inst);

        // difftest_step(); // Call difftest step to check registers

        core->clk = 1; // Set clock to high
        core->eval();  // Evaluate the model again

        // checkregs(); // Check registers after execution
    }

    static void cpu_exec(size_t n) {
        for (size_t i = 0; i < n; ++i) {
            if (npc_state.get_state() != RUNNING) {
                return;
            }
            cpu_exec_once();
        }
    }

    static uint32_t get_register(size_t index) {
        if (index >= regs.size()) {
            throw std::out_of_range("Register index out of range");
        }
        return core->rootp->cpu__DOT__regf__DOT__regfile[index];
    }
};
