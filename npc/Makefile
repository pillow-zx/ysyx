# Makefile for building and simulating the NPC project
TOPNAME = ysyx_25060173_core
INC_PATH ?=  # Include paths, can be set as needed

VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc --trace\
				-O3 --x-assign fast --x-initial fast --noassert

BUILD_DIR = ./build
OBJ_DIR = $(BUILD_DIR)/obj_dir
BIN = $(BUILD_DIR)/$(TOPNAME)

WAVE = ./trace.vcd

default: $(BIN)

$(shell mkdir -p $(BUILD_DIR))
$(shell mkdir -p $(OBJ_DIR))

VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")

INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

$(BIN): $(VSRCS) $(CSRCS)
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

# 目标别名：构建项目
all: default

# 添加args支持：允许用户传递运行的程序
run-env: $(BIN) $(IMG)
	@echo "Running simulation with args: $(IMG)"

# 仿真目标：构建并运行仿真器
run: run-env
	$(BIN) $(IMG)
	@echo "Simulation completed."
	$(call git_commit, "sim RTL") # 自动提交仿真结果到git（请勿删除此行）

# 查看波形
wave: $(WAVE)
	@gtkwave $(WAVE) &

# 清理目标：删除所有构建产物
clean:
	rm -rf $(BUILD_DIR)

# 包含父目录的Makefile（可能包含git_commit函数定义等）
include ../Makefile
