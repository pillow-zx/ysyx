#include "tools.h"
#include "cpu.h"
#include "monitor/sdb.h"


int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    std::vector<uint32_t> insts;
    // std::vector<uint32_t> insts = get_insts(argv[1]);
    // handle_commands(insts);
    // PRINT_GREEN_0("NPC simulation completed successfully!");

    npc_init(insts,argv[1]);

    npc_start(insts);
    return 0;
}