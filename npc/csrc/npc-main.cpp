#include "monitor/sdb.h"
#include <verilated.h>

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);

    npc_init(argc, argv);

    npc_start();
    return 0;
}
