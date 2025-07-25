# Project configuration
TOPNAME = ysyx_25060173_core
PROJECT_NAME = npc
NPC_HOME ?= $(abspath .)

# Directory structure
BUILD_DIR = $(NPC_HOME)/build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

# Log file
LOG_FILE = $(BUILD_DIR)/npc-log.txt

# Source directories
VSRC_DIR = $(NPC_HOME)/vsrc
CSRC_DIR = $(NPC_HOME)/csrc

# Output files
WAVE = $(NPC_HOME)/trace.vcd

# Create build directories
$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(OBJ_DIR))

# Export variables for sub-makefiles
export NPC_HOME BUILD_DIR OBJ_DIR TOPNAME
