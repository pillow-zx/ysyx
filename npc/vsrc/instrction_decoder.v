module ysyx_25060173_instruction_decoder (
    input wire [31:0] inst,
    output wire inst_addi,
    output wire inst_ebreak
);

  // 使用直接比较的方式来判断指令类型
  wire [2:0] op_14_12;
  wire [7:0] op_14_12_d;

  assign op_14_12 = inst[14:12];

  ysyx_25060173_decoder_3_8 u_ysyx_25060173_decoder_3_8 (
      .in (op_14_12),
      .out(op_14_12_d)
  );

  assign inst_ebreak = op_14_12_d[3'h00] & (inst[6:0] == 7'b1110011);
  assign inst_addi   = op_14_12_d[3'h00] & (inst[6:0] == 7'b0010011);

endmodule

