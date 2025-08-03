#pragma once

#include <dlfcn.h>

void init_difftest(unsigned int *core_regs, void *pmems);

void difftest_step();

void checkregs();