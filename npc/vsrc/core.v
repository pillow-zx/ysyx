import "DPI-C" context function void inst_ebreak();

module ysyx_25060173_core(
        input wire clk,
        input wire reset,
        input wire [31:0] inst,
        output wire [31:0] result
    );
    reg         rst;
    always @(posedge clk) begin
        rst <= ~reset;
    end

    reg         valid;
    always @(posedge clk) begin
        if (rst) begin
            valid <= 0;
        end
        else begin
            valid <= 1;
        end
    end

    reg [31:0] pc;
    wire [31:0] nextpc;
    wire       en;

    always @(posedge clk) begin
        if (rst) begin
            pc <= 32'h80000000;
        end
        else if (valid) begin
            pc <= nextpc;
        end
    end

    assign nextpc = pc + 4;


    wire [4:0] op_11_7;
    wire [4:0] op_19_15;
    wire [4:0] op_24_20;
    wire [4:0] rd;
    wire [4:0] rs1;
    wire [4:0] rs2;

    assign op_11_7  = inst[11:7];   // 11-7
    assign op_19_15 = inst[19:15];  // 19-15
    assign op_24_20 = inst[24:20];  // 24-20
    assign rd       = inst[11:7];   // 11-7
    assign rs1      = inst[19:15];  // 19-15
    assign rs2      = inst[24:20];  // 24-20

    wire [11:0] i12;

    assign i12 = inst[31:20];  // 31-20


    wire        inst_addi;
    wire        inst_ebreak;

    ysyx_25060173_instruction_decoder u_instruction_decoder(
                                          .inst(inst),
                                          .inst_addi(inst_addi),
                                          .inst_ebreak(inst_ebreak)
                                      );
    always @(posedge clk) begin
        if (inst_ebreak) begin
            inst_ebreak();
        end
    end

    wire need_i12;
    wire [31:0] imm;
    wire [4:0] raddr1;
    wire [31:0] rdata1;
    wire [4:0] raddr2;
    wire [31:0] rdata2;
    wire [4:0] waddr;
    wire [31:0] wdata;
    wire we;

    assign need_i12 = inst_addi;
    assign imm = need_i12 ? {{20{i12[11]}}, i12} : 32'b0;  // 立即数扩展
    assign we = inst_addi ? 1'b1 : 1'b0;
    assign raddr1 = rs1;
    assign raddr2 = rs2;

    ysyx_25060173_RegisterFile u_ysyx_25060173_RegisterFile(
                                   .clk    	(clk     ),
                                   .raddr1 	(raddr1  ),
                                   .rdata1 	(rdata1  ),
                                   .raddr2 	(raddr2  ),
                                   .rdata2 	(rdata2  ),
                                   .we     	(we      ),
                                   .waddr  	(waddr   ),
                                   .wdata  	(wdata   )
                               );



    wire [31:0] alu_src1;
    wire [31:0] alu_src2;
    wire [31:0] alu_result;
    wire alu_op;

    assign alu_src1 = rdata1;
    assign alu_src2 = need_i12 ? imm : rdata2;
    assign alu_op   = inst_addi;


    ysyx_25060173_alu alu(
                          .alu_src1(alu_src1),
                          .alu_src2(alu_src2),
                          .alu_op(alu_op),
                          .alu_result(alu_result)
                      );

    assign waddr = rd;
    assign wdata = alu_result;

    assign result = alu_result;

endmodule
