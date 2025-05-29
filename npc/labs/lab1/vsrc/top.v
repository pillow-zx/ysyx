module top (
        input wire [1:0] X0,
        input wire [1:0] X1,
        input wire [1:0] X2,
        input wire [1:0] X3,
        input wire [1:0] Y,
        output wire [1:0] F
    );
    MuxKeyWithDefault #(4, 2, 2) mux (
                          .out(F),
                          .key(Y),
                          .default_out(2'b00),
                          .lut({
                                   2'b00, X0[1:0],  // 第一个键值对：键=00, 值=X0
                                   2'b01, X1[1:0],  // 第二个键值对：键=01, 值=X1
                                   2'b10, X2[1:0],  // 第三个键值对：键=10, 值=X2
                                   2'b11, X3[1:0]   // 第四个键值对：键=11, 值=X3
                               })
                      );
endmodule

module MuxKeyInternal #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1, HAS_DEFAULT = 0) (
        output reg [DATA_LEN-1:0] out,
        input [KEY_LEN-1:0] key,
        input [DATA_LEN-1:0] default_out,
        input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
    );

    localparam PAIR_LEN = KEY_LEN + DATA_LEN;
    wire [PAIR_LEN-1:0] pair_list [NR_KEY-1:0];
    wire [KEY_LEN-1:0] key_list [NR_KEY-1:0];
    wire [DATA_LEN-1:0] data_list [NR_KEY-1:0];

    generate
        for (genvar n = 0; n < NR_KEY; n = n + 1) begin
            assign pair_list[n] = lut[PAIR_LEN*(n+1)-1 : PAIR_LEN*n];
            assign data_list[n] = pair_list[n][DATA_LEN-1:0];
            assign key_list[n]  = pair_list[n][PAIR_LEN-1:DATA_LEN];
        end
    endgenerate

    reg [DATA_LEN-1 : 0] lut_out;
    reg hit;
    integer i;
    always @(*) begin
        lut_out = 0;
        hit = 0;
        for (i = 0; i < NR_KEY; i = i + 1) begin
            lut_out = lut_out | ({DATA_LEN{key == key_list[i]}} & data_list[i]);
            hit = hit | (key == key_list[i]);
        end
        if (!HAS_DEFAULT)
            out = lut_out;
        else
            out = (hit ? lut_out : default_out);
    end

endmodule

module MuxKey #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1) (
        output [DATA_LEN-1:0] out,
        input [KEY_LEN-1:0] key,
        input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
    );
    MuxKeyInternal #(NR_KEY, KEY_LEN, DATA_LEN, 0) i0 (out, key, {DATA_LEN{1'b0}}, lut);
endmodule

module MuxKeyWithDefault #(NR_KEY = 2, KEY_LEN = 1, DATA_LEN = 1) (
        output [DATA_LEN-1:0] out,
        input [KEY_LEN-1:0] key,
        input [DATA_LEN-1:0] default_out,
        input [NR_KEY*(KEY_LEN + DATA_LEN)-1:0] lut
    );
    MuxKeyInternal #(NR_KEY, KEY_LEN, DATA_LEN, 1) i0 (out, key, default_out, lut);
endmodule
// module top (
//     input  wire [1:0] X0,
//     input  wire [1:0] X1,
//     input  wire [1:0] X2,
//     input  wire [1:0] X3,
//     input  wire [1:0] Y,
//     output reg [1:0] F
// );
//   always @(*) begin
//     case (Y)
//       2'b00: F = X0;
//       2'b01: F = X1;
//       2'b10: F = X2;
//       2'b11: F = X3;
//       default: F = 2'b00;
//     endcase
//   end
// endmodule
