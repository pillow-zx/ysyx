// 触发器模板
module Reg #(WIDTH = 1, RESET_VAL = 0) (
        input clk,
        input rst,
        input [WIDTH - 1 : 0] din,
        output reg [WIDTH - 1 : 0] dout,
        input wen
    );

    always @(posedge clk) begin
        if (rst) begin
            dout <= RESET_VAL;
        end
        else if (wen) begin
            dout <= din;
        end
    end


endmodule

// 选择器模板的内部实现
module MuxKeyInternal #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1, HAS_DEFAULT = 0) (
        output [DATA_LEN - 1 : 0] out,
        input [NR_KEY - 1 : 0] key,
        input [DATA_LEN - 1 : 0] default_out,
        input [NR_KEY * DATA_LEN - 1 : 0] lut
    );

    localparam PAIR_LEN = KEY_LEN + DATA_LEN;
    wire [PAIR_LEN - 1 : 0] pair_list [NR_KEY - 1 : 0];
    wire [KEY_LEN - 1 : 0] key_list [NR_KEY - 1 : 0];
    wire [DATA_LEN - 1 : 0] data_list [NR_KEY - 1 : 0];

    genvar n;
    generate
        for (n = 0; n < NR_KEY; n = n + 1) begin
            assign pair_list[n] = lut[PAIR_LEN * (n + 1) - 1 : PAIR_LEN * n];
            assign data_list[n] = pair_list[n][DATA_LEN - 1 : 0];
            assign key_list[n] = pair_list[n][PAIR_LEN - 1 : DATA_LEN];
        end
    endgenerate

    reg [DATA_LEN - 1 : 0] lut_out;
    reg hit;
    integer i;
    always @(*) begin
        lut_out = 0;
        hit = 0;
        for (i = 0; i < NR_KEY; i = i + 1) begin
            lut_out = lut_out |({DATA_LEN{key == key_list[i]}} & data_list[i]);
            hit = hit | (key == key_list[i]);
        end
        if (!HAS_DEFAULT) begin
            out = lut_out;
        end
        else begin
            out = hit ? lut_out : default_out;
        end
    end
endmodule

// 不带默认值的选择器模板
module MuxKey #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1)(
        output [DATA_LEN - 1 : 0] out,
        input [KEY_LEN - 1 : 0] key,
        input [DATA_LEN - 1 : 0] default_out,
        input [NR_KEY * (KEY_LEN + DATA_LEN) - 1 : 0] lut
    );
    MuxKeyInternal #(
                       .NR_KEY(NR_KEY),
                       .KEY_LEN(KEY_LEN),
                       .DATA_LEN(DATA_LEN),
                       .HAS_DEFAULT(0)
                   ) mux_key_internal (
                       .out(out),
                       .key(key),
                       .default_out({DATA_LEN{1'b0}}), // Default output is zero when HAS_DEFAULT is 0
                       .lut(lut)
                   );

endmodule

// 带默认值的选择器模板
module MuxKeyWithDefault #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1)(
        output [DATA_LEN - 1 : 0] out,
        input [KEY_LEN - 1 : 0] key,
        input [DATA_LEN - 1 : 0] default_out,
        input [NR_KEY * (KEY_LEN + DATA_LEN) - 1 : 0] lut
    );
    MuxKeyInternal #(
                       .NR_KEY(NR_KEY),
                       .KEY_LEN(KEY_LEN),
                       .DATA_LEN(DATA_LEN),
                       .HAS_DEFAULT(1)
                   ) mux_key_internal (
                       .out(out),
                       .key(key),
                       .default_out(default_out),
                       .lut(lut)
                   );

endmodule


// 解码器模块
module decoder #(parameter INPUT_WIDTH) (
    input wire [INPUT_WIDTH-1:0] in,
    output wire [(1<<INPUT_WIDTH)-1:0] out
);

    genvar i;
    generate for (i=0; i<(1<<INPUT_WIDTH); i=i+1) begin : gen_decoder
        assign out[i] = (in == i);
    end
    endgenerate
endmodule

module decoder_2_4(
        input wire [ 1:0] in,
        output wire [ 3:0] out
    );

    decoder #(.INPUT_WIDTH(2)) dec_2_4 (
        .in(in),
        .out(out)
    );

endmodule


module decoder_4_16(
        input wire [ 3:0] in,
        output wire [15:0] out
    );

    decoder #(.INPUT_WIDTH(4)) dec_4_16 (
        .in(in),
        .out(out)
    );

endmodule


module decoder_5_32(
        input wire [ 4:0] in,
        output wire [31:0] out
    );

    decoder #(.INPUT_WIDTH(5)) dec_5_32 (
        .in(in),
        .out(out)
    );

endmodule

module decoder_6_64(
        input wire [ 5:0] in,
        output wire [63:0] out
    );

    decoder #(.INPUT_WIDTH(6)) dec_6_64 (
        .in(in),
        .out(out)
    );

endmodule
