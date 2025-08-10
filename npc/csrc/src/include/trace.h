#pragma once

#include <capstone/capstone.h>
#include <macro.h>
#include <iostream>
#include <cstdint>
#include <string>
#include <fstream>
#include <elf.h>
#include <cstring> // For strcmp and strncpy
#include "Vcpu.h"
#include "Vcpu___024root.h"

// 包含 Capstone RISC-V 架构的头文件，用于获取寄存器和指令ID
#include <capstone/riscv.h>

extern Vcpu *core; // Global Vcpu instance

extern Elf32_Ehdr *ftrace_file_header;
extern Elf32_Shdr *ftrace_section_header;
extern Elf32_Sym *ftrace_file_symtab;
extern char *ftrace_file_strtab;
extern int ftrace_file_symtab_num;

#define CALL_FUNC_TIMES 64

class Traces {
  private:
    std::ofstream itrace_file;
    csh handle;
    cs_insn *insn;
    size_t count;

  public:
    Traces() {
        insn = nullptr;
        count = 0;
    }

    ~Traces() {
        if (itrace_file.is_open()) {
            itrace_file.close();
        }
        if (insn) {
            // cs_free(insn, count); // This is not needed here as it's done in itrace_handle
            insn = nullptr;
        }
        if (handle) {
            cs_close(&handle);
        }
        free(ftrace_file_header);
        free(ftrace_section_header);
        free(ftrace_file_symtab);
        free(ftrace_file_strtab);
        ftrace_file_header = nullptr;
        ftrace_section_header = nullptr;
        ftrace_file_symtab = nullptr;
        ftrace_file_strtab = nullptr;
    }

    void itrace_init(std::string filename) {
        itrace_file.open(filename);
        if (!itrace_file.is_open()) {
            std::cerr << "Error opening trace file: " << filename << std::endl;
            throw std::runtime_error("Failed to open trace file");
        }
        // 确保 Capstone 只打开一次
        if (cs_open(CS_ARCH_RISCV, CS_MODE_RISCV32, &handle) != CS_ERR_OK) {
            std::cerr << "Failed to initialize Capstone disassembler" << std::endl;
            throw std::runtime_error("Capstone initialization failed");
        }
    }

    void itrace_handle(uint32_t pc, uint32_t inst) {
        // 使用 CS_OPT_DETAIL 启用详细模式，以获取操作数等信息
        cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
        
        count = cs_disasm(handle, reinterpret_cast<const uint8_t *>(&inst), sizeof(inst), pc, 0, &insn);

        if (count > 0) {
            for (size_t i = 0; i < count; i++) {
                itrace_file << "PC: 0x" << std::hex << pc << ", Instruction: " << insn[i].mnemonic << " "
                            << insn[i].op_str << std::endl;
            }
            cs_free(insn, count);
        } else {
            itrace_file << "PC: 0x" << std::hex << pc << ", Instruction: " << "UNKNOWN" << std::endl;
        }
    }

  private:
    FILE *fp = nullptr;
    struct {
        uint32_t pc;
        char name[256];
    } call_stack[CALL_FUNC_TIMES];
    int call_stack_top = 0;

  public:
    void ftrace_init(std::string filename) {
        fp = fopen(filename.c_str(), "rb");
        if (!fp) {
            std::cerr << "Error opening trace file: " << filename << std::endl;
            throw std::runtime_error("Failed to open trace file");
        }

        ftrace_file_header = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
        size_t ret = fread(ftrace_file_header, sizeof(Elf32_Ehdr), 1, fp);
        if (ret != 1) { /* handle read error */ }
        fseek(fp, ftrace_file_header->e_shoff, SEEK_SET);
        ftrace_section_header = (Elf32_Shdr *)malloc(ftrace_file_header->e_shnum * sizeof(Elf32_Shdr));
        ret = fread(ftrace_section_header, sizeof(Elf32_Shdr), ftrace_file_header->e_shnum, fp);
        if (ret != ftrace_file_header->e_shnum) { /* handle read error */ }

        for (int i = 0; i < ftrace_file_header->e_shnum; i++) {
            if (ftrace_section_header[i].sh_type == SHT_SYMTAB) {
                fseek(fp, ftrace_section_header[i].sh_offset, SEEK_SET);
                ftrace_file_symtab = (Elf32_Sym *)malloc(ftrace_section_header[i].sh_size);
                ret = fread(ftrace_file_symtab, ftrace_section_header[i].sh_size, 1, fp);
                if (ret != 1) { /* handle read error */ }
                ftrace_file_symtab_num = ftrace_section_header[i].sh_size / sizeof(Elf32_Sym);

                int strtab_index = ftrace_section_header[i].sh_link;
                if (strtab_index < ftrace_file_header->e_shnum) {
                    fseek(fp, ftrace_section_header[strtab_index].sh_offset, SEEK_SET);
                    ftrace_file_strtab = (char *)malloc(ftrace_section_header[strtab_index].sh_size);
                    ret = fread(ftrace_file_strtab, ftrace_section_header[strtab_index].sh_size, 1, fp);
                    if (ret != 1) { /* handle read error */ }
                }
                break;
            }
        }
        fclose(fp);
        fp = nullptr;
    }

