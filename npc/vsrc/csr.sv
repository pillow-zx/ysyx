/* verilator lint_off DECLFILENAME*/
module csr (
    input logic clk,
    input logic rst_n,
    input logic csr_we,
    input logic [11:0] addr,
    input logic [31:0] csr_wdata,
    input logic is_ecall,
    input logic is_mret,
    input logic [31:0] pc_in,
    output logic [31:0] csr_rdata,
    output logic [31:0] next_pc
);
    logic [31:0] mtvec, mepc, mcause, mstatus;

    localparam MTVEC = 12'h305;
    localparam MEPC = 12'h341;
    localparam MCAUSE = 12'h342;
    localparam MSTATUS = 12'h300;

    always_comb begin
        case (addr)
            MTVEC: csr_rdata = mtvec;
            MEPC: csr_rdata = mepc;
            MSTATUS: csr_rdata = mstatus;
            MCAUSE: csr_rdata = mcause;
            default: csr_rdata = 32'b0;
        endcase
    end

    function automatic [31:0] update_mstatus_mret(input [31:0] in_mstatus);
        update_mstatus_mret = in_mstatus;
        update_mstatus_mret[3] = in_mstatus[7];
        update_mstatus_mret[7] = 1'b1;
        update_mstatus_mret[12:11] = 2'b00;
    endfunction

    always_comb begin
        if (is_ecall) begin
            next_pc = mtvec;
        end else if (is_mret) begin
            next_pc = mepc;
        end else begin
            next_pc = 32'b0;
        end
    end

    always_ff @(posedge clk or negedge rst_n) begin
        if (!rst_n) begin
            mtvec <= 32'b0;
            mepc <= 32'b0;
            mcause <= 32'b0;
            mstatus <= 32'h1800;
            // $display("pc: %h, reset csr regs mstatus: %h", pc_in, mstatus);
        end else begin
            if (is_ecall) begin
                // $display("pc: %h, mtvec: %h, mepc: %h, mcause: %h, mstatus: %h", pc_in, mtvec,
                //          mepc, mcause, mstatus);
                mepc   <= pc_in;
                mcause <= 32'd11;
                // mstatus <= {mstatus[31:13], 2'b11, mstatus[10:0]};
            end else if (is_mret) begin
                // mstatus <= update_mstatus_mret(mstatus);
            end else if (csr_we) begin
                case (addr)
                    MTVEC: mtvec <= csr_wdata;
                    MEPC: mepc <= csr_wdata;
                    MCAUSE: mcause <= csr_wdata;
                    MSTATUS: mstatus <= csr_wdata;
                    default begin

                    end
                endcase
            end
        end
    end


endmodule
