module top (
        input wire [2:0] A ,
        input wire [2:0] B ,
        input wire [2:0] choice,
        output reg [2:0] result,
        output reg overflow,
        output reg carry_out,
        output reg zero
    );

    always @(*) begin
        // Initialize outputs
        result = 3'b000;
        overflow = 1'b0;
        carry_out = 1'b0;
        zero = 1'b0;

        case (choice)
            3'b000: begin // A + B
                {carry_out, result} = A + B;
                overflow = (A[2] & B[2] & ~result[2]) | (~A[2] & ~B[2] & result[2]);
                if (result == 3'b000) begin
                    zero = 1'b1; // Set zero if result is zero
                end
            end
            3'b001: begin // A - B
                // result = A - B;
                {carry_out, result} = A - B;
                overflow = (A[2] & ~B[2]) | (~A[2] & B[2] & result[2]);

                if (result == 3'b000) begin
                    zero = 1'b1; // Set zero if result is zero
                end
            end
            3'b010: begin
                result = ~A; // NOT A
            end
            3'b011: begin // A & B
                result = A & B;
            end
            3'b100: begin // A | B
                result = A | B;
            end
            3'b101: begin // A ^ B
                result = A ^ B;
            end
            3'b110: begin// 有符号比较
                result = (A == B) ? 3'b001 : 3'b000;
            end
            default: begin
                result = 3'b000;
            end
        endcase
        // Set zero flag if result is zero
        if (result == 3'b000) begin
            zero = 1'b1;
        end
        else begin
            zero = 1'b0;
        end
    end


endmodule
