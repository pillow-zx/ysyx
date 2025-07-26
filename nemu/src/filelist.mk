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

SRCS-y += src/nemu-main.c											# 参与编译的源文件的候选集合
DIRS-y += src/cpu src/monitor src/utils								# 参与编译的目录集合, 该目录下的所有文件都会被加入到SRCS-y中
DIRS-$(CONFIG_MODE_SYSTEM) += src/memory							# 如果CONFIG_MODE_SYSTEM存在, 则将src/memory添加到DIRS-$(CONFIG_MODE_SYSTEM)中
DIRS-BLACKLIST-$(CONFIG_TARGET_AM) += src/monitor/sdb				# 不参与编译的目录集合, 该目录下的所有文件都会被加入到SRCS-BLACKLIST-y中

SHARE = $(if $(CONFIG_TARGET_SHARE),1,0)
LIBS += $(if $(CONFIG_TARGET_NATIVE_ELF),-lreadline -ldl -pie,)
LIBS += $(if $(CONFIG_TARGET_SHARE),-lreadline -ldl,)

# 如果定义了mainargs变量, 则将其作为ASFLAGS的参数
ifdef mainargs
ASFLAGS += -DBIN_PATH=\"$(mainargs)\"
endif
# 如果CONFIG_TARGET_AM存在, 则将src/am-bin.S添加到SRCS-$(CONFIG_TARGET_AM)中
SRCS-$(CONFIG_TARGET_AM) += src/am-bin.S
.PHONY: src/am-bin.S
