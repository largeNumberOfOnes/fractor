`include "UARTrx.sv"
`include "UARTtx.sv"

module ping_pong
(
    input   wire            clock,      // PIN_Y2
    input   wire            ireset,     // PIN_M23(KEY0)
    input   wire            rx,         // PIN_G12

    output  wire            tx,         // PIN_G9
    output  reg     [7:0]   leds,       // PINS(G21, G22, G20, H21, E24, E25, E22, E21)
    output  reg     [7:0]   data_leds,  // PINS(H19, J19, E18, F18, F21, E19, F19, G19)
    output  wire            idle        // PIN_H15
);

reg reset;
always @(posedge clock)
    reset <= ~ireset;

wire    [7:0]   data_in;
wire            recv;

always @(posedge clock)
    data_leds <= recv ? data_in : data_leds;

always @(posedge clock)
    leds <= reset ? 8'b0 : (leds + {7'b0, recv});

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
