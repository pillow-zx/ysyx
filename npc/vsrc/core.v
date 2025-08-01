import "DPI-C" context function void ebreak_handler();
import "DPI-C" context function int pmem_read(input int addr);
import "DPI-C" context function void pmem_write(input int addr, input int data);
/* verilator lint_off DECLFILENAME*/
module ysyx_25060173_core (
        input wire clk,
        input wire reset,
        input wire [31:0] inst,
        output wire [31:0] result,
        output wire [31:0] next_pc,
        output reg [31:0] now_pc
    );
    reg rst;
    always @(posedge clk) begin
        rst <= ~reset;
    end

    reg valid;
    always @(posedge clk) begin
        if (rst) begin
            valid <= 0;
        end
        else begin
            valid <= 1;
        end
    end

    reg  [31:0] pc;
    wire [31:0] nextpc;
    wire        en;

    always @(posedge clk) begin
        if (rst) begin
            pc <= 32'h80000000;
        end
        else if (valid) begin
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

    wire [11:0] I_imm;
    wire [11:0] S_imm;
    wire [12:0] B_imm;
    wire [19:0] U_imm;
    wire [20:0] J_imm;

    assign I_imm = inst_ebreak | inst_addi ? inst[31:20] : 12'b0;  // 31-20
    assign S_imm = inst_sw ? {inst[31:25], inst[11:7]} : 12'b0;  // 31-25, 11-7;
    assign B_imm = inst_beq ? {inst[31], inst[7], inst[30:25], inst[11:8], 1'b0} : 13'b0;
    assign U_imm = inst[31:12];  // 31-12
    assign J_imm = {inst[31], inst[19:12], inst[20], inst[30:21], 1'b0};  // 31, 19-12, 20, 30-21, 0}

    // R 型指令
    wire inst_sub;
    wire inst_add;
    wire inst_and;
    // I 型指令
    wire inst_jalr;
    wire inst_ebreak;
    wire inst_addi;
    // S 型指令
    wire inst_sw;
    // B 型指令
    wire inst_beq;
    wire inst_bge;
    wire inst_bgeu;
    wire inst_blt;
    wire inst_bltu;
    wire inst_bne;
    // U 型指令
    wire inst_auipc;
    wire inst_lui;
    // J 型指令
    wire inst_jal;

    ysyx_25060173_instruction_decoder u_instruction_decoder (
                                          .inst(inst),
                                          .inst_beq(inst_beq),
                                          .inst_bge(inst_bge),
                                          .inst_bgeu(inst_bgeu),
                                          .inst_blt(inst_blt),
                                          .inst_bltu(inst_bltu),
                                          .inst_sub(inst_sub),
                                          .inst_bne(inst_bne),
                                          .inst_add(inst_add),
                                          .inst_and(inst_and),
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

    wire need_I_imm;
    wire need_S_imm;
    wire need_B_imm;
    wire need_U_imm;
    wire need_J_imm;

    wire [31:0] imm;
    wire [4:0] raddr1;
    wire [31:0] rdata1;
    wire [4:0] raddr2;
    wire [31:0] rdata2;
    wire [4:0] waddr;
    wire [31:0] wdata;
    wire we;

    assign need_I_imm = inst_addi | inst_ebreak | inst_jalr;
    assign need_S_imm = inst_sw;
    assign need_B_imm = inst_beq | inst_bge | inst_bgeu | inst_blt | inst_bltu | inst_bne;
    assign need_U_imm = inst_auipc | inst_lui;
    assign need_J_imm = inst_jal;

    assign imm = need_I_imm ? {{20{I_imm[11]}}, I_imm} :
           need_S_imm ? {{20{S_imm[11]}}, S_imm} :
           need_B_imm ? {{19{B_imm[12]}}, B_imm} :
           need_U_imm ? {U_imm, 12'b0} :
           need_J_imm ? {{11{J_imm[20]}}, J_imm} :
           32'b0;  // 立即数扩展

    assign we = inst_sub | inst_add | inst_addi | inst_auipc |
           inst_and | inst_lui | inst_jal | inst_jalr ? 1'b1 : 1'b0;
    assign raddr1 = rs1;
    assign raddr2 = rs2;
    
    wire        pmem_en;
    wire [31:0] pmem_addr;
    wire [31:0] pmem_wdata;

    assign pmem_en = inst_sw;
    assign pmem_addr = inst_sw ? (rdata1 + imm) : 32'b0;
    assign pmem_wdata = inst_sw ? rdata2 : 32'b0;

    always_comb begin
        if (pmem_en) begin
            pmem_write(pmem_addr, pmem_wdata);
        end
    end


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
    wire [10:0] alu_op;

    assign alu_src1   = inst_auipc | inst_jal ? pc : rdata1;
    assign alu_src2   = need_I_imm | need_S_imm | need_B_imm | need_U_imm | need_J_imm ? imm : rdata2;
    assign alu_op[0]  = inst_addi;  // ALU operation for ADDI
    assign alu_op[1]  = inst_auipc;  // ALU operation for AUIPC
    assign alu_op[2]  = inst_add;  // ALU operation for ADD
    assign alu_op[3]  = inst_sub;  // ALU operation for SUB
    assign alu_op[4]  = inst_and;  // ALU operation for AND
    assign alu_op[5]  = inst_bne;  // ALU operation for BNE
    assign alu_op[6]  = inst_bge;  // ALU operation for B
    assign alu_op[7]  = inst_bgeu;  // ALU operation for BGEU
    assign alu_op[8]  = inst_blt;  // ALU operation for BLT
    assign alu_op[9]  = inst_bltu;  // ALU operation for
    assign alu_op[10] = inst_beq;  // ALU operation for BEQ


    ysyx_25060173_alu u_alu (
                          .alu_src1(alu_src1),
                          .alu_src2(alu_src2),
                          .alu_op(alu_op),
                          .alu_result(alu_result)
                      );

    assign waddr = rd;
    assign wdata = inst_lui ? imm :
           inst_jalr | inst_jal ? pc + 4 : alu_result;

    assign result = alu_result;

    assign nextpc = inst_jal ? pc + imm :
           inst_jalr ? (rdata1 + imm) & ~1 :
       inst_bge ? alu_result[0] ? pc + imm : pc + 4 :
       inst_blt ? alu_result[0] ? pc + imm : pc + 4 :
       inst_bgeu ? alu_result[0] ? pc + imm : pc + 4 :
       inst_bltu ? alu_result[0] ? pc + imm : pc + 4 :
       inst_beq ? (alu_result == 32'b0) ? pc + imm: pc + 4 :
       inst_bne ? (alu_result != 32'b0) ? pc + imm : pc + 4 :
           pc + 4;

    assign next_pc = nextpc;

    assign now_pc = pc;

endmodule
