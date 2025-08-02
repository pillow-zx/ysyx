/* verilator lint_off DECLFILENAME*/
// ALU模块：实现32位RISC-V算术逻辑运算单元
// 支持加减运算、逻辑运算、比较运算等功能
module ysyx_25060173_alu (
        input  wire [31:0] alu_src1,
        input  wire [31:0] alu_src2,
        input  wire [12:0] alu_op,
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


    wire [31:0] add_sub_result;              // 加减运算结果
    wire [31:0] and_result;                  // 逻辑与运算结果
    wire [31:0] signed_cmp_result;           // 有符号比较结果
    wire [31:0] unsigned_cmp_result;         // 无符号比较/相等比较结果
    wire [31:0] logic_result;                // 逻辑运算结果


    wire [31:0] adder_a;
    wire [31:0] adder_b;
    wire        adder_cin;
    wire [31:0] adder_result;
    wire        adder_cout;

    assign adder_a = alu_src1;
    assign adder_b = op_sub | op_beq | op_bne | op_bgeu | op_bltu | op_sltiu ? ~alu_src2 : alu_src2;
    assign adder_cin = op_sub | op_beq | op_bne | op_bgeu | op_bltu | op_sltiu ? 1'b1 : 1'b0;

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

    // 各种运算结果的生成
    assign add_sub_result = adder_result;        // 加减运算直接使用加法器结果
    assign and_result = alu_src1 & alu_src2;     // 逻辑与运算

    assign logic_result = alu_src1 << alu_src2[5:0];

    // 最终结果选择：根据操作类型选择相应的运算结果
    assign alu_result = op_and ? and_result :                                 // 逻辑与运算
           op_bge | op_blt ? signed_cmp_result :                              // 有符号比较运算
           op_bgeu | op_bltu | op_beq | op_sltiu ? unsigned_cmp_result :      // 无符号比较运算
           op_slli ? logic_result :                                           // 逻辑运算立即数
           add_sub_result;                                                    // 默认：加减运算结果

endmodule
