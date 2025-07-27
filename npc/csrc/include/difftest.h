#pragma once

#include <dlfcn.h>

void init_difftest(unsigned int *core_regs, void *pmems);

void difftest_step_and_check(unsigned int *core_regs,unsigned int pc, unsigned int dnpc, unsigned int inst);

