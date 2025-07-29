module ysyx_25060173_alu (
    input  wire [31:0] alu_src1,
    input  wire [31:0] alu_src2,
    input  wire [10:0] alu_op,
    output wire [31:0] alu_result
);

  wire op_addi;
  wire op_auipc;
  wire op_add;
  wire op_sub;
  wire op_and;
  wire op_beq;
  wire op_bne;
  wire op_bge;
  wire op_bgeu;
  wire op_blt;
  wire op_bltu;


  assign op_addi  = alu_op[0];
  assign op_auipc = alu_op[1];
  assign op_add   = alu_op[2];
  assign op_sub   = alu_op[3];
  assign op_and   = alu_op[4];
  assign op_bne   = alu_op[5];
  assign op_bge   = alu_op[6];
  assign op_bgeu  = alu_op[7];
  assign op_blt   = alu_op[8];
  assign op_bltu  = alu_op[9];
  assign op_beq   = alu_op[10];

  wire [31:0] add_sub_result;
  wire [31:0] and_result;
  wire [31:0] bge_bltu_result;
  wire [31:0] beq_bgeu_bltu_bne_result;

  wire [31:0] adder_a;
  wire [31:0] adder_b;
  wire        adder_cin;
  wire [31:0] adder_result;
  wire        adder_cout;

  assign adder_a = alu_src1;
  assign adder_b = op_sub | op_beq | op_bne | op_bgeu | op_bltu ? ~alu_src2 : alu_src2;
  assign adder_cin = op_sub | op_beq | op_bne | op_bgeu | op_bltu ? 1'b1 : 1'b0;

  assign {adder_cout, adder_result} = adder_a + adder_b + {{31{1'b0}}, adder_cin};


  assign bge_bltu_result[31:1] = 31'b0;  // Set the upper bits to zero
  assign bge_bltu_result[0] = (alu_src1[31] & ~alu_src2[31]) |
           (~(alu_src1[31] ^ alu_src2[31]) & adder_result[31]);
  assign beq_bgeu_bltu_bne_result[31:1] = 31'b0;  // Set the upper bits to zero
  assign beq_bgeu_bltu_bne_result[0] = ~adder_cout;

  assign add_sub_result = adder_result;
  assign and_result = alu_src1 & alu_src2;

  assign alu_result = op_and ? and_result :
           op_bge | op_blt ? bge_bltu_result :
           op_bgeu | op_bltu ? beq_bgeu_bltu_bne_result :
           add_sub_result;

endmodule
