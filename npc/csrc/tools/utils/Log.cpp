#include <Log.h>
#include <stdio.h>
#include <capstone/capstone.h>

void itrace(const uint32_t pc_index, std::vector<uint32_t> &insts, std::string log_file) {
    std::ofstream log_stream(log_file, std::ios::app);
    if (!log_stream) {
        std::cerr << "Failed to open log file: " << log_file << std::endl;
        return;
    }

    // 检查pc_index是否有效
    if (pc_index >= insts.size()) {
        log_stream << "Invalid PC index: " << pc_index << " (max: " << insts.size() - 1 << ")" << std::endl;
        return;
    }

    csh handle;
    cs_insn *insn;
    size_t count;

    // 初始化capstone
    if (cs_open(CS_ARCH_RISCV, CS_MODE_RISCV32, &handle) != CS_ERR_OK) {
        log_stream << "Failed to initialize capstone" << std::endl;
        return;
    }

    // 计算实际的PC地址用于显示
    uint32_t actual_pc = 0x80000000 + pc_index * 4;
    
    // 反汇编指令
    count = cs_disasm(handle, reinterpret_cast<const uint8_t*>(&insts[pc_index]), sizeof(uint32_t), actual_pc, 1, &insn);

    if(count > 0) {
        for (size_t i = 0; i < count; i++) {
            log_stream << "0x" << std::hex << insn[i].address << ":\t"
                       << insn[i].mnemonic << "\t" << insn[i].op_str << std::endl;
        }
        cs_free(insn, count);
    } else {
        log_stream << "Failed to disassemble instruction at PC index: " << pc_index 
                   << " (PC: 0x" << std::hex << actual_pc << ")" << std::endl;
    }
    cs_close(&handle);
}