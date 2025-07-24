VERILATOR = verilator
VERILATOR_CFLAGS += -MMD --build -cc --trace\
				-O3 --x-assign fast --x-initial fast --noassert

# Find all Verilog source files
VSRCS = $(shell find $(VSRC_DIR) -name "*.v")

# Verilator build rules
verilator-build: $(VSRCS) $(CSRCS)
	@echo "Building with Verilator..."
	@rm -rf $(OBJ_DIR)
	$(VERILATOR) $(VERILATOR_CFLAGS) \
		--top-module $(TOPNAME) $^ \
		$(addprefix -CFLAGS , $(CXXFLAGS)) $(addprefix -LDFLAGS , $(LDFLAGS)) \
		--Mdir $(OBJ_DIR) --exe -o $(abspath $(BIN))

.PHONY: verilator-build
