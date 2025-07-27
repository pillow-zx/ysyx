#include "dpi-c.h"
#include "cpu.h"
#include <iostream>

extern "C" void ebreak_handler() {
    if (npc_STATE) {
        std::cout << "EBREAK instruction encountered at PC: " << std::hex << core->now_pc << std::dec << std::endl;
        npc_STATE = false; // Set the state to false to stop execution
    }
}