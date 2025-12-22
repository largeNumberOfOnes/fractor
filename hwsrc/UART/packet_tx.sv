`include "UARTtx.sv"

module packet_tx
(
    input   wire                    clock,
    input   wire                    reset,
    input   wire    [7:0]           cmd,
    input   wire    [15:0]          len,
    input   wire    [255:0]         payload,
    input   wire                    send,

    output  wire                    tx,
    output  reg                     busy
);

reg [295:0] buffer;
reg [8:0]   ptr;

wire start;
wire UART_idle;

always @(posedge clock)
    buffer <= start ? {8'h55, 8'hAA, cmd, len, len, payload} : buffer;

always @(posedge clock)
begin
    if(reset | start)
        ptr <= 0;
    else
        ptr <= (ptr < 296) ? (ptr + 1) : ptr;
end

UARTtx tx_unit
(
    .clock(clock),
    .reset(reset),
    .data(buffer >> (ptr << 3)),
    .send(busy & UART_idle),

    .tx(tx),
    .idle(UART_idle)
);

endmodule
