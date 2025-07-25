#pragma once

#include "tools.h"
#include "macro.h"


void itrace(uint32_t insts, const std::string log_file);

// void mtrace(const uint32_t pc_index, std::vector<uint32_t> &insts);

void ftrace(const uint32_t pc_index, std::vector<uint32_t> &insts);

void ftrace_elf_init(char *ftrace_file);

void ftrace(uint32_t inst);