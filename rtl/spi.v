module spi(
    input i_clk,
    input i_reset,

    input i_addr,
    input i_cs,
    input i_we,
    input  [7:0] i_dat,
    output [7:0] o_dat,

    input i_miso,
    output o_mosi,
    output o_sck,
    output reg o_ss,

    output o_irq
);

parameter DIV = 1;

reg r_mosi;

// rx/tx data
reg [7:0] r_data;

wire w_start_tx = i_cs && i_we && (i_addr == 1'b1); // write to tx-register

// state machine state
localparam
    FIRST_EDGE = 0,
    LAST_EDGE = 7,
    IRQ = 8,
    IDLE = 9;
reg [3:0] r_state; // current state


// counter to divide i_clk
reg [DIV:0] counter;
always @(posedge i_clk)
    if (r_state == IDLE)
        counter <= 0;
    else
        counter <= counter + 1;


wire tick = &counter;

// address map
// 0: ctl/status reg
//    bit #0: ss
//    bit #7: transmission active (read only)
// 1: rx/tx reg

wire [7:0] w_status = { r_state != IDLE, 6'h0, o_ss };

always @(posedge i_clk)
begin
    if (i_cs && i_we) begin
        if (i_addr == 1'b0) begin
            o_ss <= i_dat[0];
        end
    end
    if (i_reset) begin
        o_ss <= 1'b0;
    end
end

// Mode 0: Data sampled on rising edge and shifted out on the falling edge

always @(posedge i_clk) begin
    case (r_state)
        IDLE: if (w_start_tx) begin
            r_state <= FIRST_EDGE;
            r_data <= i_dat;
            r_mosi <= i_dat[7];
        end
        default: if (tick) begin
            r_mosi <= r_data[7];
            // miso sample on posedgde
            r_data <= {r_data[6:0], i_miso};
            r_state <= r_state + 1;
        end
        IRQ: begin
            $display("miso: %x", r_data);
            r_state <= IDLE;
        end
    endcase
    if (i_reset)
        r_state <= IDLE;
end


assign o_dat = i_addr ? r_data : w_status;

assign o_mosi = o_ss && (r_state < 8) ? r_data[7] : 0;
assign o_sck = (r_state < 8) ? counter[DIV] : 1'b0;

assign o_irq = (r_state == IRQ);

endmodule
