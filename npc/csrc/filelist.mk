# File list for source files and headers

# Base include paths
SRC_INC_PATH = $(abspath $(CSRC_DIR)/include)
INC_PATH += $(SRC_INC_PATH)

# Cpu source files
CPU_CSRCS = $(abspath $(shell find $(CSRC_DIR)/cpu -name "*.c" -or -name "*.cc" -or -name "*.cpp"))

# Memory source files
MEM_CSRCS = $(abspath $(shell find $(CSRC_DIR)/memory -name "*.c" -or -name "*.cc" -or -name "*.cpp"))

# Src source files
SRC_CSRCS = $(abspath $(shell find $(CSRC_DIR)/src -name "*.c" -or -name "*.cc" -or -name "*.cpp"))

# Add cpu and src sources to main CSRCS
CSRCS += $(CPU_CSRCS) $(MEM_CSRCS)
CSRCS += $(SRC_CSRCS)

# Tools source files
include $(CSRC_DIR)/tools/tools.mk

# Monitor source files
include $(CSRC_DIR)/monitor/monitor.mk


