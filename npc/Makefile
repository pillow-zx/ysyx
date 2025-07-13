# 顶层模块名称，Verilator将生成对应的C++类V$(TOPNAME)
TOPNAME = top

# Verilator工具路径
VERILATOR = verilator
# Verilator编译选项：
# -MMD: 生成依赖文件
# --build: 自动构建可执行文件
# -cc: 生成C++代码
# -O3: 高级优化
# --x-assign fast: 快速处理X态赋值
# --x-initial fast: 快速处理X态初始值
# --noassert: 禁用断言检查以提高性能
VERILATOR_CFLAGS += -MMD --build -cc  \
				-O3 --x-assign fast --x-initial fast --noassert

# 构建目录配置
BUILD_DIR = ./build              # 主构建目录
OBJ_DIR = $(BUILD_DIR)/obj_dir   # Verilator生成的对象文件目录
BIN = $(BUILD_DIR)/$(TOPNAME)    # 最终生成的可执行文件路径

# 默认目标：构建可执行文件
default: $(BIN)

# 创建构建目录（在Make解析时立即执行）
$(shell mkdir -p $(BUILD_DIR))

# 项目源文件自动发现
# 查找vsrc目录下所有.v文件（Verilog源文件）
VSRCS = $(shell find $(abspath ./vsrc) -name "*.v")
# 查找csrc目录下所有C/C++源文件
CSRCS = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cc" -or -name "*.cpp")


# Verilator编译规则配置
# 包含路径标志（当前为空，可根据需要添加头文件路径）
INCFLAGS = $(addprefix -I, $(INC_PATH))
# C++编译器标志，定义顶层模块名称宏
CXXFLAGS += $(INCFLAGS) -DTOP_NAME="\"V$(TOPNAME)\""

# 主要构建目标：生成可执行的仿真器
# 依赖：所有Verilog源文件和C++源文件
$(BIN): $(VSRCS) $(CSRCS)
	@rm -rf $(OBJ_DIR)  # 清理旧的对象文件目录
	# 调用Verilator进行编译：
	# --top-module: 指定顶层模块
	# $^: 所有依赖文件（VSRCS + CSRCS）
	# --Mdir: 指定生成文件目录
	# --exe: 生成可执行文件
	# -o: 指定输出文件名
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

# 目标别名：构建项目
all: default

# 仿真目标：构建并运行仿真器
sim: $(BIN)
	@$^  # 执行生成的仿真器可执行文件
	@echo "Simulation completed."
	$(call git_commit, "sim RTL") # 自动提交仿真结果到git（请勿删除此行）

# 清理目标：删除所有构建产物
clean:
	rm -rf $(BUILD_DIR)

# 包含父目录的Makefile（可能包含git_commit函数定义等）
include ../Makefile

# 声明伪目标（这些目标不对应实际文件）
.PHONY: default all clean run sim
