#include "tools.h"
#include "cpu.h"


void handle_commands(std::vector<uint32_t> &insts) {
    welcome();
    while (npc_STATE) {
        std::string command = get_string("(npc)>");
        std::vector<std::string> tokens = Stringsplit(command, " ");
        if (tokens.empty()) continue;
        if (tokens[0] == "help") {
            command_lists();
        } else if (tokens[0] == "q") {
            npc_STATE = false;
        } else if (tokens[0] == "c") {
            cpu_exec(-1, insts);
        } else if (tokens[0] == "si") {
            int n = (tokens.size() > 1) ? std::stoi(tokens[1]) : 1;
            cpu_exec(n, insts);
        } else if (tokens[0] == "info") {
            if (tokens.size() > 1 && tokens[1] == "r") {
                show_regs();
            } else if (tokens.size() > 1 && tokens[1] == "m") {
                show_memory(insts);
            } else {
                std::cout << "Unknown info command: " << command << std::endl;
            }
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
    PRINT_GREEN_0("NPC simulation completed successfully!");
    return 0;
}