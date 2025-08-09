#include <iostream>
#include <getopt.h>
#include <vector>
#include <string>
#include <fstream>
#include <tools.h>
#include <difftest.h>
#include <device.h>
#include <cpu.h>
#include <macro.h>
#include <Log.h>
#include <memory.h>

static std::string diff_so_file;
static std::string itrace_file;
static std::string img_file;

unsigned int core_regs[NPC_BITS + 1] = {0};

void npc_start() {
    while (npc_STATE) {
        if (BATCH_MODE) {
            // 批处理模式下直接执行
            cpu_exec(-1);
            continue;
        }
        std::string command = get_string("(npc)>");
        std::vector<std::string> tokens = Stringsplit(command, " ");
        if (tokens.empty())
            continue;
        if (tokens[0] == "help") {
            command_lists();
        } else if (tokens[0] == "q") {
            npc_STATE = false;
        } else if (tokens[0] == "c") {
            cpu_exec(-1);
        } else if (tokens[0] == "si") {
            int n = (tokens.size() > 1) ? std::stoi(tokens[1]) : 1;
            cpu_exec(n);
        } else if (tokens[0] == "info") {
            if (tokens.size() > 1 && tokens[1] == "r") {
                show_regs();
            } else {
                std::cout << "Unknown info command: " << command << std::endl;
            }
        } else if (tokens[0] == "x") {
            if (tokens.size() < 2) {
                std::cout << "Usage: x <n> - to show n words of memory" << std::endl;
                continue;
            }
            uint32_t n = std::stoi(tokens[1]);
            uint32_t start_addr = static_cast<uint32_t>(std::stoul(tokens[2], nullptr, 16));
            ASSERT_WITH_MSG_0(std::stoul(tokens[1], nullptr, 0) < (DEFAULT_MEM_START + CONFIG_MSIZE) / 4,
                              "Invalid memory address");
            // ASSERT_WITH_MSG_0(std::stoul(tokens[2], nullptr, 16) > (DEFAULT_MEM_START + CONFIG_MSIZE) ||
            //                       std::stoul(tokens[2], nullptr, 16) < DEFAULT_MEM_START,
            //                   "Invalid memory address");
            show_memory(n, start_addr);
        } else {
            std::cout << "Unknown command: " << command << std::endl;
            continue;
        }
    }
}

static void init_cpu() {
    npc_STATE = true;
    reset();
}

static long load_img() {
    if (img_file.empty()) {
        std::cerr << "No image file specified. Use -i <filename> to specify an image file." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::ifstream img_stream(img_file, std::ios::binary);
    ASSERT_WITH_MSG_0(img_stream.is_open(), "Failed to open image file: " + img_file);

    img_stream.seekg(0, std::ios::end);
    std::streamsize size = img_stream.tellg();
    img_stream.seekg(0, std::ios::beg);

    long count = size / sizeof(uint32_t);
    std::cout << ANSI_COLOR_GREEN << "The image is " << img_file << " size = " << count << std::endl;

    img_stream.read(reinterpret_cast<char *>(write_pmem().data()), size);

    ASSERT_WITH_MSG_0(img_stream, "Failed to read image file: " + img_file);
    img_stream.close();

    return count;
}

static void init_itrace() {
    if (itrace_file.empty()) {
        std::cerr << "No log file specified. Use -l <filename> to specify a log file." << std::endl;
        exit(EXIT_FAILURE);
    }
    std::ofstream ofs;
    ofs.open(itrace_file, std::ios::trunc); // 清空文件内容
    ofs.close();
    std::cout << ANSI_COLOR_GREEN << "Log file initialized: " << itrace_file << ANSI_COLOR_RESET << std::endl;
}

#include <getopt.h>

extern char *ftrace_file; // 用于存储ELF格式的镜像文件路径

static int parse_args(int argc, char **argv) {
    const struct option table[] = {
        {"help", no_argument, nullptr, 'h'},         {"log", required_argument, nullptr, 'l'},
        {"ftrace", required_argument, nullptr, 'f'}, {"img", required_argument, nullptr, 'i'},
        {"batch", no_argument, nullptr, 'b'},
    };
    int opt;
    while ((opt = getopt_long(argc, argv, "hl:f:i:", table, nullptr)) != -1) {
        switch (opt) {
            case 'h':
                std::cout << "Usage: npc [options] <filename>\n"
                          << "Options:\n"
                          << "  -h, --help          Show this help message\n"
                          << "  -l, --log <file>    Log output to specified file\n"
                          << "  -f, --ftrace <file> Enable function tracing with specified ELF file\n";
                return 0;
            case 'b': BATCH_MODE = true; break;
            case 'l':
                // Handle log file option
                itrace_file = optarg;
                init_itrace();
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
            default: std::cerr << "Unknown option: " << static_cast<char>(opt) << std::endl; return -1;
        }
    }
    return 0;
}

void npc_init(int argc, char **argv) {
    parse_args(argc, argv);

    init_memory();

    long img_size = load_img();

    init_device();

    init_cpu();

    // Initialize difftest
    init_difftest(core_regs, write_pmem().data());

    welcome();
}
