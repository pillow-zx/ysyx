#pragma once

#include <iostream>
#include "macro.h"
#include "tools.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vysyx_25060173_core.h"

extern bool npc_STATE;
extern Vysyx_25060173_core *core;

void cpu_exec(int n, std::vector<uint32_t> &insts);
