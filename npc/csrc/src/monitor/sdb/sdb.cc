#include <monitor.h>
#include <sstream>
#include <bitset>

static std::string get_string(std::string prompt = "(npc) > ") {
    std::string str;
    std::cout << prompt;
    std::getline(std::cin, str);
    return str;
}

static std::vector<std::string> split_string(const std::string &str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

static int string_to_int(std::string str) {
    return std::stoi(str);
}

void show_registers() {
    size_t times = 0;
    for (size_t i = 0; i < CPU_REGISTERS; i++) {
        std::cout << regs[i] << " = " << std::bitset<32>(cpu.get_register(i)) << std::endl;
        if (times++ == 8) {
            std::cout << std::endl;
            times = 0;
        }
    }
}

void show_memory(size_t addr, size_t len) {
    if (len < 1) {
        std::cerr << "Error: Length must be greater than 0." << std::endl;
        return;
    }
    if (addr < MEMORY_START || addr + len > MEMORY_START + MEMORY_SIZE) {
        std::cerr << "Error: Address out of bounds." << std::endl;
        return;
    }
    for (size_t i = 0; i < len; i++) {
        std::cout << "Memory[" << std::hex << (addr + i) << "] = " << std::bitset<32>(memory.read_data(addr + i, 4))
                  << std::endl;
    }
}

std::vector<std::string> get_commands(std::string prompt) {
    std::string command = get_string(prompt);
    if (command.empty()) {
        return {};
    }

    // Split the command into tokens
    std::vector<std::string> tokens = split_string(command, ' ');
    return tokens;
}
