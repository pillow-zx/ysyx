#include "tools.h"

extern bool npc_STATE;

std::string get_string(std::string prompt) {
    std::cout << prompt;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

uint32_t get_integer(std::string prompt) {
    std::string input = get_string(prompt);
    uint32_t value;
    std::istringstream iss(input);
    while (!(iss >> value)) {
        std::cout << "Invalid input. Please enter a valid integer: ";
        std::getline(std::cin, input);
        iss.clear();
        iss.str(input);
    }
    return value;
}

std::vector<uint32_t> get_insts(std::string prompt) {
    std::ifstream file(prompt, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Could not open file: " + prompt);
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    uint32_t count = static_cast<uint32_t>(size / sizeof(uint32_t));
    std::vector<uint32_t> insts(count);

    if (!file.read(reinterpret_cast<char*>(insts.data()), count * sizeof(uint32_t))) {
        throw std::runtime_error("Error reading file: " + prompt);
    }

    file.close();
    npc_STATE = true; // Set the state to true after reading instructions
    return insts;
}

void command_lists() {
    std::cout << "Available commands:" << std::endl;
    std::cout << "help - Show this help message" << std::endl;
    std::cout << "q - Exit the simulator" << std::endl;
    std::cout << "c - Continue execution after an EBREAK" << std::endl;
    std::cout << "si [n] - Step through 'n' instructions (default is 1)" << std::endl;
}



void welcome() {
    std::cout << "Welcome to the RISC-V CPU Simulator!" << std::endl;
    std::cout << "This simulator executes RISC-V instructions and handles EBREAK." << std::endl;
    std::cout << "You can enter commands to interact with the simulator." << std::endl;
    command_lists();
}