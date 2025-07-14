module ysyx_25060173_alu(
        input wire [31:0] alu_src1,
        input wire [31:0] alu_src2,
        input wire alu_op,
        output wire [31:0] alu_result
    );

    wire op_addi;

    assign op_addi = alu_op;

    wire [31:0] adder_a;
    wire [31:0] adder_b;
    wire adder_cin;
    wire [31:0] adder_result;
    wire adder_cout;

    assign adder_a   = alu_src1;
    assign adder_b   = (op_addi) ? alu_src2 : 32'b0;  // For addi, use alu_src2, otherwise zero
    assign adder_cin = (op_addi) ? 1'b0 : 1'b0;
    assign {adder_cout, adder_result} = adder_a + adder_b + adder_cin;

    assign alu_result = (op_addi) ? adder_result : 32'b0;  // For addi, output the adder result, otherwise zero

endmodule
