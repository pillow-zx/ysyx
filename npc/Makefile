# NPC Project Main Makefile

# Set default goal explicitly before including any other files
.DEFAULT_GOAL := default

# Project configuration
include config.mk

# Source files configuration
include source.mk

# Verilator configuration
include verilator.mk

# Default target
default: all

# Main build target
all: $(BIN)

# Binary build target
$(BIN): verilator-build

# Run targets
run-env: $(BIN) $(IMG)
	@echo "Running simulation with args: $(IMG)"

run: run-env
	$(BIN) -i$(IMG) -l$(LOG_FILE) -f$(ELF)
	@echo "Simulation completed."

# Additional targets
wave: $(WAVE)
	@gtkwave $(WAVE) &

# Clean target
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)

# Info targets
info:
	@echo "Project: $(PROJECT_NAME)"
	@echo "Top module: $(TOPNAME)"
	@echo "Build directory: $(BUILD_DIR)"
	@echo "Binary: $(BIN)"
	@echo "Verilog sources: $(words $(VSRCS)) files"
	@echo "C/C++ sources: $(words $(CSRCS)) files"

help:
	@echo "Available targets:"
	@echo "  all/default  - Build the project"
	@echo "  run          - Build and run simulation"
	@echo "  clean        - Clean build files"
	@echo "  wave         - Open waveform viewer"
	@echo "  info         - Show project information"
	@echo "  help         - Show this help"

.PHONY: default all clean info help run run-env wave

-include ../Makefile

