#include "tools.h"
#include "cpu.h"


void handle_commands(std::vector<uint32_t> &insts) {
    welcome();
    while (npc_STATE) {
        std::string command = get_string("(npc)>");
        if (command == "help") {
            command_lists();
        } else if (command == "q") {
            npc_STATE = false;
        } else if (command == "c") {
            cpu_exec(-1, insts);
        } else if (command.substr(0, 3) == "si ") {
            int n = std::stoi(command.substr(3));
            cpu_exec(n, insts);
            std::cout << "NOW PC: " << std::hex << core->now_pc << std::dec << std::endl;
        } else {
            std::cout << "Unknown command: " << command << std::endl;
            continue;
        }
    }
}

int main(int argc, char **argv) {
    Verilated::commandArgs(argc, argv);
    std::vector<uint32_t> insts = get_insts(argv[1]);
    handle_commands(insts);
    return 0;
}