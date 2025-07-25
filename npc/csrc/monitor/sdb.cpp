#include "sdb.h"

static std::string itrace_file;
static std::string img_file;

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

static void init_insts(std::vector<uint32_t> &insts) {
    if (img_file.empty()) {
        std::cerr << "Error: No image file specified." << std::endl;
        exit(EXIT_FAILURE);
    }
    insts = get_insts(img_file);
}

#include <getopt.h>

extern char *ftrace_file; // 用于存储ELF格式的镜像文件路径

static int parse_args(int argc, char **argv) {
    const struct option table[] = {
        {"help", no_argument, nullptr, 'h'},
        {"log", required_argument, nullptr, 'l'},
        {"ftrace", required_argument, nullptr, 'f'},
        {"img", required_argument, nullptr, 'i'},
    };
    int opt;
    std::cout << argv[0] << " - NPC (Nyuzi Processor Core) Simulator\n";
    std::cout << argv[1] << " - Image file for the Nyuzi processor\n";
    while ((opt = getopt_long(argc, argv, "hl:f:i:", table, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                std::cout << "Usage: npc [options] <filename>\n"
                          << "Options:\n"
                          << "  -h, --help          Show this help message\n"
                          << "  -l, --log <file>    Log output to specified file\n"
                          << "  -f, --ftrace <file> Enable function tracing with specified ELF file\n";
                return 0;
            case 'l':
                // Handle log file option
                itrace_file = optarg;
                break;
            case 'f':
                // Handle ftrace file option
                ftrace_file = optarg;
                ftrace_elf_init(ftrace_file);
                break;
            case 'i':
                // Handle image file option
                img_file = optarg;
                break;
            default:
                std::cerr << "Unknown option: " << static_cast<char>(opt) << std::endl;
                return -1;
        }
    }
    return 0;
}



void npc_init(std::vector<uint32_t> &insts, int argc, char **argv) {
    parse_args(argc, argv);

    init_insts(insts);

    npc_STATE = true;
    reset();
}