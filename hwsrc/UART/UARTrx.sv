`include "UART_config.sv"

// 8N1(10 bits per byte) UART receiver
module UARTrx
(
    input   wire                    clock,
    input   wire                    reset,
    input   wire                    rx,

    output  reg     [`WIDTH-1:0]    data,
    output  reg                     data_valid // 1 tick after receiving
);

localparam CLOCKS_PER_BIT   = `CLK_FREQ / `BAUD_RATE;
localparam HALF_DELAY       = CLOCKS_PER_BIT / 2;
localparam COUNTER_WIDTH    = $clog2(CLOCKS_PER_BIT);
localparam BIT_CNT_WIDTH    = $clog2(`WIDTH);

reg     [COUNTER_WIDTH:0]   baud_counter;
reg     [BIT_CNT_WIDTH:0]   bit_counter;
reg                         idle;
wire                        read_enable;
wire                        stop_bit;
wire                        start;
reg                         rx_half_sync;
reg                         rx_sync;

assign read_enable  = (baud_counter == 0) & {~idle};
assign stop_bit     = read_enable & bit_counter[3] & rx_sync;
assign start        = idle & {~rx_sync};

always @(posedge clock)
begin
    if(reset)
    begin
        rx_half_sync    <= 1'b1; // idle
        rx_sync         <= 1'b1; // idle
    end
    else
    begin
        rx_half_sync    <= rx;
        rx_sync         <= rx_half_sync;
    end
end

always @(posedge clock)
begin
    if(reset | start)
        bit_counter <= 4'b0;
    else
        bit_counter <= (read_enable & (bit_counter < `WIDTH)) ? (bit_counter + 1) : bit_counter;
end

genvar i;
generate
    for(i = 0; i < `WIDTH; i++)
    begin : data_assignment
        always @(posedge clock)
            data[i] <= ((bit_counter == i) & read_enable) ? rx_sync : data[i];
    end
endgenerate

always @(posedge clock)
begin
    if(reset)
        data_valid <= 1'b0;
    else
        data_valid <= read_enable & (bit_counter == {`WIDTH - 1});
end

always @(posedge clock)
    baud_counter <= start ? 3*HALF_DELAY : // skip start-bit and go to middle
                    (baud_counter == 0) ? (CLOCKS_PER_BIT - 1) : (baud_counter - 1);

always @(posedge clock)
begin
    if(reset)
        idle <= 1'b1;
    else
        idle <= idle ? rx_sync : stop_bit;
end

endmodule
