#include <difftest.h>
#include <cstdint>
#include <macro.h>
#include "Vysyx_25060173_core___024root.h"
#include "Vysyx_25060173_core_ysyx_25060173_core.h"
#include "Vysyx_25060173_core_ysyx_25060173_RegisterFile.h"
#include "cpu.h"
#include "riscv.h"

// Declare external core model instance
extern Vysyx_25060173_core *core;

// 直接声明库函数（extern "C" 确保C链接）
extern "C" {
void difftest_init(void *regs, void *pmems);
void difftest_memcpy(uint32_t addr, void *buf, unsigned int n, bool direction);
void difftest_regcpy(void *dut, bool direction);
void difftest_exec(uint64_t n, unsigned int pc, unsigned int dnpc, unsigned int inst);
}

static void checkregs(unsigned int *difftest_regs, unsigned int *regs, int npc_bits) {
    for (int i = 0; i < npc_bits; i++) {
        if (difftest_regs[i] != regs[i]) {
            PRINT_RED_0("Mismatch at register " << i << ": difftest = 0x" << std::hex << difftest_regs[i]
                                                << ", actual = 0x" << regs[i]);
            exit(1);
        }
    }
    PRINT_GREEN_0("All registers match!");
}

void init_difftest(unsigned int *core_regs, void *pmems) {
    // 直接调用库函数初始化 difftest
    for (int i = 0; i < NPC_BITS; i++) {
        // 通过Verilator提供的公共接口访问寄存器文件
        core_regs[i] = core->__PVT__ysyx_25060173_core->__PVT__u_ysyx_25060173_RegisterFile->__PVT__regfile[i];
    }
    difftest_init(core_regs, pmems);
    PRINT_GREEN_0("Difftest initialized with port " << core_regs << " and memory " << pmems);
}

void difftest_step_and_check(unsigned int *core_regs,unsigned int pc, unsigned int dnpc, unsigned int inst) {
    unsigned int difftest_regs[NPC_BITS] = {0};
    // 通过Verilator提供的公共接口获取当前寄存器状态
    for (int i = 0; i < NPC_BITS; i++) {
        core_regs[i] = core->__PVT__ysyx_25060173_core->__PVT__u_ysyx_25060173_RegisterFile->__PVT__regfile[i];
    }
    for (int i = 0; i < NPC_BITS; i++) {
        std::cout << "core_regs[" << i << "] = 0x" << std::hex << core_regs[i] << std::dec << std::endl;
    }
    difftest_exec(1, pc, dnpc, inst);
    difftest_regcpy(difftest_regs, false);
    for (int i = 0; i < NPC_BITS; i++) {
        std::cout << "difftest_regs[" << i << "] = 0x" << std::hex << difftest_regs[i] << std::dec << std::endl;
    }
    checkregs(difftest_regs, core_regs, NPC_BITS);
}
