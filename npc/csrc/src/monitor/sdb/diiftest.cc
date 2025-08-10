#include <monitor.h>


extern "C" {
void difftest_init(void *regs, void *pmems);
void difftest_memcpy(unsigned int addr, void *buf, size_t n, bool direction);
void difftest_regcpy(void *dut, bool direction);
void difftest_exec(unsigned long n);
}

static unsigned int difftest_regs[NPC_BITS + 1] = {0}; // +1 for PC

void init_difftest() {
    for (int i = 0; i < NPC_BITS; i++) {
        difftest_regs[i] = core->rootp->cpu__DOT__regf__DOT__regfile[i];
    }
    difftest_regs[NPC_BITS] = core->rootp->cpu__DOT__pc;
    difftest_init(difftest_regs, memory.get_memory().data());
}

void checkregs() {
    for (int i = 0; i < NPC_BITS; i++) {
        if (difftest_regs[i] != core->rootp->cpu__DOT__regf__DOT__regfile[i]) {
            printf("Register %d mismatch: %08x != %08x\n", i, difftest_regs[i], core->rootp->cpu__DOT__regf__DOT__regfile[i]);
            exit(1);
        }
    }
}

void difftest_step() {
    difftest_exec(1);
    difftest_regcpy(difftest_regs, false);
}