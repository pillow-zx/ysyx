# Makefile for AbstractMachine Kernels and Libraries

### *Get a more readable version of this Makefile* by `make html` (requires python-markdown) 获取更易读的 Makefile 版本
html:
	cat Makefile | sed 's/^\([^#]\)/    \1/g' | markdown_py > Makefile.html
.PHONY: html

## 1. Basic Setup and Checks 基础设置和检查

### Default to create a bare-metal kernel image #默认创建一个裸机内核镜像
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS  = image
  .DEFAULT_GOAL = image
endif

### Override checks when `make clean/clean-all/html` 
### 当执行 `make clean/clean-all/html` 时，跳过检查, findstring 检查目标是否在指定的列表中
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html),)

### Print build info message	
### 打印构建信息
$(info # Building $(NAME)-$(MAKECMDGOALS) [$(ARCH)])

### Check: environment variable `$AM_HOME` looks sane
### 检查环境变量 `$AM_HOME` 是否正确, wildcard 检查文件是否存在
### 通过检查 `$(AM_HOME)/am/include/am.h` 是否存在来验证 `$AM_HOME` 是否为一个 AbstractMachine 仓库
ifeq ($(wildcard $(AM_HOME)/am/include/am.h),)
  $(error $$AM_HOME must be an AbstractMachine repo)
endif

### Check: environment variable `$ARCH` must be in the supported list 
### 检查环境变量 `$ARCH` 是否在支持的列表中, notdir 获取文件名，basename 去掉后缀名，shell 执行命令获取文件列表
### ARCH 为在cpu-tests下执行make时传入的架构, 在这里检查是否在支持的架构列表中
ARCHS = $(basename $(notdir $(shell ls $(AM_HOME)/scripts/*.mk)))
ifeq ($(filter $(ARCHS), $(ARCH)), )
  $(error Expected $$ARCH in {$(ARCHS)}, Got "$(ARCH)")
endif

### Extract instruction set architecture (`ISA`) and platform from `$ARCH`. Example: `ARCH=x86_64-qemu -> ISA=x86_64; PLATFORM=qemu`
### 将 `$ARCH` 中的指令集架构（ISA）和平台提取出来。例如：`ARCH=x86_64-qemu -> ISA=x86_64; PLATFORM=qemu`
### subst 将 `-` 替换为空格，word 1 和 word 2 分别获取第一个和第二个单词, 在这里将riscv32-nemu分割为riscv32和nemu
ARCH_SPLIT = $(subst -, ,$(ARCH))
ISA        = $(word 1,$(ARCH_SPLIT))
PLATFORM   = $(word 2,$(ARCH_SPLIT))

### Check if there is something to build	
### 检查是否有需要构建的内容, flavor 检查变量 SRCS 是否被定义, 如果未定义则报错
ifeq ($(flavor SRCS), undefined)
  $(error Nothing to build)
endif

### Checks end here
endif

## 2. General Compilation Targets 通用编译目标

### Create the destination directory (`build/$ARCH`) 
### 创建目标目录（`build/$ARCH`）
### WORK_DIR 工作目录，shell pwd 获取当前工作目录
### DST_DIR 目标目录，WORK_DIR/build/$(ARCH)
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build/$(ARCH)
$(shell mkdir -p $(DST_DIR))

### Compilation targets (a binary image or archive) 
### 编译目标（一个二进制镜像或归档文件）, abspath 获取绝对路径
IMAGE_REL = build/$(NAME)-$(ARCH)
IMAGE     = $(abspath $(IMAGE_REL))
ARCHIVE   = $(WORK_DIR)/build/$(NAME)-$(ARCH).a

### Collect the files to be linked: object files (`.o`) and libraries (`.a`) 
### 收集要链接的文件：目标文件（`.o`）和库文件（`.a`）
### addprefix 将前缀添加到每个文件名，addsuffix 将后缀添加到每个文件名，basename 去掉后缀名, sort 对文件名进行排序, join 将两个列表连接起来
OBJS      = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
LIBS     := $(sort $(LIBS) am klib) # lazy evaluation ("=") causes infinite recursions 惰性求值(=) 导致无限递归
LINKAGE   = $(OBJS) \
  $(addsuffix -$(ARCH).a, $(join \
    $(addsuffix /build/, $(addprefix $(AM_HOME)/, $(LIBS))), \
    $(LIBS) ))

## 3. General Compilation Flags 通用编译标志

### (Cross) compilers, e.g., mips-linux-gnu-g++ 
### (交叉)编译器，例如 riscv64-linux-gnu-gcc, 如果为定义CROSS_COMPILE则使用本地gcc
### ld 链接器，ar 归档工具，objdump 和 objcopy 用于处理目标文件，readelf 用于读取 ELF 文件
AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
AR        = $(CROSS_COMPILE)ar
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf

### Compilation flags	
### 编译标志, INC_PATH 包含路径，WORK_DIR/include 是工作目录下的 include 目录
### INCFLAGS 包含标志，-I 后面跟着包含路径
INC_PATH += $(WORK_DIR)/include $(addsuffix /include/, $(addprefix $(AM_HOME)/, $(LIBS)))
INCFLAGS += $(addprefix -I, $(INC_PATH))

### Linker scripts	# 链接脚本, := 获取文件名，basename 去掉后缀名，sort 对文件名进行排序
ARCH_H := arch/$(ARCH).h
### Linker scripts for the architecture, e.g., `arch/x86_64-qemu.ld` 链接脚本，例如 `arch/x86_64-qemu.ld`
CFLAGS   += -O2 -MMD -Wall -Werror $(INCFLAGS) \
            -D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__ \
            -D__ARCH__=$(ARCH) -D__ARCH_$(shell echo $(ARCH) | tr a-z A-Z | tr - _) \
            -D__PLATFORM__=$(PLATFORM) -D__PLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z | tr - _) \
            -DARCH_H=\"$(ARCH_H)\" \
            -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector \
            -Wno-main -U_FORTIFY_SOURCE -fvisibility=hidden
### Linker flags for C code, e.g., `-T arch/x86_64-qemu.ld` 链接器标志，例如 `-T arch/x86_64-qemu.ld`
CXXFLAGS +=  $(CFLAGS) -ffreestanding -fno-rtti -fno-exceptions
### Linker flags for C++ code, e.g., `-T arch/x86_64-qemu.ld` 链接器标志，例如 `-T arch/x86_64-qemu.ld`
ASFLAGS  += -MMD $(INCFLAGS)
### Linker flags for the final image, e.g., `-T arch/x86_64-qemu.ld` 链接器标志，例如 `-T arch/x86_64-qemu.ld`
LDFLAGS  += -z noexecstack $(addprefix -T, $(LDSCRIPTS))

## 4. Arch-Specific Configurations 架构特定配置

### Paste in arch-specific configurations (e.g., from `scripts/x86_64-qemu.mk`) # 粘贴架构特定配置（例如，来自 `scripts/x86_64-qemu.mk`）
-include $(AM_HOME)/scripts/$(ARCH).mk

## 5. Compilation Rules 编译规则

### Rule (compile): a single `.c` -> `.o` (gcc) # 规则（编译）：单个 `.c` -> `.o` (gcc), realpath 获取文件的绝对路径
$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@) && echo + CC $<
	@$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cc` -> `.o` (g++) # 规则（编译）：单个 `.cc` -> `.o` (g++)
$(DST_DIR)/%.o: %.cc
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cpp` -> `.o` (g++) # 规则（编译）：单个 `.cpp` -> `.o` (g++)
$(DST_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.S` -> `.o` (gcc, which preprocesses and calls as) # 规则（编译）：单个 `.S` -> `.o` (gcc，预处理并调用 as)
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo + AS $<
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

### Rule (recursive make): build a dependent library (am, klib, ...) # 规则（递归 make）：构建一个依赖的库（am, klib, ...）
$(LIBS): %:
	@$(MAKE) -s -C $(AM_HOME)/$* archive

### Rule (link): objects (`*.o`) and libraries (`*.a`) -> `IMAGE.elf`, the final ELF binary to be packed into image (ld)
### 规则（链接）：对象文件（`*.o`）和库文件（`*.a`）-> `IMAGE.elf`，最终的 ELF 二进制文件，将打包到镜像中（ld）, filter 检查 ARCH 是否为 native
$(IMAGE).elf: $(LINKAGE) $(LDSCRIPTS)
	@echo \# Creating image [$(ARCH)]
	@echo + LD "->" $(IMAGE_REL).elf
ifneq ($(filter $(ARCH),native),)
	@$(CXX) -o $@ -Wl,--whole-archive $(LINKAGE) -Wl,-no-whole-archive $(LDFLAGS_CXX)
else
	@$(LD) $(LDFLAGS) -o $@ --start-group $(LINKAGE) --end-group
endif

### Rule (archive): objects (`*.o`) -> `ARCHIVE.a` (ar) # 规则（归档）：对象文件（`*.o`）-> `ARCHIVE.a` (ar), realpath 获取文件的相对路径
$(ARCHIVE): $(OBJS)
	@echo + AR "->" $(shell realpath $@ --relative-to .)
	@$(AR) rcs $@ $^

### Rule (`#include` dependencies): paste in `.d` files generated by gcc on `-MMD`
### 规则（`#include` 依赖）：粘贴由 gcc 在 `-MMD` 上生成的 `.d` 文件
-include $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))

## 6. Miscellaneous 杂项

### Build order control	# 构建顺序控制
image: image-dep
archive: $(ARCHIVE)
image-dep: $(LIBS) $(IMAGE).elf
.NOTPARALLEL: image-dep
.PHONY: image image-dep archive run $(LIBS)

### Clean a single project (remove `build/`) # 清理单个项目（删除 `build/`）
clean:
	rm -rf Makefile.html $(WORK_DIR)/build/
.PHONY: clean

### Clean all sub-projects within depth 2 (and ignore errors) # 清理深度为 2 的所有子项目（并忽略错误）
CLEAN_ALL = $(dir $(shell find . -mindepth 2 -name Makefile))
clean-all: $(CLEAN_ALL) clean
$(CLEAN_ALL):
	-@$(MAKE) -s -C $@ clean
.PHONY: clean-all $(CLEAN_ALL)
