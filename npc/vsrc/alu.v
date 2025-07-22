module ysyx_25060173_alu (
    input  wire [31:0] alu_src1,
    input  wire [31:0] alu_src2,
    input  wire [ 1:0] alu_op,
    output wire [31:0] alu_result
);

  wire op_addi;
  wire op_auipc;

  assign op_addi  = alu_op[0];
  assign op_auipc = alu_op[1];

  wire [31:0] addi_result;
  wire [31:0] auipc_result;

  wire [31:0] adder_a;
  wire [31:0] adder_b;
  wire adder_cin;
  wire [31:0] adder_result;
  wire adder_cout;

  assign adder_a = alu_src1;
  assign adder_b = alu_src2;
  assign adder_cin = 1'b0;

  assign {adder_cout, adder_result} = adder_a + adder_b + {{31{1'b0}}, adder_cin};

  assign addi_result = adder_result;
  assign auipc_result = adder_result;

  assign alu_result = (op_addi ? addi_result : op_auipc ? auipc_result : 32'b0);

endmodule
