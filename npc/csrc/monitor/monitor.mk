# Monitor module configuration
MONITOR_INC_PATH = $(abspath $(CSRC_DIR)/monitor/include)
MONITOR_CSRCS = $(abspath $(shell find $(CSRC_DIR)/monitor -name "*.c" -or -name "*.cc" -or -name "*.cpp"))

# Add to source files and include paths if monitor sources exist
ifneq ($(MONITOR_CSRCS),)
    INC_PATH += $(MONITOR_INC_PATH)
    CSRCS += $(MONITOR_CSRCS)
endif

# Monitor info target
monitor-info:
	@echo "Monitor source directory: $(CSRC_DIR)/monitor"
	@echo "Monitor include directory: $(MONITOR_INC_PATH)"
	@echo "Monitor source files: $(MONITOR_CSRCS)"

.PHONY: monitor-info
