/* verilator lint_off DECLFILENAME*/
module regfile #(
    ADDR_WIDTH = 5,  //  地址宽度：5为可寻址32个寄存器
    DATA_WIDTH = 32  //  数据宽度：32位
) (
    input  logic                    clk,
    input  logic [ADDR_WIDTH - 1:0] raddr1,
    output logic [DATA_WIDTH - 1:0] rdata1,
    input  logic [ADDR_WIDTH - 1:0] raddr2,
    output logic [DATA_WIDTH - 1:0] rdata2,
    input  logic                    we,      // 写使能信号
    input  logic [ADDR_WIDTH - 1:0] waddr,
    input  logic [DATA_WIDTH - 1:0] wdata
);
    reg [DATA_WIDTH - 1:0] regfile[(1 << ADDR_WIDTH) - 1 : 0];


    always @(posedge clk) begin
        if (we && waddr != 0) begin
            regfile[waddr] <= wdata;
        end
        regfile[0] <= 0;  // 保持寄存器0为0
    end

    assign rdata1 = (raddr1 == 0) ? 0 : regfile[raddr1];
    assign rdata2 = (raddr2 == 0) ? 0 : regfile[raddr2];

endmodule
