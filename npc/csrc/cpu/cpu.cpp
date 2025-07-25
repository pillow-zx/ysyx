#include <cpu.h>
#include <tools.h>

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

void reset() {
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

static void cpu_exec_once(uint32_t inst) {

    itrace(inst, logfile);
    // mtrace(pc_index, read_pmem(core->now_pc));

    core->clk = 0;
    core->inst = inst; // Get instruction based on current PC
    core->eval();

    core->clk = 1;
    core->eval();
    
    // Call ftrace after clock edge to get correct next_pc
    ftrace(inst);
}

static void say_pc() {
    PRINT_BLUE_0("Current PC: " << std::hex << core->now_pc << std::dec);
}

void cpu_exec(uint32_t n) {
    for (; n > 0 && npc_STATE; n--) {
        // Execute one instruction
        cpu_exec_once(read_pmem(core->now_pc - DEFAULT_PC_START));
        // Print current PC
        // say_pc();
    }
    if (!npc_STATE) {
        std::cout << "Final PC: " << std::hex << core->now_pc << std::dec << std::endl;
        return;
    }
}
