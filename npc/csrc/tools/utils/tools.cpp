#include <tools.h>
#include <macro.h>

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

void show_memory(const std::vector<uint32_t> &insts) {
    PRINT_MAGENTA_0("=================================");
    PRINT_MAGENTA_0("=======Memory information=======");
    PRINT_MAGENTA_0("=================================");
    for (const auto &inst : insts) {
        PRINT_GREEN_0(std::bitset<32>(inst).to_string());
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