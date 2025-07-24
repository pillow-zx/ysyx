#pragma once

#include "macro.h"
#include "tools.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vysyx_25060173_core.h"
#include "Vysyx_25060173_core___024root.h"

const std::vector<std::string> regs = {"$0", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
                                       "a1", "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
                                       "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

extern bool npc_STATE;
extern Vysyx_25060173_core *core;

void cpu_exec(int n, std::vector<uint32_t> &insts);

void reset();

void show_regs();
