#include <tools.h>
#include <macro.h>
#include <bitset>
#include <iostream>
#include <sstream>
#include <memory.h>


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

std::vector<std::string> Stringsplit(const std::string &str, const std::string &delim = " ") {
    size_t start = 0;
    size_t end = str.find(delim);
    std::vector<std::string> tokens;
    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delim.length();
        end = str.find(delim, start);
    }
    tokens.push_back(str.substr(start, end));
    return tokens;
}

void show_memory(uint32_t n, uint32_t start_addr) {
    PRINT_MAGENTA_0("=================================");
    PRINT_MAGENTA_0("=======Memory information=======");
    PRINT_MAGENTA_0("=================================");
    for (uint32_t i = 0; i < n; i++) {
        uint32_t addr = i * 4 + start_addr; // Assuming 4-byte words and starting address
        uint32_t value = read_pmem(addr);
        std::string formatted = boost::str(boost::format("0x%08x: %s (0x%08x)") 
                                         % addr 
                                         % std::bitset<32>(value).to_string() 
                                         % value);
        PRINT_BLUE_0(formatted);
    }
    PRINT_MAGENTA_0("=================================");
    PRINT_MAGENTA_0("=======End of Memory Info=======");
    PRINT_MAGENTA_0("=================================");
}

void command_lists() {
    PRINT_GREEN_0("=================================");
    PRINT_GREEN_0("=======Available commands=======");
    PRINT_GREEN_0("=================================");
    PRINT_GREEN_0("help - Show this help message");
    PRINT_GREEN_0("q - Exit the simulator");
    PRINT_GREEN_0("c - Continue execution after an EBREAK");
    PRINT_GREEN_0("si [n] - Step through 'n' instructions (default is 1)");
    PRINT_GREEN_0("info r - Show register information");
    PRINT_GREEN_0("info m - Show memory information");
    PRINT_GREEN_0("=================================");
    PRINT_GREEN_0("=================================");
}

void welcome() {
    PRINT_GREEN_0("=================================");
    PRINT_GREEN_0("=======Welcome to the NPC=======");
    PRINT_GREEN_0("=================================");
    command_lists();
}