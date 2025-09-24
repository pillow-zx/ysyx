module cpu (
    input logic clk,
    input logic rst_n
);
    //================= DPI-C =================//
    import "DPI-C" function void inst_ebreak();
    import "DPI-C" function int get_inst(input int unsigned addr);
    import "DPI-C" function int pmem_read(
        input int unsigned addr,
        input int unsigned size
    );
    import "DPI-C" function void pmem_write(
        input int unsigned addr,
        input int unsigned size,
        input int unsigned data
    );

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
    logic [ 6:0] opcode;
    logic [ 2:0] funct3;
    logic [ 6:0] funct7;
    logic [11:0] funct12;
    logic [31:0] imm;
    logic [ 4:0] rs1;
    logic [ 4:0] rs2;
    logic [ 4:0] rd;

    assign opcode  = inst[6:0];
    assign funct3  = inst[14:12];
    assign funct7  = inst[31:25];
    assign funct12 = inst[31:20];
    assign rs1     = inst[19:15];
    assign rs2     = inst[24:20];
    assign rd      = inst[11:7];


    always_comb begin
        case (opcode)
            // I-type (loads, immediate ALU, JALR)
            7'b0010011, 7'b0000011, 7'b1100111: begin
                // 移位立即数指令需要特殊处理：shift-immediate for RV32I: funct3==001 or 101 (SLLI/SRLI/SRAI)
                if (opcode == 7'b0010011 && (funct3 == 3'b001 || funct3 == 3'b101)) begin
                    // 移位量为 inst[24:20], 无符号扩展
                    imm = {27'b0, inst[24:20]};
                end else begin
                    // 普通I-type: 12位立即数符号扩展
                    imm = {{20{inst[31]}}, inst[31:20]};
                end
            end

            // S-type: store指令
            7'b0100011: imm = {{20{inst[31]}}, inst[31:25], inst[11:7]};
            // B-type: 分支指令
            7'b1100011: imm = {{19{inst[31]}}, inst[31], inst[7], inst[30:25], inst[11:8], 1'b0};
            // U-type: lui, auipc指令
            7'b0110111, 7'b0010111: imm = {inst[31:12], 12'b0};
            // J-type: jal指令
            7'b1101111: imm = {{11{inst[31]}}, inst[31], inst[19:12], inst[20], inst[30:21], 1'b0};
            default: imm = 32'b0;
        endcase
    end

    //================================ 控制信号 ==================================//

    always_comb begin
        // 初始化相关数据
        we        = 1'b0;
        mem_we    = 1'b0;
        mem_re    = 1'b0;
        alu_src1  = 32'b0;
        alu_src2  = 32'b0;
        alu_op    = 4'b0000;
        branch    = 1'b0;
        jump      = 1'b0;
        wb_sel    = 3'b000;
        mem_size  = 3'd4;  // 默认读取一个字节

        // csr相关
        is_csr    = 1'b0;
        csr_we    = 1'b0;
        is_ecall  = 1'b0;
        is_mret   = 1'b0;
        csr_wdata = 32'b0;

        case (opcode)
            /* R-type: 寄存器操作 */
            7'b0110011: begin
                we = 1'b1;
                alu_src1 = rdata1;
                alu_src2 = rdata2;
                wb_sel = 3'b000;  // ALU result
                case ({
                    funct7, funct3
                })
                    10'b0000000_000: alu_op = 4'b0000;  // ADD
                    10'b0100000_000: alu_op = 4'b0001;  // SUB
                    10'b0000000_001: alu_op = 4'b0010;  // SLL
                    10'b0000000_010: alu_op = 4'b0011;  // SLT
                    10'b0000000_011: alu_op = 4'b0100;  // SLTU
                    10'b0000000_100: alu_op = 4'b0101;  // XOR
                    10'b0000000_101: alu_op = 4'b0110;  // SRL
                    10'b0100000_101: alu_op = 4'b0111;  // SRA
                    10'b0000000_110: alu_op = 4'b1000;  // OR
                    10'b0000000_111: alu_op = 4'b1001;  // AND
                    default:         alu_op = 4'b0000;
                endcase
            end

            /* I-type: 寄存器立即数操作 */
            7'b0010011: begin
                we       = 1'b1;
                alu_src1 = rdata1;
                alu_src2 = imm;
                wb_sel   = 3'b000;  // ALU result
                case (funct3)
                    3'b000:  alu_op = 4'b0000;  // ADDI
                    3'b001:  alu_op = 4'b0010;  // SLLI
                    3'b010:  alu_op = 4'b0011;  // SLTI
                    3'b011:  alu_op = 4'b0100;  // SLTIU
                    3'b100:  alu_op = 4'b0101;  // XORI
                    3'b101: begin
                        if (funct7[5]) alu_op = 4'b0111;  // SRAI
                        else alu_op = 4'b0110;  // SRLI
                    end
                    3'b110:  alu_op = 4'b1000;  // ORI
                    3'b111:  alu_op = 4'b1001;  // ANDI
                    default: alu_op = 4'b0000;
                endcase
            end

            /* Load: 从内存加载数据 */
            7'b0000011: begin
                we = 1'b1;
                mem_re = 1'b1;
                // 使用add指令获取地址
                alu_src1 = rdata1;
                alu_src2 = imm;
                alu_op = 4'b0000;  // ADD
                wb_sel = 3'b001;  // Memory data
                case (funct3)
                    3'b000: begin
                        mem_size = 3'd1;
                    end  // LB - 读取字节
                    3'b001: begin
                        mem_size = 3'd2;
                    end  // LH - 读取半字节
                    3'b010: begin
                        mem_size = 3'd4;
                    end  // LW - 加载字
                    3'b100: begin
                        mem_size = 3'd1;
                    end  // LBU - 无符号加载字节
                    3'b101: begin
                        mem_size = 3'd2;
                    end  // LHU - 无符号加载半字节
                    default: begin
                        mem_size = 3'd4;
                    end
                endcase
            end

            /* write: 向内存写入数据 */
            7'b0100011: begin
                mem_we   = 1'b1;
                alu_src1 = rdata1;
                alu_src2 = imm;
                alu_op   = 4'b0000;  // ADD
                case (funct3)
                    3'b000:  mem_size = 3'd1;  // SB - 写入字节
                    3'b001:  mem_size = 3'd2;  // SH - 写入半字节
                    3'b010:  mem_size = 3'd4;  // SW - 写入字
                    default: mem_size = 3'd4;
                endcase
            end


            /* Branch: 条件分支 */
            7'b1100011: begin
                branch = 1'b1;  // 标记为分支指令
            end

            /* JAL */
            7'b1101111: begin
                we = 1'b1;  // 写入返回地址
                jump = 1'b1;  // 标记为跳转指令
                wb_sel = 3'b010;  // 写回pc + 4
            end

            /* JALR */
            7'b1100111: begin
                we = 1'b1;  // 写入返回地址
                jump = 1'b1;  // 标记为跳转指令
                wb_sel = 3'b010;  // 写回PC+4
                alu_src1 = rdata1;  // 基址寄存器
                alu_src2 = imm;  // 偏移量
                alu_op = 4'b0000;  // ADD 计算跳转地址
            end

            /* LUI */
            7'b0110111: begin
                we = 1'b1;  // 使能寄存器写入
                wb_sel = 3'b011;  // 直接写回立即数
            end


            /* AUIPC */
            7'b0010111: begin
                we = 1'b1;  // 使能寄存器写入
                wb_sel = 3'b000;  // 写回ALU结果
                alu_src1 = pc;  // 当前PC
                alu_src2 = imm;  // 立即数
                alu_op = 4'b0000;  // 加法运算
            end

            /* csr inst */
            7'b1110011: begin
                case (funct3)
                    3'b000: begin
                        is_csr = 1'b1;
                        if (inst == 32'h30200073) begin
                            is_mret = 1'b1;
                        end else if (inst == 32'h00000073) begin
                            is_ecall = 1'b1;
                        end else begin
                            $display("Unhandled system instruction: <inst>: %h, at <pc>: %h", inst,
                                     pc);
                        end
                    end
                    3'b001: begin  // csrrw
                        we = 1'b1;
                        wb_sel = 3'b100;
                        csr_we = 1'b1;
                        csr_wdata = rdata1;
                    end
                    3'b010: begin  // csrrs
                        we = 1'b1;
                        csr_we = 1'b1;
                        wb_sel = 3'b100;
                        // 获得csr_wdata
                        csr_wdata = csr_rdata | rdata1;
                    end
                    3'b011: begin  // csrrc
                        we = 1'b1;
                        wb_sel = 3'b100;
                        csr_we = 1'b1;
                        // 获得csr_wdata
                        csr_wdata = csr_rdata & ~rdata1;
                    end
                    default begin

                    end
                endcase
            end

            default: begin
                // $display("undefined inst <inst>: %h at <pc>: %h", inst, pc);
            end
        endcase
    end



    //================================ reg ====================================//
    logic [31:0] rdata1;
    logic [31:0] rdata2;
    logic        we;  // 寄存器写使能
    logic [31:0] wdata;  // 写回数据
    logic [ 2:0] wb_sel;  // 写回数据宽度

    always_comb begin
        case (wb_sel)
            3'b000:  wdata = alu_result;
            3'b001:  wdata = mem_rdata_ext;  // 内存读取数据
            3'b010:  wdata = pc + 4;  // JALR/JAL
            3'b011:  wdata = imm;  // LUI
            3'b100:  wdata = csr_rdata;  // 保存csr相关值
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

    //================================ csr ====================================//
    logic        csr_we;
    logic        is_ecall;
    logic        is_mret;
    logic [31:0] csr_wdata;
    logic [31:0] csr_rdata;

    csr regc (
        .clk(clk),
        .rst_n(rst_n),
        .csr_we(csr_we),
        .addr(funct12),
        .csr_wdata(csr_wdata),
        .is_ecall(is_ecall),
        .is_mret(is_mret),
        .pc_in(pc),
        .csr_rdata(csr_rdata),
        .next_pc(next_pc)
    );

    //================================ mem ====================================//
    logic [31:0] mem_addr;  // 写回内存地址
    logic [31:0] mem_wdata;  // 写回数据
    logic        mem_we;  // 写回数据使能
    logic        mem_re;  // 读数据使能
    logic [31:0] mem_rdata;  // 读取的数据
    logic [31:0] mem_rdata_ext;  // 符号扩展后的内存读数据
    logic [ 2:0] mem_size;  // 内存访问大小


    assign mem_addr  = alu_result;
    assign mem_wdata = rdata2;

    always_comb begin
        if (mem_re) begin
            mem_rdata = pmem_read(mem_addr, {{29{1'b0}}, mem_size});
        end else begin
            mem_rdata = 32'b0;
        end
    end

    // 内存数据符号扩展逻辑
    always_comb begin
        mem_rdata_ext = mem_rdata;
        if (mem_re) begin
            case (funct3)
                3'b000:  // LB - 字节加载，符号扩展
                mem_rdata_ext = {{24{mem_rdata[7]}}, mem_rdata[7:0]};
                3'b001:  // LH - 半字加载，符号扩展
                mem_rdata_ext = {{16{mem_rdata[15]}}, mem_rdata[15:0]};
                3'b010:  // LW - 字加载
                mem_rdata_ext = mem_rdata;
                3'b100:  // LBU - 无符号字节加载
                mem_rdata_ext = {24'b0, mem_rdata[7:0]};
                3'b101:  // LHU - 无符号半字加载
                mem_rdata_ext = {16'b0, mem_rdata[15:0]};
                default: mem_rdata_ext = mem_rdata;
            endcase
        end
    end

    // 内存写入逻辑
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
    logic signed_lt;  // 有符号小于比较结果
    logic unsigned_lt;  // 无符号小于比较结果
    logic equal;  // 相等比较结果
    logic branch_taken;  // 分支是否跳转
    logic branch;

    always_comb begin
        signed_lt    = ($signed(rdata1) < $signed(rdata2));
        unsigned_lt  = (rdata1 < rdata2);
        equal        = (rdata1 == rdata2);

        branch_taken = 1'b0;
        if (branch) begin
            case (funct3)
                3'b000:  branch_taken = equal;  // BEQ
                3'b001:  branch_taken = ~equal;  // BNE
                3'b100:  branch_taken = signed_lt;  // BLT
                3'b101:  branch_taken = ~signed_lt;  // BGE
                3'b110:  branch_taken = unsigned_lt;  // BLTU
                3'b111:  branch_taken = ~unsigned_lt;  // BGEU
                default: branch_taken = 1'b0;
            endcase
        end
    end

    //================================= nextpc ====================================//
    logic jump;
    logic [31:0] next_pc;
    logic is_csr;

    // 计算下一个PC值的逻辑
    always_comb begin
        if (is_csr && (is_ecall || is_mret)) begin
            nextpc = next_pc;
        end else if (jump) begin
            if (opcode == 7'b1100111) begin  // JALR指令
                // JALR: 跳转地址 = (rs1 + imm) & ~1，清除最低位
                nextpc = alu_result & ~32'h1;
            end else begin  // JAL指令
                // JAL: 跳转地址 = PC + imm
                nextpc = pc + imm;
            end
        end else if (branch_taken) begin
            nextpc = pc + imm;
        end else begin
            nextpc = pc + 4;
        end
    end
endmodule
