#include <cpu.h>
#include <Log.h>
#include <macro.h>
#include <tools.h>
#include <memory.h>
#include <difftest.h>
#include <iostream>
#include <unistd.h>
#include "Vysyx_25060173_core___024root.h"
#include "Vysyx_25060173_core_ysyx_25060173_core.h"
#include "Vysyx_25060173_core_ysyx_25060173_RegisterFile.h"


bool BATCH_MODE = false; // 是否为批处理模式
bool npc_STATE = true;
Vysyx_25060173_core *core = new Vysyx_25060173_core;
static uint32_t temp_pc = 0x80000000;
static std::string logfile = "/home/waysorry/ysyx/ysyx-workbench/npc/log.txt";

void show_regs() {
    PRINT_MAGENTA_0("=================================");
    PRINT_MAGENTA_0("=======Register information=====");
    PRINT_MAGENTA_0("=================================");
    uint32_t count = 0;
    for (int i = 0; i < NPC_BITS; i++) {
        // 通过Verilator提供的公共接口访问寄存器文件
        uint32_t reg_value;
        reg_value = core->__PVT__ysyx_25060173_core->__PVT__u_ysyx_25060173_RegisterFile->__PVT__regfile[i];
        std::string prompt = boost::str(boost::format("%s: 0x%08x") % regs.at(i) % reg_value);
        std::cout << "\x1b[1;34m" << prompt << "\x1b[0m" << " ";
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

void reset() {
    core->clk = 1;
    core->reset = 1; // 拉高复位信号
    core->eval();

    core->reset = 0; // 拉低复位信号
    core->clk = 0; // 拉低时钟信号
    core->eval();
}

void say_pc() {
    PRINT_BLUE_0("Current PC: " << std::hex << core->now_pc << std::dec);
}

static void cpu_exec_once() {
    // mtrace(pc_index, read_pmem(core->now_pc));
    itrace(read_pmem(core->now_pc), logfile);

    ftrace(read_pmem(core->now_pc));

    core->clk = 1; // 拉高时钟信号
    core->eval(); // 评估当前状态
    // say_pc(); // 打印当前PC

    
    core->clk = 0; // 拉低时钟信号
    core->eval(); // 评估当前状态
    // say_pc(); // 打印当前PC

    // difftest_step_and_check(); // 执行difftest并检查寄存器
}

void cpu_exec(uint32_t n) {
    for (; n > 0 && npc_STATE; n--) {
        cpu_exec_once();
    }
}
