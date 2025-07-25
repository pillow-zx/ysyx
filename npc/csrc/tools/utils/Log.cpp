#include <Log.h>
#include <stdio.h>
#include <capstone/capstone.h>

void itrace(uint32_t inst, std::string log_file) {
    std::ofstream log_stream(log_file, std::ios::app);
    if (!log_stream) {
        std::cerr << "Failed to open log file: " << log_file << std::endl;
        return;
    }

    csh handle;
    cs_insn *insn;
    size_t count;

    // 初始化capstone
    if (cs_open(CS_ARCH_RISCV, CS_MODE_RISCV32, &handle) != CS_ERR_OK) {
        log_stream << "Failed to initialize capstone" << std::endl;
        return;
    }

    count = cs_disasm(handle, reinterpret_cast<const uint8_t *>(&inst), sizeof(uint32_t), core->now_pc, 1, &insn);

    if (count > 0) {
        for (size_t i = 0; i < count; i++) {
            log_stream << "0x" << std::hex << insn[i].address << ":\t" << insn[i].mnemonic << "\t" << insn[i].op_str
                       << std::endl;
        }
        cs_free(insn, count);
    } else {
        log_stream << "Failed to disassemble instruction at 0x" << std::hex << core->now_pc << std::endl;
    }
    cs_close(&handle);
}

// void mtrace(const uint32_t pc_index, std::vector<uint32_t> &insts) {
//     std::cout << ANSI_COLOR_BLUE << "PC: " << std::hex << (pc_index * 4 + 0x80000000)
//               << ", Instruction: " << std::bitset<32>(insts[pc_index]) << ANSI_COLOR_RESET << std::endl;
// }

#include <elf.h>
#include <stdlib.h>

char *ftrace_file = NULL;                // 用于存储ELF格式的镜像文件路径
Elf32_Ehdr *ftrace_file_header;          // ELF文件头结构体
Elf32_Shdr *ftrace_file_sections = NULL; // ELF节头结构体数组
char *ftrace_file_strtab = NULL;         // ELF符号名字符串表
Elf32_Sym *ftrace_file_symtab = NULL;    //  ELF符号表
int ftrace_file_symtab_num;

