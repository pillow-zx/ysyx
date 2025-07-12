module core (
    input wire  clk,
    input wire rst
);

reg reset;
always @(posedge clk) begin
    reset <= rst;
end

reg valid;
always @(posedge clk) begin
    if (reset) begin
        valid <= 1'b0;
    end else begin
        valid <= 1'b1; // 假设每个时钟周期都有效
    end
end

endmodule