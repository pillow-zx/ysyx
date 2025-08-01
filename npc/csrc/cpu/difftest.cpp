#include <difftest.h>
#include <macro.h>
#include <cpu.h>
#include "Vysyx_25060173_core___024root.h"
#include "Vysyx_25060173_core_ysyx_25060173_core.h"
#include "Vysyx_25060173_core_ysyx_25060173_RegisterFile.h"
#include "cpu.h"

// Declare external core model instance
extern Vysyx_25060173_core *core;

extern "C" {
void difftest_init(void *regs, void *pmems);
void difftest_memcpy(unsigned int addr, void *buf, size_t n, bool direction);
void difftest_regcpy(void *dut, bool direction);
void difftest_exec(unsigned long n);
}

static unsigned int difftest_regs[NPC_BITS + 1] = {0}; // +1 for PC

void init_difftest(unsigned int *core_regs, void *pmems) {
    // Initialize difftest with the core registers and memory
    for (int i = 0; i < NPC_BITS; i++) {
        // Access the core registers through the Verilator interface
        core_regs[i] = core->__PVT__ysyx_25060173_core->__PVT__u_ysyx_25060173_RegisterFile->__PVT__regfile[i];
    }
    core_regs[NPC_BITS] = core->now_pc; // Store the current PC in the last register
    difftest_init(core_regs, pmems);
    PRINT_GREEN_0("Difftest initialized with core registers at " << core_regs << " and memory at " << pmems);
}

extern unsigned int core_regs[NPC_BITS + 1]; // Declare the core registers array

static void update_core_regs() {
    for (int i = 0; i < NPC_BITS; i++) {
        // Update the core registers from the Verilator interface
        core_regs[i] = core->__PVT__ysyx_25060173_core->__PVT__u_ysyx_25060173_RegisterFile->__PVT__regfile[i];
    }
    core_regs[NPC_BITS] = core->now_pc; // Update the current PC
}

static void checkregs() {
    update_core_regs(); // Update the core registers before checking
    for (int i = 0; i < NPC_BITS; i++) {
        if (difftest_regs[i] != core_regs[i]) {
            PRINT_RED_0("Mismatch at register "
                        << regs[i] << ": difftest = 0x" << std::hex << difftest_regs[i] << ", actual = 0x"
                        << core->__PVT__ysyx_25060173_core->__PVT__u_ysyx_25060173_RegisterFile->__PVT__regfile[i]);
            PRINT_RED_0("THE PC = 0x" << std::hex << core->now_pc);
            exit(1);
        }
    }
}

void difftest_step_and_check() {
    // for (int i = 0; i < NPC_BITS; i++) {
    //     PRINT_BLUE_0("core_regs[" << i << "] = 0x" << std::hex << core_regs[i] << std::dec);
    // }
    difftest_exec(1);                      // Execute one instruction in the difftest
    difftest_regcpy(difftest_regs, false); // Copy the difftest registers to the local array
    checkregs();                           // Check if the difftest registers match the core registers
    // Update the core registers from the difftest registers
}
