/* verilator lint_off DECLFILENAME*/
// ALU模块：实现32位RISC-V算术逻辑运算单元
// 支持加减运算、逻辑运算、比较运算等功能
module ysyx_25060173_alu (
        input  wire [31:0] alu_src1,
        input  wire [31:0] alu_src2,
        input  wire [25:0] alu_op,
        output wire [31:0] alu_result
    );


    wire op_addi;
    wire op_auipc;
    wire op_add;
    wire op_sub;
    wire op_and;
    wire op_beq;    // 相等比较
    wire op_bne;    // 不等比较
    wire op_bge;    // 有符号大于等于比较
    wire op_bgeu;   // 无符号大于等于比较
    wire op_blt;    // 有符号小于比较
    wire op_bltu;   // 无符号小于比较
    wire op_sltiu;  // 无符号小于立即数设置
    wire op_slli;   // 逻辑左移立即数
    wire op_sltu;   // 无符号小于比较（SLTU）
    wire op_xor;    // 逻辑异或运算
    wire op_or;     // 逻辑或运算（新增）
    wire op_slt;    // 有符号小于比较（SLT）
    wire op_sra;    // 算术右移（SRA）
    wire op_srl;    // 逻辑右移（SRL）
    wire op_sll;    // 逻辑左移（SLL）
    wire op_andi;   // 逻辑与立即数（ANDI）
    wire op_srli;   // 逻辑右移立即数（SRLI）
    wire op_srai;   // 算术右移立即数（SRAI）
    wire op_xori;   // 逻辑异或立即数（XORI）
    wire op_ori;    // 逻辑或立即数（ORI）
    wire op_slti;   // 有符号小于立即数（SLTI）


    assign op_addi  = alu_op[0];
    assign op_auipc = alu_op[1];
    assign op_add   = alu_op[2];
    assign op_sub   = alu_op[3];
    assign op_and   = alu_op[4];
    assign op_bne   = alu_op[5];    // 位5：不等比较
    assign op_bge   = alu_op[6];    // 位6：有符号大于等于比较
    assign op_bgeu  = alu_op[7];    // 位7：无符号大于等于比较
    assign op_blt   = alu_op[8];    // 位8：有符号小于比较
    assign op_bltu  = alu_op[9];    // 位9：无符号小于比较
    assign op_beq   = alu_op[10];   // 位10：相等比较
    assign op_sltiu = alu_op[11];   // 位11：无符号小于立即数设置
    assign op_slli  = alu_op[12];   // 位12：逻辑左移立即数
    assign op_sltu  = alu_op[13];   // 位13：无符号小于比较（SLTU）
    assign op_xor   = alu_op[14];   // 位14：逻辑异或运算
    assign op_or    = alu_op[15];   // 位15：逻辑或运算（新增）
    assign op_slt   = alu_op[16];   // 位16：有符号小于比较（SLT）
    assign op_sra   = alu_op[17];   // 位17：算术右移（SRA）
    assign op_srl   = alu_op[18];   // 位18：逻辑右移（SRL）
    assign op_sll   = alu_op[19];   // 位19：逻辑左移（SLL）
    assign op_andi  = alu_op[20];   // 位20：逻辑与立即数（ANDI）
    assign op_srli  = alu_op[21];   // 位21：逻辑右移立即数（SRLI）
    assign op_srai  = alu_op[22];   // 位22：算术右移立即数（SRAI）
    assign op_xori  = alu_op[23];   // 位23：逻辑异或立即数（XORI）
    assign op_ori   = alu_op[24];   // 位24：逻辑或立即数（ORI）
    assign op_slti  = alu_op[25];   // 位25：有符号小于立即数（SLTI）


    wire [31:0] add_sub_result;              // 加减运算结果
    wire [31:0] signed_cmp_result;           // 有符号比较结果
    wire [31:0] unsigned_cmp_result;         // 无符号比较/相等比较结果
    wire [31:0] l_mv_result;                  //  移位结果

    wire [31:0] adder_a;
    wire [31:0] adder_b;
    wire        adder_cin;
    wire [31:0] adder_result;
    wire        adder_cout;

    assign adder_a = alu_src1;
    assign adder_b = op_sub | op_beq | op_bne | op_bgeu | op_bltu | op_sltiu | op_slt | 
                     op_sltu | op_slti | op_bge | op_blt ? ~alu_src2 : alu_src2;
    assign adder_cin = op_sub | op_beq | op_bne | op_bgeu | op_bltu | op_sltiu | op_slt |
                       op_sltu | op_slti | op_bge | op_blt ? 1'b1 : 1'b0;

    assign {adder_cout, adder_result} = adder_a + adder_b + {{31{1'b0}}, adder_cin};

    // 有符号比较结果计算（用于bge和blt指令）
    // 有符号比较逻辑：
    // 1. 如果src1为负数且src2为正数，则src1 < src2，结果为1（对于blt）
    // 2. 如果符号相同，则比较减法结果的符号位
    assign signed_cmp_result[31:1] = 31'b0;  // 高31位清零，只保留比较结果位

    assign signed_cmp_result[0] = (alu_src1[31] & ~alu_src2[31]) |
           (~(alu_src1[31] ^ alu_src2[31]) & adder_result[31]);

    // 无符号比较/相等比较结果计算（用于beq、bne、bgeu、bltu指令）
    // 无符号比较逻辑：通过进位标志判断
    // 当alu_src1 >= alu_src2时，减法不会产生借位，即进位为1
    // ~adder_cout表示：无进位时结果为1（表示src1 < src2）
    assign unsigned_cmp_result[31:1] = 31'b0;  // 高31位清零
    assign unsigned_cmp_result[0] = ~adder_cout;

    // 相等比较结果计算（用于beq和bne指令）
    // 相等比较逻辑：当两个操作数相等时，减法结果为0
    wire [31:0] equal_cmp_result;
    assign equal_cmp_result[31:1] = 31'b0;  // 高31位清零
    assign equal_cmp_result[0] = (adder_result == 32'b0);  // 当减法结果为0时，表示相等

    // 各种运算结果的生成
    assign add_sub_result = adder_result;               // 加减运算直接使用加法器结果

    wire [4:0] shift_amount; // 移位操作的移位量
    assign shift_amount = alu_src2[4:0]; // 取低5位

    wire [31:0] sra_result;
    assign sra_result = $signed(alu_src1) >>> shift_amount;

    assign l_mv_result = op_sra | op_srai ? sra_result : // 算术右移
                       op_srl | op_srli ? (alu_src1 >> shift_amount) : // 逻辑右移
                       op_sll | op_slli ? (alu_src1 << shift_amount) : // 逻辑左移
                       op_xor | op_xori ? alu_src1 ^ alu_src2 :    // 逻辑异或运算
                       op_or | op_ori ? alu_src1 | alu_src2 :           // 逻辑或运算
                       op_and | op_andi ? alu_src1 & alu_src2 :    // 逻辑与运算
                       32'b0; // 其他移位操作结果清零


    // 最终结果选择：根据操作类型选择相应的运算结果
    assign alu_result = op_bge | op_blt | op_slt | op_slti ? signed_cmp_result :
                        op_bgeu | op_bltu | op_sltiu | op_sltu ? unsigned_cmp_result :
                        op_beq | op_bne ? equal_cmp_result :
                        op_sra | op_srl | op_sll | op_slli | op_xor |
                        op_or | op_and | op_andi | op_srli | op_srai |
                        op_xori | op_ori ? l_mv_result :     // 算术/逻辑移位运算
                        add_sub_result;                                                    // 默认：加减运算结果

endmodule
