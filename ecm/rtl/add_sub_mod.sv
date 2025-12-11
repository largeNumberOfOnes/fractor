module add_sub_mod
#(
    parameter NUM_WIDTH = 256
)
(
    input  wire [NUM_WIDTH-1:0] A,
    input  wire [NUM_WIDTH-1:0] B,
    input  wire [NUM_WIDTH-1:0] N,
    output wire [NUM_WIDTH-1:0] sum_N,   // (A + B) mod N
    output wire [NUM_WIDTH-1:0] dif_N    // (A - B) mod N
);

    wire [NUM_WIDTH:0] A_ext;
    wire [NUM_WIDTH:0] B_ext;
    wire [NUM_WIDTH:0] N_ext;

    wire [NUM_WIDTH:0] sum_ext ;
    wire [NUM_WIDTH:0] sum_corr;

    wire [NUM_WIDTH:0] dif_ext ;
    wire [NUM_WIDTH:0] dif_corr;

    assign
        A_ext = {1'b0, A},
        B_ext = {1'b0, B},
        N_ext = {1'b0, N};

    // Add: sum_N = (A + B) mod N
    assign
        sum_ext  =   A_ext + B_ext,      // 0 .. 2^(WIDTH+1)-2
        sum_corr = sum_ext - N_ext;      // correction, if it is needed

    assign sum_N = sum_ext >= N_ext ? sum_corr[NUM_WIDTH-1:0]
                                    : sum_ext [NUM_WIDTH-1:0];

    // Sub: dif_N = (A - B) mod N
    assign
        dif_ext  =   A_ext - B_ext,     // may be negative
        dif_corr = dif_ext + N_ext;     // correction for negative difference

    assign dif_N = dif_ext[NUM_WIDTH] ? dif_corr[NUM_WIDTH-1:0]
                                      : dif_ext [NUM_WIDTH-1:0];

endmodule
