# VERILATOR
VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc --trace\
				-O2 --x-assign fast --x-initial fast --noassert

# Find all Verilog source files
VSRCS = $(shell find $(VSRC_DIR) -name "*.v")

# Verilator build rules
verilator-build: $(VSRCS) $(SRC)
	@echo "Building with Verilator..."
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $(VSRCS) $(SRC) \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

# Project configuration
TOPNAME = cpu
PROJECT_NAME = npc

# Directory structure
BUILD_DIR = $(NPC_HOME)/build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)


# Create build directories
$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(OBJ_DIR))


# Source files configuration
INC_PATH ?=

# MAIN SOURCE FILE
MAIN = $(abspath $(shell pwd)/csrc/npc-main.cc)

include $(NPC_HOME)/csrc/filelist.mk

# SRC
SRC = $(MEM_SRC) $(MONITOR_SRC) $(CPU_SRC) $(DEV_SRC) $(MAIN)

# INCLUDE SOURCES FILES
INCFLAGS += $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -std=c++17 -g

# Boost library
BOOST_INCLUDE = -DBOOST_ALL_DYN_LINK
BOOST_LIB = -lboost_program_options

# Capstone library
CAPSTONE_LIB = -lcapstone
CAPSTONE_CFLAGS = -I/usr/include/capstone

# Difftest library
DIFFTEST_LIB = $(abspath $(NPC_HOME)/lib/libdifftest.a)

CXXFLAGS += -I$(BOOST_INCLUDE) $(CAPSTONE_CFLAGS)
LDFLAGS += $(BOOST_LIB) $(CAPSTONE_LIB) $(DIFFTEST_LIB)

# Default target
default: all

# Main build target
all: $(BIN)

# Binary build target
$(BIN): verilator-build

# Log file
LOG_FILE ?= $(BUILD_DIR)/log.txt

# Run Arguments
ARG ?= --image=$(IMG) --log=$(LOG_FILE) --elf=$(ELF) --batch

# Run targets
run: $(BIN)
	@echo "Running simulation with args: $(ARG)"
	$(BIN) $(ARG)
	@echo "Simulation completed."

# Clean targets
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR)

-include ../Makefile
