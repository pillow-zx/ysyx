#pragma once

#include <memory.h>
#include <cpu.h>


extern Cpu cpu;

/* init functions */
void npc_init(int argc, char *argv[]); // Function to initialize the NPC
void npc_start();

/* sdb functions */
std::vector<std::string> get_commands(std::string prompt = "(npc) > "); // Function to get a command string from the user

void show_registers(); // Function to show the CPU registers

void show_memory(size_t addr, size_t len);
