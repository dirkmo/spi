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

// CPOL: 0: clock idle low, 1: clock idle high
// CPHA: 0: sample miso on first edge, 1: sample miso on second edge
reg cpol, cpha;


// rx/tx data
// data to send is overwritten while receiving
reg [7:0] r_data;


// counter to divide i_clk
reg [0:0] counter;
always @(posedge i_clk)
    if (|counter || start_tx)
        counter <= counter + 1;

wire w_edge = &counter && (r_idx!=4'b1111);

// clock edge counter 0..15
reg [3:0] r_idx;
always @(posedge i_clk) begin
    if (w_edge)
        r_idx <= r_idx + 1'b1;
    if (start_tx)
        r_idx <= 0;
end



// address map
// 0: ctl/status reg
//    bit #0: ss
//    bit #1: cpol
//    bit #2: cpha
//    bit #7: transmission active (read only)
// 1: rx/tx reg


always @(posedge i_clk)
begin
    if (i_cs && i_we) begin
        if (i_addr == 1'b0) begin
            { cpha, cpol, o_ss }<= i_dat[2:0];
        end
    end
    if (i_reset) begin
        { cpha, cpol, o_ss } <= 3'b000;
    end
end

wire start_tx = i_cs && i_we && (i_addr == 1'b1); // write to tx-register

always @(posedge i_clk) begin
    if (start_tx) begin
        r_data <= i_dat;
    end else if (w_edge && r_idx < 8) begin
        r_data <= { r_data[6:0], i_miso };
    end
end


assign o_dat = r_data;

assign o_mosi = r_data[7];
assign o_sck = r_idx[0] ? ~cpol : cpol;


endmodule
