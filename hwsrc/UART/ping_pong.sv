`include "UARTrx.sv"
`include "UARTtx.sv"

module ping_pong
(
    input   wire    clock,      // PIN_Y2
    input   wire    ireset,     // PIN_M23(KEY0)
    input   wire    rx,         // PIN_G12

    output  wire    tx          // PIN_G9
);

wire reset;
assign reset = ~ireset;

wire    [7:0]   data_in;
wire            recv;
wire            idle;

UARTrx rx_unit
(
    .clock(clock),
    .reset(reset),
    .rx(rx),

    .data(data_in),
    .data_valid(recv)
);

UARTtx tx_unit
(
    .clock(clock),
    .reset(reset),
    .data(data_in + 8'b1),
    .send(recv),

    .tx(tx),
    .idle(idle)
);

endmodule
