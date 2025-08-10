module cpu(
    input logic clk,
    input logic rst_n
);
    //================= DPI-C =================//
    import "DPI-C" function void inst_ebreak();
    import "DPI-C" function int get_inst(input int unsigned addr);
    import "DPI-C" function int pmem_read(input int unsigned addr, input int unsigned size);
    import "DPI-C" function void pmem_write(input int unsigned addr, input int unsigned size, input int unsigned data);

    //==================== pc ===================================//
    logic [31:0] pc;
    logic [31:0] nextpc;
    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            pc <= 32'h8000_0000;
            $display("CPU reset --- pc: %h", pc);
        end else begin
            // $display("CPU running --- pc: %h", pc);
            pc <= nextpc;
        end
    end

    //==================== 取指 ===================================//
    logic [31:0] inst;
    always_comb begin
        inst = get_inst(pc);
    end

    //==================== ebreak ===================================//
    logic ebreak;
    always_comb begin
        ebreak = ((inst == 32'h00100073) || (inst == 32'h0000006f));
    end
    always_ff @(posedge clk) begin
        // $display("CPU ebreak --- inst  <inst>: %h", inst);
        // $display("CPU ebreak --- ebreak: %b", ebreak);
        if (ebreak) begin
            inst_ebreak();
        end
    end

    //=================== 解码 ===================================//
    logic [6:0] opcode;
    logic [2:0] funct3;
    logic [6:0] funct7;
    logic [31:0] imm;
    logic [4:0] rs1;
    logic [4:0] rs2;
    logic [4:0] rd;

    assign opcode = inst[6:0];
    assign funct3 = inst[14:12];
    assign funct7 = inst[31:25];
    assign rs1    = inst[19:15];
    assign rs2    = inst[24:20];
    assign rd     = inst[11:7];


    always_comb begin
        case (opcode)
            // I-type (loads, immediate ALU, JALR)
            7'b0010011,
            7'b0000011,
            7'b1100111: begin
                // shift-immediate for RV32I: funct3==001 or 101 (SLLI/SRLI/SRAI)
                if (opcode == 7'b0010011 && (funct3 == 3'b001 || funct3 == 3'b101)) begin
                    // shamt is inst[24:20], zero-extend
                    imm = {27'b0, inst[24:20]};
                end else begin
                    imm = {{20{inst[31]}}, inst[31:20]};
                end
            end

            // S-type
            7'b0100011:
                imm = {{20{inst[31]}}, inst[31:25], inst[11:7]};
            // B-type
            7'b1100011:
                imm = {{19{inst[31]}}, inst[31], inst[7], inst[30:25], inst[11:8], 1'b0};
            // U-type
            7'b0110111,
            7'b0010111:
                imm = {inst[31:12], 12'b0};
            // J-type
            7'b1101111:
                imm = {{11{inst[31]}}, inst[31], inst[19:12], inst[20], inst[30:21], 1'b0};
            default: imm = 32'b0;
        endcase
    end

    //================================ 控制信号 ==================================//

    always_comb begin
        we       = 1'b0;
        mem_we   = 1'b0;
        mem_re   = 1'b0;
        alu_src1 = 32'b0;
        alu_src2 = 32'b0;
        alu_op   = 4'b0000;
        branch   = 1'b0;
        jump     = 1'b0;
        wb_sel   = 2'b00;
        mem_size = 3'd4;

        case (opcode)
            /* R-type */
            7'b0110011: begin
                we  = 1'b1;
                alu_src1 = rdata1;
                alu_src2 = rdata2;
                wb_sel = 2'b00; // ALU result
                case ({funct7, funct3})
                    10'b0000000_000: alu_op = 4'b0000; // ADD
                    10'b0100000_000: alu_op = 4'b0001; // SUB
                    10'b0000000_001: alu_op = 4'b0010; // SLL
                    10'b0000000_010: alu_op = 4'b0011; // SLT
                    10'b0000000_011: alu_op = 4'b0100; // SLTU
                    10'b0000000_100: alu_op = 4'b0101; // XOR
                    10'b0000000_101: alu_op = 4'b0110; // SRL
                    10'b0100000_101: alu_op = 4'b0111; // SRA
                    10'b0000000_110: alu_op = 4'b1000; // OR
                    10'b0000000_111: alu_op = 4'b1001; // AND
                    default:         alu_op = 4'b0000;
                endcase
            end

            /* I-type */
            7'b0010011: begin
                we       = 1'b1; 
                alu_src1 = rdata1;
                alu_src2 = imm;
                wb_sel   = 2'b00; // ALU result
                case (funct3)
                    3'b000: alu_op = 4'b0000; // ADDI
                    3'b001: alu_op = 4'b0010; // SLLI
                    3'b010: alu_op = 4'b0011; // SLTI
                    3'b011: alu_op = 4'b0100; // SLTIU
                    3'b100: alu_op = 4'b0101; // XORI
                    3'b101: begin
                        if (funct7[5]) alu_op = 4'b0111; // SRAI
                        else           alu_op = 4'b0110; // SRLI
                    end
                    3'b110: alu_op = 4'b1000; // ORI
                    3'b111: alu_op = 4'b1001; // ANDI
                endcase
            end

            /* Load */
            7'b0000011: begin
                we = 1'b1; 
                mem_re = 1'b1;
                alu_src1 = rdata1;
                alu_src2 = imm;
                alu_op = 4'b0000; // ADD
                wb_sel = 2'b01; // Memory data
                case (funct3)
                    3'b000: begin 
                        mem_size = 3'd1; 
                    end // LB
                    3'b001: begin 
                        mem_size = 3'd2; 
                    end // LH
                    3'b010: begin 
                        mem_size = 3'd4; 
                    end
                    3'b100: begin
                        mem_size = 3'd1;
                    end
                    3'b101: begin
                        mem_size = 3'd2;
                    end
                    default: begin
                        mem_size = 3'd4;
                    end
                endcase
            end

            /* write */
            7'b0100011: begin
                mem_we = 1'b1;
                alu_src1 = rdata1;
                alu_src2 = imm;
                alu_op = 4'b0000; // ADD
                case (funct3)
                    3'b000: mem_size = 3'd1; // SB
                    3'b001: mem_size = 3'd2; // SH
                    3'b010: mem_size = 3'd4; // SW
                    default: mem_size = 3'd4;
                endcase
            end


            /* Branch */
            7'b1100011: begin
                branch = 1'b1;
                alu_src1 = rdata1;
                alu_src2 = rdata2;
                case (funct3)
                    3'b000: alu_op = 4'b0000; // BEQ
                    3'b001: alu_op = 4'b0001; // BNE
                    3'b100: alu_op = 4'b0011; // BLT
                    3'b101: alu_op = 4'b0100; // BGE
                    3'b110: alu_op = 4'b0101; // BLTU
                    3'b111: alu_op = 4'b0110; // BGEU
                    default: alu_op = 4'b0000; // 默认操作
                endcase
            end

            /* JAL */
            7'b1101111: begin
                we = 1'b1;
                jump = 1'b1;
                wb_sel = 2'b10;
            end

            /* JALR */
            7'b1100111: begin
                we = 1'b1;
                jump = 1'b1;
                wb_sel = 2'b10;
                alu_src1 = rdata1;
                alu_src2 = imm;
                alu_op = 4'b0000; // ADD
            end

            /* LUI */
            7'b0110111: begin
                we = 1'b1;
                wb_sel = 2'b11;
            end


            /* AUIPC */
            7'b0010111: begin
                we = 1'b1;
                wb_sel = 2'b00;
                alu_src1 = pc;
                alu_src2 = imm;
                alu_op = 4'b0000; // ADD
            end

            default: begin

            end
        endcase
    end



    //================================ reg ====================================//
    logic [31:0] rdata1;
    logic [31:0] rdata2;
    logic        we;
    logic [31:0] wdata;
    logic [1:0] wb_sel;

    always_comb begin
        case(wb_sel) 
            2'b00: wdata = alu_result;
            2'b01: wdata = mem_rdata_ext;
            2'b10: wdata = pc + 4; // JALR
            2'b11: wdata = imm;
            default: wdata = 32'b0;
        endcase
    end

    regfile regf (
        .clk(clk),
        .raddr1(rs1),
        .rdata1(rdata1),
        .raddr2(rs2),
        .rdata2(rdata2),
        .we(we),
        .waddr(rd),
        .wdata(wdata)
    );

    //================================ mem ====================================//
    logic [31:0] mem_addr;
    logic [31:0] mem_wdata;
    logic        mem_we;
    logic        mem_re;
    logic [31:0] mem_rdata;
    logic [31:0] mem_rdata_ext;
    logic [ 2:0] mem_size;


    assign mem_addr = alu_result;
    assign mem_wdata = rdata2;

    always_comb begin
        if (mem_re) begin
            mem_rdata = pmem_read(mem_addr, {{29{1'b0}}, mem_size});
        end else begin
            mem_rdata = 32'b0;
        end
    end

    always_comb begin
        mem_rdata_ext = mem_rdata;
        if (mem_re) begin
            case (funct3)
                3'b000: mem_rdata_ext = {{24{mem_rdata[7]}},  mem_rdata[7:0]};   // LB (sign extend byte)
                3'b001: mem_rdata_ext = {{16{mem_rdata[15]}}, mem_rdata[15:0]};  // LH (sign extend half)
                3'b010: mem_rdata_ext = mem_rdata;                              // LW
                3'b100: mem_rdata_ext = {24'b0, mem_rdata[7:0]};                 // LBU
                3'b101: mem_rdata_ext = {16'b0, mem_rdata[15:0]};                // LHU
                default: mem_rdata_ext = mem_rdata;
            endcase
        end
    end

    always_ff @(posedge clk) begin
        if (mem_we) begin
            pmem_write(mem_addr, {{29{1'b0}}, mem_size}, mem_wdata);
        end
    end

    //================================ alu ======================================//
    logic [31:0] alu_src1;
    logic [31:0] alu_src2;
    logic [ 3:0] alu_op;
    logic [31:0] alu_result;
    logic        alu_zero;
    logic        alu_negative;

    alu alu_inst (
        .alu_src1(alu_src1),
        .alu_src2(alu_src2),
        .alu_op(alu_op),
        .alu_result(alu_result),
        .alu_zero(alu_zero),
        .alu_negative(alu_negative)
    );


    //================================= branch ==================================//
    logic signed_lt;
    logic unsigned_lt;
    logic equal;
    logic        branch;
    logic        branch_taken;

    always_comb begin
        signed_lt   = ($signed(rdata1) < $signed(rdata2));
        unsigned_lt = (rdata1 < rdata2);
        equal       = (rdata1 == rdata2);

        branch_taken = 1'b0;
        if (branch) begin
            case (funct3)
                3'b000: branch_taken = equal;           // BEQ
                3'b001: branch_taken = ~equal;          // BNE
                3'b100: branch_taken = signed_lt;       // BLT
                3'b101: branch_taken = ~signed_lt;      // BGE
                3'b110: branch_taken = unsigned_lt;     // BLTU
                3'b111: branch_taken = ~unsigned_lt;    // BGEU
                default: branch_taken = 1'b0;
            endcase
        end
    end

    //================================= nextpc ====================================//
    logic       jump;

    always_comb begin
        if (jump) begin
            if (opcode == 7'b1100111) begin
                nextpc = alu_result & ~32'h1;
            end else begin
                nextpc = pc + imm;
            end
        end else if (branch_taken) begin
            nextpc = pc + imm;
        end else begin
            nextpc = pc + 4;
        end
    end



endmodule
