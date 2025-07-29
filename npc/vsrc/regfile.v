// 32位寄存器文件模块
module ysyx_25060173_RegisterFile #(
    ADDR_WIDTH = 5,
    DATA_WIDTH = 32
) (
    input  wire                    clk,
    input  wire [ADDR_WIDTH - 1:0] raddr1,
    output wire [DATA_WIDTH - 1:0] rdata1,
    input  wire [ADDR_WIDTH - 1:0] raddr2,
    output wire [DATA_WIDTH - 1:0] rdata2,
    input  wire                    we,      // 写使能信号
    input  wire [ADDR_WIDTH - 1:0] waddr,
    input  wire [DATA_WIDTH - 1:0] wdata
);
  reg [DATA_WIDTH - 1:0] regfile[(1 << ADDR_WIDTH) - 1:0];

  always @(posedge clk) begin
    if (we) begin
      regfile[waddr] <= wdata;
    end
    regfile[0] <= 0;  // 保持寄存器0为0
  end

  assign rdata1 = (raddr1 == 0) ? 0 : regfile[raddr1];
  assign rdata2 = (raddr2 == 0) ? 0 : regfile[raddr2];

endmodule
