#pragma once


#include <string>
#include <cstdint>
#include <vector>


std::string get_string(std::string prompt);

uint32_t get_integer(std::string prompt);

std::vector<std::string> Stringsplit(const std::string &str, const std::string &delim);

std::vector<uint32_t> get_insts(std::string prompt);

void show_memory(uint32_t n);

void command_lists();

void welcome();

