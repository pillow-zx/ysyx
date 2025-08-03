import "DPI-C" context function void ebreak_handler();
import "DPI-C" context function int pmem_read(input int addr);
import "DPI-C" context function void pmem_write(input int addr, input int data);
import "DPI-C" context function int unsigned inst_read(input int unsigned addr);
/* verilator lint_off DECLFILENAME*/
module ysyx_25060173_core (
        input wire clk,
        input wire reset,
        output wire [31:0] result,
        output wire [31:0] next_pc,
        output reg [31:0] now_pc
    );
    wire valid;

    assign valid = ~reset;

    reg  [31:0] pc;
    wire [31:0] nextpc;

    always @(posedge clk) begin
        if (reset) begin
            pc <= 32'h80000000;  // 初始化 PC
        end
        else begin
            pc <= nextpc;
        end
    end

    wire [31:0] inst;
    // 修改指令读取时序：在时钟高电平时读取指令，这样与寄存器写入时序对齐
    assign inst = ~reset && clk ? inst_read(pc) : 32'b0;  // 从内存中读取指令


    wire [4:0] op_11_7;
    wire [4:0] op_19_15;
    wire [4:0] op_24_20;
    wire [4:0] shamt;    // 移位量，只需要5位
    wire [4:0] rd;
    wire [4:0] rs1;
    wire [4:0] rs2;

    assign op_11_7  = inst[11:7 ];  // 11-7
    assign op_19_15 = inst[19:15];  // 19-15
    assign op_24_20 = inst[24:20];  // 24-20
    assign shamt    = inst[24:20];  // 移位量：24-20（5位）
    assign rd       = inst[11:7 ];  // 11-7
    assign rs1      = inst[19:15];  // 19-15
    assign rs2      = inst[24:20];  // 24-20

    wire [11:0] I_imm;
    wire [11:0] S_imm;
    wire [12:0] B_imm;
    wire [19:0] U_imm;
    wire [20:0] J_imm;

    assign I_imm = inst[31:20];  // 31-20
    assign S_imm = {inst[31:25], inst[11:7]};  // 31-25, 11-7;
    assign B_imm = {inst[31], inst[7], inst[30:25], inst[11:8], 1'b0};
    assign U_imm = inst[31:12];  // 31-12
    assign J_imm = {inst[31], inst[19:12], inst[20], inst[30:21], 1'b0};  // 31, 19-12, 20, 30-21, 0}

    // R 型指令
    wire inst_sub;
    wire inst_add;
    wire inst_and;
    wire inst_sltu;
    wire inst_xor;
    wire inst_or;
    wire inst_slt;
    wire inst_sra;
    wire inst_srl;
    wire inst_sll;
    // I 型指令
    wire inst_jalr;
    wire inst_ebreak;
    wire inst_xori;
    wire inst_addi;
    wire inst_srli;
    wire inst_lw;
    wire inst_lui;
    wire inst_andi;
    wire inst_ori;
    wire inst_slti;
    wire inst_slli;
    wire inst_sltiu;
    wire inst_srai;
    wire inst_lbu;
    wire inst_lb;
    wire inst_lhu;
    wire inst_lh;
    // S 型指令
    wire inst_sw;
    wire inst_sb;
    wire inst_sh;
    // B 型指令
    wire inst_beq;
    wire inst_bge;
    wire inst_bgeu;
    wire inst_blt;
    wire inst_bltu;
    wire inst_bne;
    // U 型指令
    wire inst_auipc;
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
                                          .inst_srli(inst_srli),
                                          .inst_lh(inst_lh),
                                          .inst_bne(inst_bne),
                                          .inst_add(inst_add),
                                          .inst_and(inst_and),
                                          .inst_srai(inst_srai),
                                          .inst_sh(inst_sh),
                                          .inst_lb(inst_lb),
                                          .inst_addi(inst_addi),
                                          .inst_xori(inst_xori),
                                          .inst_xor(inst_xor),
                                          .inst_auipc(inst_auipc),
                                          .inst_srl(inst_srl),
                                          .inst_ebreak(inst_ebreak),
                                          .inst_lhu(inst_lhu),
                                          .inst_lbu(inst_lbu),
                                          .inst_or(inst_or),
                                          .inst_ori(inst_ori),
                                          .inst_slti(inst_slti),
                                          .inst_slli(inst_slli),
                                          .inst_sb(inst_sb),
                                          .inst_andi(inst_andi),
                                          .inst_sll(inst_sll),
                                          .inst_lui(inst_lui),
                                          .inst_jal(inst_jal),
                                          .inst_sltiu(inst_sltiu),
                                          .inst_slt(inst_slt),
                                          .inst_sra(inst_sra),
                                          .inst_sltu(inst_sltu),
                                          .inst_lw(inst_lw),
                                          .inst_jalr(inst_jalr),
                                          .inst_sw(inst_sw)
                                      );
    // 修改ebreak处理时序：在时钟高电平时处理，与指令读取时序同步
    always @(*) begin
        if (~reset && clk && inst_ebreak) begin
            ebreak_handler();
        end
    end

    wire need_I_imm;
    wire need_S_imm;
    wire need_B_imm;
    wire need_U_imm;
    wire need_J_imm;

    wire [31:0] imm;
    wire [ 4:0] raddr1;
    wire [31:0] rdata1;
    wire [ 4:0] raddr2;
    wire [31:0] rdata2;
    wire [ 4:0] waddr;
    wire [31:0] wdata;
    wire        we;

    assign need_I_imm = inst_addi | inst_ebreak | inst_jalr | inst_lw | inst_sltiu |
                        inst_lbu | inst_andi | inst_lb | inst_lhu | inst_xori | inst_lh |
                        inst_ori | inst_slti;  // 修复：移位立即数指令单独处理，不使用扩展立即数
    assign need_S_imm = inst_sw | inst_sb | inst_sh;  // S 型指令需要立即数
    assign need_B_imm = inst_beq | inst_bge | inst_bgeu | inst_blt | inst_bltu | inst_bne;
    assign need_U_imm = inst_auipc | inst_lui;
    assign need_J_imm = inst_jal;

    assign imm = need_I_imm ? {{20{I_imm[11]}}, I_imm} :
           need_S_imm ? {{20{S_imm[11]}}, S_imm} :
           need_B_imm ? {{19{B_imm[12]}}, B_imm} :
           need_U_imm ? {U_imm, 12'b0} :
           need_J_imm ? {{11{J_imm[20]}}, J_imm} :
           32'b0;  // 立即数扩展

    // 现在指令在时钟高电平时有效，写使能信号可以直接使用组合逻辑
    assign we = inst_sub | inst_add | inst_addi | inst_auipc |
           inst_and | inst_lui | inst_jal | inst_jalr | inst_lw |
           inst_sltiu | inst_slli | inst_sltu | inst_xor | inst_or |
           inst_slt | inst_sra |  inst_srl | inst_sll | inst_andi | 
           inst_lbu | inst_srli | inst_srai | inst_lb | inst_lhu |
           inst_xori | inst_lh | inst_ori | inst_slti ? 1'b1 : 1'b0;

    assign raddr1 = rs1;
    assign raddr2 = rs2;

    wire        pmem_en;
    wire        pmem_re;
    wire [31:0] pmem_addr;
    wire [31:0] pmem_wdata;
    reg  [31:0] pmem_rdata;

    assign pmem_en = inst_sw | inst_sb | inst_sh;  // 只有存储指令需要使能内存写操作
    assign pmem_re = inst_lw | inst_lbu | inst_lb | inst_lhu | inst_lh;
    assign pmem_addr = inst_lw | inst_sw | inst_lbu | inst_lb | inst_lhu | inst_lh | inst_sb | inst_sh ? (rdata1 + imm) : 32'b0;
    assign pmem_wdata = inst_sw ? rdata2 :
                        inst_sb ? {24'b0, rdata2[7:0]} :
                        inst_sh ? {16'b0, rdata2[15:0]} :
                        32'b0;

    always_comb begin
        if (pmem_en) begin
            pmem_write(pmem_addr, pmem_wdata);
            pmem_rdata = 32'b0;  // 写操作不返回数据
        end
        else if (pmem_re) begin
            pmem_rdata = pmem_read(pmem_addr);
        end
        else begin
            pmem_rdata = 32'b0;  // 如果不是读操作，返回0
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
    wire [25:0] alu_op;

    assign alu_src1   = inst_auipc | inst_jal ? pc : rdata1;
    assign alu_src2   = inst_slli | inst_srli | inst_srai ? {27'b0, shamt} :  // 移位立即数指令使用移位量
                        need_I_imm | need_S_imm | need_U_imm | need_J_imm ? imm :  // 其他立即数指令使用扩展后的立即数
                        rdata2;  // 寄存器-寄存器指令使用rs2
    assign alu_op[0]  = inst_addi;  // ALU operation for ADDI
    assign alu_op[1]  = inst_auipc;  // ALU operation for AUIPC
    assign alu_op[2]  = inst_add;  // ALU operation for ADD
    assign alu_op[3]  = inst_sub;  // ALU operation for SUB
    assign alu_op[4]  = inst_and;  // ALU operation for AND
    assign alu_op[5]  = inst_bne;  // ALU operation for BNE
    assign alu_op[6]  = inst_bge;  // ALU operation for B
    assign alu_op[7]  = inst_bgeu;  // ALU operation for BGEU
    assign alu_op[8]  = inst_blt;  // ALU operation for BLT
    assign alu_op[9]  = inst_bltu;  // ALU operation for BLTU
    assign alu_op[10] = inst_beq;  // ALU operation for BEQ
    assign alu_op[11] = inst_sltiu;  // ALU operation for SLTIU
    assign alu_op[12] = inst_slli;  // ALU operation for SLLI
    assign alu_op[13] = inst_sltu;  // ALU operation for SLTU
    assign alu_op[14] = inst_xor;  // ALU operation for XOR
    assign alu_op[15] = inst_or;  // ALU operation for OR
    assign alu_op[16] = inst_slt;  // ALU operation for SLT
    assign alu_op[17] = inst_sra;  // ALU operation for SRA
    assign alu_op[18] = inst_srl;  // ALU operation for SRL
    assign alu_op[19] = inst_sll;  // ALU operation for SLL
    assign alu_op[20] = inst_andi;  // ALU operation for ANDI
    assign alu_op[21] = inst_srli;  // ALU operation for SRLI
    assign alu_op[22] = inst_srai;  // ALU operation for SRAI
    assign alu_op[23] = inst_xori;  // ALU operation for XORI
    assign alu_op[24] = inst_ori;   // ALU operation for ORI
    assign alu_op[25] = inst_slti;  // ALU operation for SLTI

    ysyx_25060173_alu u_alu (
                          .alu_src1(alu_src1),
                          .alu_src2(alu_src2),
                          .alu_op(alu_op),
                          .alu_result(alu_result)
                      );

    assign waddr = rd;
    assign wdata = inst_lui ? imm :
           inst_lw ? pmem_rdata :
           inst_jalr | inst_jal ? pc + 4 :
           inst_sltiu ? (alu_result[0] ? 32'd1 : 32'd0) :
           inst_sltu ? (alu_result[0] ? 32'd1 : 32'd0) :
           inst_slt | inst_slti ? (alu_result[0] ? 32'd1 : 32'd0) :
           inst_lbu ? {24'b0, pmem_rdata[7:0]} :  // LBU指令处理
           inst_lb ? {{24{pmem_rdata[7]}} , pmem_rdata[7:0]} :  // LB指令处理
           inst_lhu ? {16'b0, pmem_rdata[15:0]} :  // LHU指令处理
           inst_lh ? {{16{pmem_rdata[15]}} , pmem_rdata[15:0]} :  // LH指令处理
           alu_result;

    assign result = alu_result;

    assign nextpc = inst_jal ? pc + imm :
           inst_jalr ? (rdata1 + imm) & ~1 :
           inst_bge ? alu_result[0] ? pc + 4 : pc + imm :     // BGE: rs1 >= rs2时跳转 (result[0]=0表示rs1>=rs2)
           inst_blt ? alu_result[0] ? pc + imm : pc + 4 :     // BLT: rs1 < rs2时跳转 (result[0]=1表示rs1<rs2)
           inst_bgeu ? alu_result[0] ? pc + 4 : pc + imm :    // BGEU: rs1 >= rs2时跳转 (result[0]=0表示rs1>=rs2)
           inst_bltu ? alu_result[0] ? pc + imm : pc + 4 :    // BLTU: rs1 < rs2时跳转 (result[0]=1表示rs1<rs2)
           inst_beq ? alu_result[0] ? pc + imm : pc + 4 :     // BEQ: rs1 = rs2时跳转 (result[0]=1表示相等)
           inst_bne ? alu_result[0] ? pc + 4 : pc + imm :     // BNE: rs1 ≠ rs2时跳转 (result[0]=0表示不等)
           pc + 4;

    assign next_pc = nextpc;

    assign now_pc = pc;

endmodule