// 解析elf文件
void ftrace_elf_init(char *ftrace_file) {
    if (ftrace_file == NULL) {
        PRINT_BLUE_0("Ftrace ELF file is not set, skipping initialization.");
        return;
    }

    FILE *fp = fopen(ftrace_file, "rb");
    if (fp == NULL) {
        std::cerr << "Failed to open ftrace ELF file: " << ftrace_file << std::endl;
        return;
    }

    // 读取ELF文件头
    ftrace_file_header = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    if (ftrace_file_header == NULL) {
        std::cerr << "Failed to allocate memory for ELF header" << std::endl;
        fclose(fp);
        return;
    }
    size_t ret = fread(ftrace_file_header, sizeof(Elf32_Ehdr), 1, fp);
    if (ret != 1) {
        std::cerr << "Failed to read ELF header from '" << ftrace_file << "'" << std::endl;
        fclose(fp);
        return;
    }

    // 检查ELF文件头的魔数是否正确
    if (ftrace_file_header->e_ident[EI_MAG0] != ELFMAG0 || ftrace_file_header->e_ident[EI_MAG1] != ELFMAG1 ||
        ftrace_file_header->e_ident[EI_MAG2] != ELFMAG2 || ftrace_file_header->e_ident[EI_MAG3] != ELFMAG3) {
        fclose(fp);
        return;
    }

    // 导航到节头表
    if (ftrace_file_header->e_shoff == 0 || ftrace_file_header->e_shentsize == 0) {
        fclose(fp);
        return;
    }
    fseek(fp, ftrace_file_header->e_shoff, SEEK_SET);

    // 读取节头表
    ftrace_file_sections = (Elf32_Shdr *)malloc(ftrace_file_header->e_shnum * sizeof(Elf32_Shdr));
    if (ftrace_file_sections == NULL) {
        std::cerr << "Failed to allocate memory for section headers" << std::endl;
        fclose(fp);
        return;
    }
    ret = fread(ftrace_file_sections, sizeof(Elf32_Shdr), ftrace_file_header->e_shnum, fp);
    if (ret != ftrace_file_header->e_shnum) {
        std::cerr << "Failed to read section headers from '" << ftrace_file << "'" << std::endl;
        fclose(fp);
        return;
    }

    // 读取节头符号表和对应的字符串表
    for (int i = 0; i < ftrace_file_header->e_shnum; i++) {
        if (ftrace_file_sections[i].sh_type == SHT_SYMTAB) {
            // 读取符号表
            fseek(fp, ftrace_file_sections[i].sh_offset, SEEK_SET);
            ftrace_file_symtab = (Elf32_Sym *)malloc(ftrace_file_sections[i].sh_size);
            if (ftrace_file_symtab == NULL) {
                std::cerr << "Failed to allocate memory for symbol table" << std::endl;
                fclose(fp);
                return;
            }
            ret = fread(ftrace_file_symtab, ftrace_file_sections[i].sh_size, 1, fp);
            if (ret != 1) {
                std::cerr << "Failed to read symbol table from '" << ftrace_file << "'" << std::endl;
                fclose(fp);
                return;
            }
            // 计算符号表条目数量
            ftrace_file_symtab_num = ftrace_file_sections[i].sh_size / sizeof(Elf32_Sym);

            // 读取对应的符号名字符串表
            int strtab_index = ftrace_file_sections[i].sh_link;
            if (strtab_index < ftrace_file_header->e_shnum) {
                fseek(fp, ftrace_file_sections[strtab_index].sh_offset, SEEK_SET);
                ftrace_file_strtab = (char *)malloc(ftrace_file_sections[strtab_index].sh_size);
                if (ftrace_file_strtab == NULL) {
                    std::cerr << "Failed to allocate memory for symbol string table" << std::endl;
                    fclose(fp);
                    return;
                }
                ret = fread(ftrace_file_strtab, ftrace_file_sections[strtab_index].sh_size, 1, fp);
                if (ret != 1) {
                    std::cerr << "Failed to read symbol string table from '" << ftrace_file << "'" << std::endl;
                    fclose(fp);
                    return;
                }
            }
            break; // 找到符号表后退出循环
        }
    }

    // 关闭文件
    std::cout << ANSI_COLOR_BLUE << "Ftrace ELF file initialized: " << ftrace_file << ANSI_COLOR_RESET << std::endl;
    fclose(fp);
}

static char *get_symbol_name(uint32_t pc) {
    for (int i = 0; i < ftrace_file_symtab_num; i++) {
        if (ELF32_ST_TYPE(ftrace_file_symtab[i].st_info) == STT_FUNC && pc >= ftrace_file_symtab[i].st_value &&
            pc < ftrace_file_symtab[i].st_value + ftrace_file_symtab[i].st_size) {
            return ftrace_file_symtab[i].st_name + ftrace_file_strtab;
        }
    }
    return NULL;
}

#define CALL_FUNC_TIMES 64

static struct {
    uint32_t pc;
    char name[64];
} call_stack[CALL_FUNC_TIMES];
static int call_stack_depth = 0;

