#include "sdb.h"

void npc_start(std::vector<uint32_t> &insts) {
    welcome();
    while (npc_STATE) {
        std::string command = get_string("(npc)>");
        std::vector<std::string> tokens = Stringsplit(command, " ");
        if (tokens.empty())
            continue;
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

void npc_init(std::vector<uint32_t> &insts, std::string filename) {
    reset(); // Reset the CPU state
    npc_STATE = true; // Set the simulation state to running
    // Load instructions from the specified file
    insts = get_insts(filename);
}