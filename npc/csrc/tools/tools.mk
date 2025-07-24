# Tools module confitguration
TOOLS_INC_PATH = $(abspath $(CSRC_DIR)/tools/include)
TOOLS_CSRCS = $(abspath $(shell find $(CSRC_DIR)/tools/utils -name "*.c" -or -name "*.cc" -or -name "*.cpp"))

# Add to source files and include paths
INC_PATH += $(TOOLS_INC_PATH)
CSRCS += $(TOOLS_CSRCS)

# Tools info target
tools-info:
	@echo "Tools source directory: $(CSRC_DIR)/tools"
	@echo "Tools include directory: $(TOOLS_INC_PATH)"
	@echo "Tools source files:"
	@$(foreach file,$(TOOLS_CSRCS),echo "  $(file)";)

.PHONY: tools-info