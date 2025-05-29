module top (
        input wire clk,
        input wire [7:0] ext_in,
        output reg [7:0] out,
        output reg [6:0] lights1,
        output wire [6:0] lights2
    );

    reg [7:0] in;
    reg i;

    always @(posedge clk) begin
        if (ext_in == 8'b0) begin
            i <= 0;
        end
        else begin
            i <= 1;
        end
        if (i) begin
            out <= {in[0]^in[2]^in[3]^in[4], in[7:1]};
            in <= out;
        end
        else begin
            in <= ext_in;
            out <= 8'b00000000;
        end
    end

    lights lights1_inst (
               .clk(clk),
               .in(in[3:0]),
               .light(lights1)
           );

    lights lights2_inst (
               .clk(clk),
               .in(in[7:4]),
               .light(lights2)
           );
endmodule



module lights (
        input wire clk,
        input wire [3:0] in,
        output reg [6:0] light
    );

    always @(posedge clk) begin
        if (in == 4'b0) begin
            light <= 7'b1000000;
        end
        else begin
            case (in)
                4'b0000:
                    light <= 7'b1000000; // 0
                4'b0001:
                    light <= 7'b1111001; // 1
                4'b0010:
                    light <= 7'b0100100; // 2
                4'b0011:
                    light <= 7'b0110000; // 3
                4'b0100:
                    light <= 7'b0011001; // 4
                4'b0101:
                    light <= 7'b0010010; // 5
                4'b0110:
                    light <= 7'b0000010; // 6
                4'b0111:
                    light <= 7'b1111000; // 7
                4'b1000:
                    light <= 7'b0000000; // 8
                4'b1001:
                    light <= 7'b0010000; // 9
                4'b1010:
                    light <= 7'b0001000; // A
                4'b1011:
                    light <= 7'b0000011; // B
                4'b1100:
                    light <= 7'b1000110; // C
                4'b1101:
                    light <= 7'b0100001; // D
                4'b1110:
                    light <= 7'b0000110; // E
                4'b1111:
                    light <= 7'b0001110; // F
                default:
                    light <= 7'b1000000; // Default case, all segments off
            endcase
        end
    end

endmodule



