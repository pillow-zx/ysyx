/* verilator lint_off DECLFILENAME*/
module ysyx_25060173_instruction_decoder (
        input wire [31:0] inst,
        output wire inst_bge,
        output wire inst_bgeu,
        output wire inst_blt,
        output wire inst_bltu,
        output wire inst_beq,
        output wire inst_sub,
        output wire inst_add,
        output wire inst_lb, // 新增加载字节指令
        output wire inst_slli,
        output wire inst_srli, // 新增逻辑右移立即数指令
        output wire inst_and,
        output wire inst_sll, // 新增逻辑左移指令
        output wire inst_bne,
        output wire inst_xori, // 新增逻辑异或立即数指令
        output wire inst_sltu,
        output wire inst_sb, // 新增存字节指令
        output wire inst_xor,
        output wire inst_srai, // 新增算术右移立即数指令
        output wire inst_or, // 新增逻辑或指令
        output wire inst_ori, // 新增逻辑或立即数指令
        output wire inst_slti, // 新增有符号小于立即数指令
        output wire inst_lbu,
        output wire inst_addi,
        output wire inst_auipc,
        output wire inst_sh, // 新增加载半字指令
        output wire inst_ebreak,
        output wire inst_sltiu,
        output wire inst_lui,
        output wire inst_lhu, // 新增加载半字指令
        output wire inst_lw,
        output wire inst_srl, // 新增逻辑右移指令
        output wire inst_lh, // 新增加载半字指令
        output wire inst_jal,
        output wire inst_jalr,
        output wire inst_sra, // 新增算术右移指令
        output wire inst_slt,
        output wire inst_andi,
        output wire inst_sw
    );

    // 使用直接比较的方式来判断指令类型
    wire [2:0] op_14_12;

    assign op_14_12    = inst[14:12];

    assign inst_and    = (op_14_12 == 3'h7) & (inst[31:25] == 7'h00) & (inst[6:0] == 7'b0110011);
    assign inst_sub    = (op_14_12 == 3'h0) & (inst[31:25] == 7'h20) & (inst[6:0] == 7'b0110011);
    assign inst_add    = (op_14_12 == 3'h0) & (inst[31:25] == 7'h00) & (inst[6:0] == 7'b0110011);
    assign inst_slli   = (op_14_12 == 3'h1) & (inst[31:25] == 7'h00) & (inst[6:0] == 7'b0010011);
    assign inst_sltu   = (op_14_12 == 3'h3) & (inst[31:25] == 7'h00) & (inst[6:0] == 7'b0110011);
    assign inst_xor    = (op_14_12 == 3'h4) & (inst[31:25] == 7'h00) & (inst[6:0] == 7'b0110011);
    assign inst_or     = (op_14_12 == 3'h6) & (inst[31:25] == 7'h00) & (inst[6:0] == 7'b0110011); // 新增逻辑或指令
    assign inst_slt    = (op_14_12 == 3'h2) & (inst[31:25] == 7'h00) & (inst[6:0] == 7'b0110011);
    assign inst_sra    = (op_14_12 == 3'h5) & (inst[31:25] == 7'h20) & (inst[6:0] == 7'b0110011); // 新增算术右移指令
    assign inst_srl    = (op_14_12 == 3'h5) & (inst[31:25] == 7'h00) & (inst[6:0] == 7'b0110011); // 新增逻辑右移指令
    assign inst_sll    = (op_14_12 == 3'h1) & (inst[31:25] == 7'h00) & (inst[6:0] == 7'b0110011); // 新增逻辑左移指令
    assign inst_srli   = (op_14_12 == 3'h5) & (inst[31:25] == 7'h00) & (inst[6:0] == 7'b0010011); // 新增逻辑右移立即数指令
    assign inst_srai   = (op_14_12 == 3'h5) & (inst[31:25] == 7'h20) & (inst[6:0] == 7'b0010011); // 新增算术右移立即数指令
    assign inst_sh     = (op_14_12 == 3'h1) & (inst[6:0] == 7'b0100011); // 新增加载半字指令
    assign inst_sb     = (op_14_12 == 3'h0) & (inst[6:0] == 7'b0100011); // 新增存字节指令
    assign inst_lh     = (op_14_12 == 3'h1) & (inst[6:0] == 7'b0000011); // 新增加载半字指令
    assign inst_xori   = (op_14_12 == 3'h4) & (inst[6:0] == 7'b0010011); // 新增逻辑异或立即数指令
    assign inst_lhu    = (op_14_12 == 3'h5) & (inst[6:0] == 7'b0000011); // 新增加载半字指令
    assign inst_lb     = (op_14_12 == 3'h0) & (inst[6:0] == 7'b0000011); // 新增加载字节指令
    assign inst_andi   = (op_14_12 == 3'h7) & (inst[6:0] == 7'b0010011); // 新增逻辑与立即数指令
    assign inst_ori    = (op_14_12 == 3'h6) & (inst[6:0] == 7'b0010011); // 新增逻辑或立即数指令
    assign inst_slti   = (op_14_12 == 3'h2) & (inst[6:0] == 7'b0010011); // 新增有符号小于立即数指令
    assign inst_lbu    = (op_14_12 == 3'h4) & (inst[6:0] == 7'b0000011);
    assign inst_sltiu  = (op_14_12 == 3'h3) & (inst[6:0] == 7'b0010011);
    assign inst_lw     = (op_14_12 == 3'h2) & (inst[6:0] == 7'b0000011);
    assign inst_addi   = (op_14_12 == 3'h0) & (inst[6:0] == 7'b0010011);
    assign inst_beq    = (op_14_12 == 3'h0) & (inst[6:0] == 7'b1100011);
    assign inst_bge    = (op_14_12 == 3'h5) & (inst[6:0] == 7'b1100011);
    assign inst_bgeu   = (op_14_12 == 3'h7) & (inst[6:0] == 7'b1100011);
    assign inst_blt    = (op_14_12 == 3'h4) & (inst[6:0] == 7'b1100011);
    assign inst_bltu   = (op_14_12 == 3'h6) & (inst[6:0] == 7'b1100011);
    assign inst_bne    = (op_14_12 == 3'h1) & (inst[6:0] == 7'b1100011);
    assign inst_jalr   = (op_14_12 == 3'h0) & (inst[6:0] == 7'b1100111);
    assign inst_sw     = (op_14_12 == 3'h2) & (inst[6:0] == 7'b0100011);
    assign inst_ebreak = (inst == 32'h00100073) || (inst == 32'h0000006f); // EBREAK指令, 这里添加对无限循环的支持
    assign inst_jal    = inst[6:0] == 7'b1101111;
    assign inst_auipc  = inst[6:0] == 7'b0010111;
    assign inst_lui    = inst[6:0] == 7'b0110111;
endmodule


