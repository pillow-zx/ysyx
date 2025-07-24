#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <sstream>
#include <fstream>


std::string get_string(std::string prompt);

uint32_t get_integer(std::string prompt);

std::vector<uint32_t> get_insts(std::string prompt);

void command_lists();

void welcome();

