`include "UART_config.sv"

// 8N1(10 bits per byte) UART transmitter
module UARTtx_sync
(
    input   wire                    clock,
    input   wire                    reset,
    input   wire    [`WIDTH-1:0]    data,
    input   wire                    send,
    output  reg                     tx,
    output  reg                     idle
);

localparam CLOCKS_PER_BIT   = `CLK_FREQ / `BAUD_RATE;
localparam COUNTER_WIDTH    = $clog2(CLOCKS_PER_BIT);
localparam BIT_CNT_WIDTH    = $clog2(`WIDTH);

reg     [COUNTER_WIDTH-1:0] baud_counter;
reg     [BIT_CNT_WIDTH:0]   bit_counter;
reg     [`WIDTH-1:0]        buffer;
wire                        write_enable;
wire                        start;

assign write_enable = (baud_counter == 0);
assign start        = send & idle;

always @(posedge clock)
    buffer <= start ? data : buffer;

always @(posedge clock)
begin
    if(reset | start)
        bit_counter <= 4'b0;
    else
        bit_counter <= (write_enable & (bit_counter < `WIDTH)) ? (bit_counter + 1) : bit_counter;
end

always @(posedge clock)
begin
    if(reset)
        tx <= 1'b1; // idle
    else if(start)
        tx <= 1'b0; // start-bit
    else if(write_enable)
        tx <= (bit_counter < `WIDTH) ? buffer[bit_counter] : 1'b1; // data or stop-bit
    else
        tx <= tx;
end

always @(posedge clock)
    baud_counter <= (start | (baud_counter == 0)) ? (CLOCKS_PER_BIT - 1) : (baud_counter - 1);

always @(posedge clock)
    idle <= write_enable & bit_counter[3];

endmodule
