import "DPI-C" context function void ebreak_handler();

module ysyx_25060173_core (
    input wire clk,
    input wire reset,
    input wire [31:0] inst,
    output wire [31:0] result,
    output wire [31:0] next_pc,
    output wire [31:0] now_pc
);
  reg rst;
  always @(posedge clk) begin
    rst <= ~reset;
  end

  reg valid;
  always @(posedge clk) begin
    if (rst) begin
      valid <= 0;
    end else begin
      valid <= 1;
    end
  end

  reg  [31:0] pc;
  wire [31:0] nextpc;
  wire        en;

  always @(posedge clk) begin
    if (rst) begin
      pc <= 32'h80000000;
    end else if (valid) begin
      pc <= nextpc;
    end
  end

  wire [4:0] op_11_7;
  wire [4:0] op_19_15;
  wire [4:0] op_24_20;
  wire [4:0] rd;
  wire [4:0] rs1;
  wire [4:0] rs2;

  assign op_11_7  = inst[11:7];  // 11-7
  assign op_19_15 = inst[19:15];  // 19-15
  assign op_24_20 = inst[24:20];  // 24-20
  assign rd       = inst[11:7];  // 11-7
  assign rs1      = inst[19:15];  // 19-15
  assign rs2      = inst[24:20];  // 24-20

  wire [ 6:0] i7;
  wire [11:0] i12;
  wire [19:0] i20;

  assign i12 = inst[31:20];  // 31-20
  assign i20 = inst[31:12];  // 31-12

  wire inst_addi;
  wire inst_auipc;
  wire inst_lui;
  wire inst_jal;
  wire inst_jalr;
  wire inst_sw;
  wire inst_ebreak;

  ysyx_25060173_instruction_decoder u_instruction_decoder (
      .inst(inst),
      .inst_addi(inst_addi),
      .inst_auipc(inst_auipc),
      .inst_ebreak(inst_ebreak),
      .inst_lui(inst_lui),
      .inst_jal(inst_jal),
      .inst_jalr(inst_jalr),
      .inst_sw(inst_sw)
  );
  always @(posedge clk) begin
    if (inst_ebreak) begin
      ebreak_handler();
    end
  end

  wire need_i7;
  wire need_i12;
  wire need_i20;
  wire need_i20_jal;
  wire [31:0] imm;
  wire [4:0] raddr1;
  wire [31:0] rdata1;
  wire [4:0] raddr2;
  wire [31:0] rdata2;
  wire [4:0] waddr;
  wire [31:0] wdata;
  wire we;

  assign need_i7 = inst_sw;
  assign need_i20_jal = inst_jal;
  assign need_i12 = inst_addi | inst_jalr;
  assign need_i20 = inst_auipc | inst_lui;
  assign imm = need_i7 ? {{25{i7[6]}}, i7}          :
               need_i12 ? {{20{i12[11]}}, i12}       :
               need_i20 ? {{12{i20[19]}}, i20} << 12 :
               need_i20_jal ? {{12{inst[31]}}, inst[19:12], inst[20], inst[30:21], 1'b0} :
               32'b0;  // 立即数扩展
  assign we = inst_addi | inst_auipc | inst_lui | inst_jal | inst_jalr | inst_sw ? 1'b1 : 1'b0;
  assign raddr1 = rs1;
  assign raddr2 = rs2;

  ysyx_25060173_RegisterFile u_ysyx_25060173_RegisterFile (
      .clk   (clk),
      .raddr1(raddr1),
      .rdata1(rdata1),
      .raddr2(raddr2),
      .rdata2(rdata2),
      .we    (we),
      .waddr (waddr),
      .wdata (wdata)
  );



  wire [31:0] alu_src1;
  wire [31:0] alu_src2;
  wire [31:0] alu_result;
  wire [ 1:0] alu_op;

  assign alu_src1  = inst_auipc | inst_jal ? pc : rdata1;
  assign alu_src2  = need_i12 | need_i20 ? imm : need_i20_jal ? imm : rdata2;
  assign alu_op[0] = inst_addi;  // ALU operation for ADDI
  assign alu_op[1] = inst_auipc;  // ALU operation for AUIPC


  ysyx_25060173_alu u_alu (
      .alu_src1(alu_src1),
      .alu_src2(alu_src2),
      .alu_op(alu_op),
      .alu_result(alu_result)
  );

  assign waddr  = rd;
  assign wdata  = inst_jalr | inst_jal ? pc + 4 : alu_result;

  assign result = alu_result;

  assign nextpc = inst_jal ? pc + imm : inst_jalr ? (rdata1 + imm) & ~1 : pc + 4;

  assign next_pc = nextpc;
  assign now_pc = pc;

endmodule
