#***************************************************************************************
# Copyright (c) 2014-2024 Zihao Yu, Nanjing University
#
# NEMU is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
# See the Mulan PSL v2 for more details.
#**************************************************************************************/

COLOR_RED := $(shell echo "\033[1;31m")
COLOR_END := $(shell echo "\033[0m")

# 检查 .config 文件是否存在
ifeq ($(wildcard .config),)
$(warning $(COLOR_RED)Warning: .config does not exist!$(COLOR_END))
$(warning $(COLOR_RED)To build the project, first run 'make menuconfig'.$(COLOR_END))
endif

# Q代表停止模式, 禁止命令的输出
# KCONFIG_PATH是tools下kconfig的路径, FIXDEP_PATH是tools下fixdep的路径
# Kconfig是NEMU的配置文件, rm-distclean是清理的文件列表
# silent是静默模式, 用于禁止命令的输出
Q            := @
KCONFIG_PATH := $(NEMU_HOME)/tools/kconfig
FIXDEP_PATH  := $(NEMU_HOME)/tools/fixdep
Kconfig      := $(NEMU_HOME)/Kconfig
rm-distclean += include/generated include/config .config .config.old
silent := -s

# include由menuconfig生成的变量和规则
CONF   := $(KCONFIG_PATH)/build/conf
MCONF  := $(KCONFIG_PATH)/build/mconf
FIXDEP := $(FIXDEP_PATH)/build/fixdep

$(CONF):
	$(Q)$(MAKE) $(silent) -C $(KCONFIG_PATH) NAME=conf

$(MCONF):
	$(Q)$(MAKE) $(silent) -C $(KCONFIG_PATH) NAME=mconf

$(FIXDEP):
	$(Q)$(MAKE) $(silent) -C $(FIXDEP_PATH)

# 构建目标menuconfig, savedefconfig, defconfig
# menuconfig用于更新当前配置, savedefconfig用于保存当前配置为最小配置
# defconfig用于生成指定的配置文件
menuconfig: $(MCONF) $(CONF) $(FIXDEP)
	$(Q)$(MCONF) $(Kconfig)
	$(Q)$(CONF) $(silent) --syncconfig $(Kconfig)

savedefconfig: $(CONF)
	$(Q)$< $(silent) --$@=configs/defconfig $(Kconfig)

%defconfig: $(CONF) $(FIXDEP)
	$(Q)$< $(silent) --defconfig=configs/$@ $(Kconfig)
	$(Q)$< $(silent) --syncconfig $(Kconfig)

.PHONY: menuconfig savedefconfig defconfig

# Help text used by make help
# 帮助文本, 用于make help命令
help:
	@echo  '  menuconfig	  - Update current config utilising a menu based program'
	@echo  '  savedefconfig   - Save current config as configs/defconfig (minimal config)'
	@echo  '  menuconfig      - 更新当前配置, 使用基于菜单的程序'
	@echo  '  savedefconfig   - 将当前配置保存为configs/defconfig (最小配置)'

# distclean命令用于清理构建目录和配置文件
distclean: clean
	-@rm -rf $(rm-distclean)

.PHONY: help distclean

# 清理构建目录和配置文件
define call_fixdep
	@$(FIXDEP) $(1) $(2) unused > $(1).tmp
	@mv $(1).tmp $(1)
endef
