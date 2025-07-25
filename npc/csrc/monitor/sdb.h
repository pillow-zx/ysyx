#pragma once

#include "tools.h"
#include "cpu.h"

void npc_start(std::vector<uint32_t> &insts);

void npc_init(std::vector<uint32_t> &insts, int argc, char **argv);