CPU_SRC = $(abspath $(shell find $(NPC_HOME)/csrc/src/cpu -name "*.cc"))
MONITOR_SRC = $(abspath $(shell find $(NPC_HOME)/csrc/src/monitor -name "*.cc"))
MONITOR_SRC += $(abspath $(shell find $(NPC_HOME)/csrc/src/monitor/sdb -name "*.cc"))
MEM_SRC = $(abspath $(shell find $(NPC_HOME)/csrc/src/memory -name "*.cc"))
DEV_SRC = $(abspath $(shell find $(NPC_HOME)/csrc/src/device -name "*.cc"))
DEV_SRC += $(abspath $(shell find $(NPC_HOME)/csrc/src/device/io -name "*.cc"))

# INCLUDE PATHS
INC_PATH = $(abspath $(NPC_HOME)/csrc/src/include)


