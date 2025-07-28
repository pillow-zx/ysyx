module ysyx_25060173_instruction_decoder (
        input wire [31:0] inst,
        output wire inst_bge,
        output wire inst_bgeu,
        output wire inst_blt,
        output wire inst_bltu,
        output wire inst_beq,
        output wire inst_sub,
        output wire inst_add,
        output wire inst_and,
        output wire inst_bne,
        output wire inst_addi,
        output wire inst_auipc,
        output wire inst_ebreak,
        output wire inst_lui,
        output wire inst_jal,
        output wire inst_jalr,
        output wire inst_sw
    );

    // 使用直接比较的方式来判断指令类型
    wire [2:0] op_14_12;

    assign op_14_12 = inst[14:12];

    assign inst_and    = (op_14_12 == 3'h7) & (inst[31:25] == 7'h00) & (inst[6:0] == 7'b0110011);
    assign inst_sub    = (op_14_12 == 3'h0) & (inst[31:25] == 7'h20) & (inst[6:0] == 7'b0110011);
    assign inst_add    = (op_14_12 == 3'h0) & (inst[31:25] == 7'h00) & (inst[6:0] == 7'b0110011);
    assign inst_addi   = (op_14_12 == 3'h0) & (inst[6:0] == 7'b0010011);
    assign inst_beq    = (op_14_12 == 3'h0) & (inst[6:0] == 7'b1100011);
    assign inst_bge    = (op_14_12 == 3'h5) & (inst[6:0] == 7'b1100011);
    assign inst_bgeu   = (op_14_12 == 3'h7) & (inst[6:0] == 7'b1100011);
    assign inst_blt    = (op_14_12 == 3'h4) & (inst[6:0] == 7'b1100011);
    assign inst_bltu   = (op_14_12 == 3'h6) & (inst[6:0] == 7'b1100011);
    assign inst_bne    = (op_14_12 == 3'h1) & (inst[6:0] == 7'b1100011);
    assign inst_jalr   = (op_14_12 == 3'h0) & (inst[6:0] == 7'b1100111);
    assign inst_sw     = (op_14_12 == 3'h2) & (inst[6:0] == 7'b0100011);
    assign inst_ebreak = ((op_14_12 == 3'h0) & (inst[6:0] == 7'b1110011)) || (inst == 32'h0000006f);
    assign inst_jal    = inst[6:0] == 7'b1101111;
    assign inst_auipc  = inst[6:0] == 7'b0010111;
    assign inst_lui    = inst[6:0] == 7'b0110111;

endmodule

    