#include "tools.h"
#include "cpu.h"
#include "monitor/sdb.h"


int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    std::vector<uint32_t> insts;

    npc_init(insts, argc, argv);

    npc_start(insts);
    return 0;
}