// ftrace_elf_start函数用于跟踪程序中的函数调用和返回
static void ftrace_elf_start(uint32_t pc, uint32_t dnpc, bool is_call, bool is_ret) {
    if (ftrace_file == NULL || ftrace_file_header == NULL || ftrace_file_sections == NULL ||
        ftrace_file_strtab == NULL || ftrace_file_symtab == NULL) {
        return; // 如果跟踪文件或相关数据未初始化，则直接返回
    }

    if (is_call) {
        // 对于函数调用，查找跳转目标地址的符号
        char *target_symbol = get_symbol_name(dnpc);
        if (target_symbol != NULL) {
            // 检查是否为重复调用（避免无限递归的情况）
            bool is_duplicate = false;
            if (call_stack_depth > 0) {
                is_duplicate = (strcmp(call_stack[call_stack_depth - 1].name, target_symbol) == 0);
            }

            if (!is_duplicate) {
                // 打印缩进表示调用层次
                for (int i = 0; i < call_stack_depth; i++) {
                    printf("  ");
                }
                std::cout << ANSI_COLOR_BLUE << "call: " << target_symbol << " at 0x" << std::hex << dnpc
                          << ANSI_COLOR_RESET << std::endl;

                // 将函数信息压入调用栈
                if (call_stack_depth < CALL_FUNC_TIMES) {
                    call_stack[call_stack_depth].pc = dnpc;
                    strncpy(call_stack[call_stack_depth].name, target_symbol,
                            sizeof(call_stack[call_stack_depth].name) - 1);
                    call_stack[call_stack_depth].name[sizeof(call_stack[call_stack_depth].name) - 1] = '\0';
                    call_stack_depth++;
                } else {
                    std::cout << ANSI_COLOR_BLUE << "Warning: Call stack overflow, ignoring call to " << target_symbol
                              << " at 0x" << std::hex << dnpc << ANSI_COLOR_RESET << std::endl;
                }
            }
        }
    } else if (is_ret && call_stack_depth > 0) {
        // 对于函数返回，检查当前pc是否在栈顶函数的范围内
        char *current_symbol = get_symbol_name(pc);
        if (current_symbol != NULL && strcmp(call_stack[call_stack_depth - 1].name, current_symbol) == 0) {
            // 函数返回
            call_stack_depth--;
            for (int i = 0; i < call_stack_depth; i++) {
                printf("  ");
            }
            std::cout << ANSI_COLOR_BLUE << "return: " << call_stack[call_stack_depth].name << " at 0x" << std::hex
                      << pc << ANSI_COLOR_RESET << std::endl;
        }
        // 如果当前符号不匹配，可能是从内联函数或其他情况返回，也尝试处理
        else if (call_stack_depth > 0) {
            call_stack_depth--;
            for (int i = 0; i < call_stack_depth; i++) {
                printf("  ");
            }
            std::cout << ANSI_COLOR_BLUE << "return: " << call_stack[call_stack_depth].name << " at 0x" << std::hex
                      << pc << ANSI_COLOR_RESET << std::endl;
        }
    }
}

#include <string.h>
void ftrace(uint32_t inst) {

    bool is_call = false;
    bool is_ret = false;

    uint32_t pc = core->now_pc - DEFAULT_PC_START;
    uint32_t dnpc = core->next_pc - DEFAULT_PC_START;

    if ((inst & 0x7f) == 0x6f) {
        uint32_t rd = (inst >> 7) & 0x1f;

        char *target_symbol = get_symbol_name(dnpc);
        if (target_symbol != NULL && rd != 0) {
            is_call = true;
        }
    } else if ((inst & 0x7f) == 0x67) {
        uint32_t rd = (inst >> 7) & 0x1f;
        uint32_t rs1 = (inst >> 15) & 0x1f;

        if (rd != 0) {
            char *target_symbol = get_symbol_name(dnpc);
            if (target_symbol != NULL) {
                is_call = true;
            }
        } else if (rd == 0 && rs1 == 1) {
            is_ret = true;
        } else if (rd == 0) {
            char *current_symbol = get_symbol_name(pc);
            if (current_symbol != NULL && call_stack_depth > 0 &&
                strcmp(call_stack[call_stack_depth - 1].name, current_symbol) == 0) {
                is_ret = true;
            }
        }
    }
    ftrace_elf_start(pc, dnpc, is_call, is_ret);
}
