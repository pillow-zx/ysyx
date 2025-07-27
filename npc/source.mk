# Source files configuration

# Base include paths - will be populated by filelist.mk
INC_PATH ?=

# Main source file
CSRCS = $(abspath $(CSRC_DIR)/npc-main.cpp)

# Include source file and headers from src
include $(CSRC_DIR)/filelist.mk

# Compiler flags
INCFLAGS = $(addprefix -I, $(INC_PATH))
CXXFLAGS += $(INCFLAGS) -std=c++17 -g

# Boost library configuration
BOOST_LIBS = -lboost_system -lboost_filesystem
BOOST_CFLAGS = -DBOOST_ALL_DYN_LINK

# Capstone library configuration
CAPSTONE_LIBS = -lcapstone
CAPSTONE_CFLAGS = -I/usr/include/capstone

# DIFFTEST library configuration
DIFFTEST_LIB_DIR = $(abspath lib)
DIFFTEST_LIBS = -L$(DIFFTEST_LIB_DIR) -ldifftest -ldl -Wl,-rpath,$(DIFFTEST_LIB_DIR)
DIFFTEST_CFLAGS = -DCONFIG_DIFFTEST

CXXFLAGS += $(BOOST_CFLAGS) $(CAPSTONE_CFLAGS) $(DIFFTEST_CFLAGS)
LDFLAGS += $(BOOST_LIBS) $(CAPSTONE_LIBS) $(DIFFTEST_LIBS)

# Export for sub-makefiles
export CSRCS INC_PATH CXXFLAGS LDFLAGS
