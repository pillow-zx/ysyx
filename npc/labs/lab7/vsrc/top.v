module top (
        input clk,
        input resetn,
        input ps2_clk,
        input ps2_data,
        output [6:0] lights1,
        output [6:0] lights2,
        output [6:0] lights3,
        output [6:0] lights4,
        output [6:0] lights5,
        output [6:0] lights6
    );


    wire [7:0] scan_code;
    wire [7:0] key_count_out;
    ps2_keyboard ps2_keyboard_inst(
                     .clk(clk),
                     .resetn(~resetn),
                     .ps2_clk(ps2_clk),
                     .ps2_data(ps2_data),
                     .scan_code(scan_code),
                     .key_count_out(key_count_out)
                 );


    wire [7:0] ascii_out;
    scan_to_ascii scan_to_ascii_inst(
                      .clk(clk),
                      .scan_in(scan_code),
                      .ascii_out(ascii_out)
                  );


    key_display key_display_inst(
                    .key_in(scan_code),
                    .lights1(lights1),
                    .lights2(lights2)
                );

    key_display key_display_inst2(
                    .key_in(ascii_out),
                    .lights1(lights3),
                    .lights2(lights4)
                );




    key_display key_display_inst3(
                    .key_in(key_count_out),
                    .lights1(lights5),
                    .lights2(lights6)
                );
endmodule

module ps2_keyboard(clk,resetn,ps2_clk,ps2_data, scan_code, key_count_out);
    input clk,resetn,ps2_clk,ps2_data;
    output reg [7:0] scan_code; // 输出扫描码
    output reg [7:0] key_count_out; // 输出按键计数

    reg [9:0] buffer;        // ps2_data bits
    reg [3:0] count;  // count ps2_data bits
    reg [2:0] ps2_clk_sync;

    always @(posedge clk) begin
        ps2_clk_sync <=  {ps2_clk_sync[1:0],ps2_clk};
    end

    wire sampling = ps2_clk_sync[2] & ~ps2_clk_sync[1];

    always @(posedge clk) begin
        if (resetn == 0) begin // reset
            count <= 0;
        end
        else begin
            if (sampling) begin
                if (count == 4'd10) begin
                    if ((buffer[0] == 0) &&  // start bit
                            (ps2_data)       &&  // stop bit
                            (^buffer[9:1])) begin      // odd  parity
                        $display("receive %x", buffer[8:1]);
                        scan_code <= buffer[8:1]; // 提取扫描码
                        if (buffer[8:1] != 8'hF0) begin // not release code
                            key_count_out <= key_count_out + 1; // count key
                        end
                    end
                    count <= 0;     // for next
                end
                else begin
                    buffer[count] <= ps2_data;  // store ps2_data
                    count <= count + 3'b1;
                end
            end
        end
    end

endmodule

module scan_to_ascii(
        input clk,
        input [7:0] scan_in,
        output reg [7:0] ascii_out
    );

    function [7:0] scan_to_ascii;
        input [7:0] scan;
        case(scan)
            8'h1C:
                scan_to_ascii = 8'h61; // a
            8'h32:
                scan_to_ascii = 8'h62; // b
            8'h21:
                scan_to_ascii = 8'h63; // c
            8'h23:
                scan_to_ascii = 8'h64; // d
            8'h24:
                scan_to_ascii = 8'h65; // e
            8'h2B:
                scan_to_ascii = 8'h66; // f
            8'h34:
                scan_to_ascii = 8'h67; // g
            8'h33:
                scan_to_ascii = 8'h68; // h
            8'h43:
                scan_to_ascii = 8'h69; // i
            8'h3B:
                scan_to_ascii = 8'h6A; // j
            8'h42:
                scan_to_ascii = 8'h6B; // k
            8'h4B:
                scan_to_ascii = 8'h6C; // l
            8'h3A:
                scan_to_ascii = 8'h6D; // m
            8'h31:
                scan_to_ascii = 8'h6E; // n
            8'h44:
                scan_to_ascii = 8'h6F; // o
            8'h4D:
                scan_to_ascii = 8'h70; // p
            8'h15:
                scan_to_ascii = 8'h71; // q
            8'h2D:
                scan_to_ascii = 8'h72; // r
            8'h1B:
                scan_to_ascii = 8'h73; // s
            8'h2C:
                scan_to_ascii = 8'h74; // t
            8'h3C:
                scan_to_ascii = 8'h75; // u
            8'h2A:
                scan_to_ascii = 8'h76; // v
            8'h1D:
                scan_to_ascii = 8'h77; // w
            8'h22:
                scan_to_ascii = 8'h78; // x
            8'h35:
                scan_to_ascii = 8'h79; // y
            8'h1A:
                scan_to_ascii = 8'h7A; // z
            8'h45:
                scan_to_ascii = 8'h30; // 0
            8'h16:
                scan_to_ascii = 8'h31; // 1
            8'h1E:
                scan_to_ascii = 8'h32; // 2
            8'h26:
                scan_to_ascii = 8'h33; // 3
            8'h25:
                scan_to_ascii = 8'h34; // 4
            8'h2E:
                scan_to_ascii = 8'h35; // 5
            8'h36:
                scan_to_ascii = 8'h36; // 6
            8'h3D:
                scan_to_ascii = 8'h37; // 7
            8'h3E:
                scan_to_ascii = 8'h38; // 8
            8'h46:
                scan_to_ascii = 8'h39; // 9
            default:
                scan_to_ascii = 8'b0; // default case, no mapping
        endcase
    endfunction

    always @(posedge clk) begin
        ascii_out <= scan_to_ascii(scan_in);
    end
endmodule


module key_display (
        input [7:0] key_in,
        output [6:0] lights1,
        output [6:0] lights2
    );

    function [6:0] light;
        input [3:0] key;
        case(key)
            4'b0000:
                light = 7'b1000000; // 0
            4'b0001:
                light = 7'b1111001; // 1
            4'b0010:
                light = 7'b0100100; // 2
            4'b0011:
                light = 7'b0110000; // 3
            4'b0100:
                light = 7'b0011001; // 4
            4'b0101:
                light = 7'b0010010; // 5
            4'b0110:
                light = 7'b0000010; // 6
            4'b0111:
                light = 7'b1111000; // 7
            4'b1000:
                light = 7'b0000000; // 8
            4'b1001:
                light = 7'b0010000; // 9
            4'b1010:
                light = 7'b0001000; // A
            4'b1011:
                light = 7'b0000011; // B
            4'b1100:
                light = 7'b1000110; // C
            4'b1101:
                light = 7'b0100001; // D
            4'b1110:
                light = 7'b0000110; // E
            4'b1111:
                light = 7'b0001110; // F
            default:
                light = 7'b1000000; // Default case, all segments
        endcase
    endfunction

    assign lights1 = light(key_in[3:0]); // lower nibble
    assign lights2 = light(key_in[7:4]); // upper nibble

endmodule