    void ftrace_handle(uint32_t inst) {
        bool is_call = false, is_ret = false;

        // 设置 Capstone 选项以获取详细信息
        cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
        cs_insn *insn = nullptr;
        size_t disasm_count = cs_disasm(handle, reinterpret_cast<const uint8_t *>(&inst), sizeof(inst), core->rootp->cpu__DOT__pc, 0, &insn);
        
        if (disasm_count > 0) {
            // Capstone JALR 指令的 ID
            if (insn->id == RISCV_INS_JAL) {
                is_call = true;
            } else if (insn->id == RISCV_INS_JALR) {
                // 检查 JALR 是否为 RET 指令的伪指令形态: jalr x0, 0(x1)
                // 目标寄存器是 x0 (RISCV_REG_X0)
                // 源寄存器是 x1 (RISCV_REG_X1) 即 ra
                if (insn->detail && insn->detail->riscv.op_count == 2) {
                    const cs_riscv_op &op0 = insn->detail->riscv.operands[0];
                    const cs_riscv_op &op1 = insn->detail->riscv.operands[1];
                    
                    if (op0.type == RISCV_OP_REG && op0.reg == RISCV_REG_X0 && 
                        op1.type == RISCV_OP_REG && op1.reg == RISCV_REG_X1) {
                        is_ret = true;
                    }
                }
            }
            cs_free(insn, disasm_count);
        }
        
        ftrace_elf_start(core->rootp->cpu__DOT__pc, core->rootp->cpu__DOT__nextpc, is_call, is_ret);
    }

  private:
    char *get_symbol_name(uint32_t pc) {
        if (ftrace_file_symtab == nullptr || ftrace_file_strtab == nullptr) {
            return nullptr;
        }
        for (int i = 0; i < ftrace_file_symtab_num; i++) {
            // 修正：通过符号地址范围来查找函数名
            if (ELF32_ST_TYPE(ftrace_file_symtab[i].st_info) == STT_FUNC) {
                if (pc >= ftrace_file_symtab[i].st_value && pc < ftrace_file_symtab[i].st_value + ftrace_file_symtab[i].st_size) {
                    return ftrace_file_strtab + ftrace_file_symtab[i].st_name;
                }
            }
        }
        return nullptr;
    }

    void ftrace_elf_start(uint32_t pc, uint32_t dnpc, bool is_call, bool is_ret) {
        if (ftrace_file_header == nullptr || ftrace_section_header == nullptr || ftrace_file_symtab == nullptr ||
            ftrace_file_strtab == nullptr) {
            std::cerr << "Ftrace ELF file not initialized properly." << std::endl;
            return;
        }

        if (is_call) {
            char *target_symbol = get_symbol_name(dnpc);
            if (target_symbol != nullptr) {
                bool is_duplicate = false;
                if (call_stack_top > 0) {
                    is_duplicate = (strcmp(call_stack[call_stack_top].name, target_symbol) == 0);
                }

                if (!is_duplicate) {
                    for (int i = 0; i < call_stack_top; i++) {
                        std::cout << "  "; // 使用两个空格增加缩进
                    }
                    std::cout << "Call: " << target_symbol << " at PC: 0x" << std::hex << pc << std::endl;

                    if (call_stack_top < CALL_FUNC_TIMES) {
                        call_stack[call_stack_top].pc = dnpc;
                        strncpy(call_stack[call_stack_top].name, target_symbol,
                                sizeof(call_stack[call_stack_top].name) - 1);
                        call_stack[call_stack_top].name[sizeof(call_stack[call_stack_top].name) - 1] = '\0';
                        call_stack_top++;
                    } else {
                        std::cerr << "Call stack overflow, increase CALL_FUNC_TIMES." << std::endl;
                    }
                }
            }
        } else if (is_ret && call_stack_top > 0) {
            char *current_symbol = get_symbol_name(pc);
            // 修正：strcmp返回0表示相等，所以我们用 == 0 判断
            // 并且只有当栈顶函数名和当前函数名匹配时才出栈
            if (current_symbol != nullptr && strcmp(call_stack[call_stack_top - 1].name, current_symbol) == 0) {
                call_stack_top--;
                for (int i = 0; i < call_stack_top; i++) {
                    std::cout << "  ";
                }
                std::cout << "Return: " << current_symbol << " at PC: 0x" << std::hex << pc << std::endl;
            } else {
                // 如果函数返回地址不匹配，说明栈可能出现问题，在此打印警告
                std::cerr << "Warning: Mismatched return symbol at PC: 0x" << std::hex << pc << std::endl;
            }
        }
        // 移除不正确的 catch-all 出栈逻辑
    }
};

extern Traces MyTrace;
