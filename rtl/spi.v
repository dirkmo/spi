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
    output reg o_ss
);

// rx/tx data
// data to send is overwritten while receiving
reg [7:0] r_data = 8'h81;

reg r_mosi;

// counter to divide i_clk
reg [0:0] counter;
always @(posedge i_clk)
    counter <= counter + 1;

wire w_edge = &counter;



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

wire w_start_tx = i_cs && i_we && (i_addr == 1'b1); // write to tx-register
reg r_start_tx;

localparam
    FIRST_EDGE = 0,
    LAST_EDGE = 15,
    IDLE = 16;

reg [4:0] r_state;
always @(posedge i_clk) begin
    if (w_edge) begin
        case (r_state)
            IDLE: if (r_start_tx) begin
                r_state <= FIRST_EDGE;
                r_mosi <= r_data[7];
            end
            default: begin
                // miso sample on posedgde
                if (r_state[0]) begin
                    r_data <= {r_data[6:0], i_miso};
                end else begin
                    r_mosi <= r_data[7];
                end
                r_state <= r_state + 1;
            end
        endcase
    end
    if (i_reset)
        r_state <= IDLE;
end

always @(posedge i_clk)
    if (r_state == IDLE)
        r_start_tx <= w_start_tx;
    else
        r_start_tx <= 0;


assign o_dat = i_addr ? r_data : w_status;

assign o_mosi = r_mosi;
assign o_sck = r_state[0];


endmodule
