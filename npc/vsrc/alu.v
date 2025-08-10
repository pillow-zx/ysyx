module alu(
    input logic [31:0] alu_src1,
    input logic [31:0] alu_src2,
    input logic [3:0] alu_op,
    output logic [31:0] alu_result,
    output logic alu_zero,
    output logic alu_negative
);

    always_comb begin
        alu_zero  = 1'b0;
        alu_negative = 1'b0;

        case(alu_op)
            4'b0000: begin // ADD
                alu_result = alu_src1 + alu_src2;
            end

            4'b0001: begin // SUB
                alu_result = alu_src1 - alu_src2;
            end

            4'b0010: begin // SLL
                alu_result = alu_src1 << alu_src2[4:0];
            end

            4'b0011: begin // SLT
                alu_result = ($signed(alu_src1) < $signed(alu_src2)) ? 32'b1 : 32'b0;
            end

            4'b0100: begin // SLTU
                alu_result = (alu_src1 < alu_src2) ? 32'b1 : 32'b0;
            end
                
            4'b0101: begin // XOR
                alu_result = alu_src1 ^ alu_src2;
            end

            4'b0110: begin // SRL
                alu_result = alu_src1 >> alu_src2[4:0];
            end

            4'b0111: begin // SRA
                alu_result = $signed(alu_src1) >>> alu_src2[4:0];
            end


            4'b1000: begin // OR
                alu_result = alu_src1 | alu_src2;
            end

            4'b1001: begin // AND
                alu_result = alu_src1 & alu_src2;
            end

            default: begin
                alu_result = 32'b0;
            end
        endcase
        alu_zero = (alu_result == 32'b0);
        alu_negative = alu_result[31]; // Check if the result is negative
    end

endmodule
