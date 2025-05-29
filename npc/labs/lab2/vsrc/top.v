module top(
        input en,
        input wire [7:0] in,
        output reg [2:0] out,
        output reg [6:0] light,
        output reg out_in
    );

    reg [2:0] _out;

    integer i;
    always @(en or in) begin
        if (en) begin
            _out = 3'b000; // Default value if enable is high
            out_in = 1'b0; // Default output_in if enable is high
            for (i = 0; i < 8; i = i + 1) begin
                if (in[7 - i] == 1) begin
                    _out = i[2:0];
                    out_in = 1'b1;
                    break; // Exit loop after finding the first '1'
                end
            end
        end
        else begin
            _out = 3'b000; // Default value if enable is low
            out_in = 1'b0; // Default output_in if enable is low
        end
        case(_out)
            3'b000:
                light = 7'b1000000; // 0
            3'b001:
                light = 7'b1111001;
            3'b010:
                light = 7'b0100100; // 2
            3'b011:
                light = 7'b0110000; // 3
            3'b100:
                light = 7'b0011001; // 4
            3'b101:
                light = 7'b0010010; // 5
            3'b110:
                light = 7'b0000010; // 6
            3'b111:
                light = 7'b1111000; // 7
            default:
                light = 7'b1000000; // Default case, all segments off
        endcase
    end
    assign out = _out;
endmodule
