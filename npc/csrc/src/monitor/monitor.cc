#include <monitor.h>
#include <difftest.h>
#include <device.h>
#include <boost/program_options.hpp>

/* npc 运行相关 */
bool BATCH_MODE = false;
npc_State npc_state;
Memory memory;
Cpu cpu;

/* Trace init */
Traces MyTrace;
std::string image;
std::string elf_file;
std::string log_file;

Elf32_Ehdr *ftrace_file_header = nullptr;    // ELF header for ftrace
Elf32_Shdr *ftrace_section_header = nullptr; // Section header for ftrace
Elf32_Sym *ftrace_file_symtab = nullptr;     // Symbol table for ftrace
char *ftrace_file_strtab = nullptr;          // String table for ftrace
int ftrace_file_symtab_num = 0;              // Size of the symbol table for ftrace

/* verilog */
Vcpu *core = new Vcpu();

static void parse_init(int argc, char *argv[]) {
    boost::program_options::options_description desc("Allowed options");

    desc.add_options()("batch,b", boost::program_options::bool_switch(&BATCH_MODE), "run in batch mode")(
        "image,i", boost::program_options::value<std::string>(&image)->required(), "set the program image file")(
        "elf,e", boost::program_options::value<std::string>(&elf_file)->required(), "set the ELF file for debugging")(
        "log,l", boost::program_options::value<std::string>(&log_file)->required(), "set the log file");

    boost::program_options::variables_map vm;
    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);
    } catch (const boost::program_options::error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << desc << std::endl;
        exit(1);
    }
}

void npc_init(int argc, char *argv[]) {
    parse_init(argc, argv); // Parse command line arguments

    cpu.init(); // Initialize the CPU

    memory.init(image); // Initialize memory with the program image

    MyTrace.itrace_init(log_file); // Initialize trace with the log file
    MyTrace.ftrace_init(elf_file); // Initialize ftrace with the ELF file

    init_device(); // Initialize device components

    init_difftest();
}

void npc_start() {
    if (BATCH_MODE) {
        std::cout << "Running in batch mode..." << std::endl;
        cpu.cpu_exec(-1);
        return; // Exit after execution in batch mode
    }

    do {
        if (BATCH_MODE == EXIT) {
            std::cout << "Program has finished, press any key to exit..." << std::endl;
            std::cin.get(); // Wait for user input to exit
            return;
        }
        std::vector<std::string> commands = get_commands("(npc) > ");
        if (commands.empty()) {
            continue; // Skip if no commands are entered
        }
        if (commands[0] == "q") {
            break; // Exit the loop if 'q' is entered
        } else if (commands[0] == "c") {
            cpu.cpu_exec(-1);
        } else if (commands[0] == "si") {
            if (commands.size() > 1) {
                int n = std::stoi(commands[1]);
                cpu.cpu_exec(n);
            } else {
                cpu.cpu_exec(1); // Default to single step if no argument is provided
            }
        } else if (commands[0] == "info") {
            if (commands.size() > 1 && commands[1] == "r") {
                show_registers(); // Show registers if 'info r' is entered
            } else {
                std::cout << "Unknown info command." << std::endl;
            }
        } else if (commands[0] == "x") {
            if (commands.size() > 2) {
                size_t len = std::stoi(commands[1]);
                size_t addr = std::stoul(commands[2], nullptr, 16);
                show_memory(addr, len);
            }
        } else {
            std::cout << "Unknown command: " << commands[0] << std::endl;
        }
    } while (true);
}
