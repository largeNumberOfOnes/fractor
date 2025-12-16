// Defines for FSM
`define FSM_STATE_WIDTH 4
`define WAIT    0
`define INIT    1 // P(X_1, Z_1), Q(X_2, Z_2), P - Q(X_3, Z_3), P + Q(X_4, Z_4)
`define STAGE_1 2 // t1 = X_2 + Z_2, t2 = X_2 - Z_2, t3 = X_1 + Z_1, t4 = X_1 - Z_1, t5 = t1 * t4, t6 = t2 * t3
`define STAGE_2 3 // waiting for t5, t6
`define STAGE_3 4 // t7 = t5 + t6, t8 = t5 - t6, t7^2, t8^2
`define STAGE_4 5 // waiting for t7^2, t8^2
`define STAGE_5 6 // X_4 = Z_3 * t7^2, Z_4= X_3 * t8^2
`define STAGE_6 7 // waiting for X_4, Z_4
`define DONE    8

`define MODULES_NUM 3

module point_add
#(
    parameter NUM_WIDTH  = 256,
    parameter WORD_WIDTH = 32
)
(
    input  wire             clk,
    input  wire             rst,

    // Control signals
    input  wire             start_add,
    output wire             busy     ,
    output wire             ready    ,

    // Input data
    input  wire [NUM_WIDTH -1:0] X_P,
    input  wire [NUM_WIDTH -1:0] Z_P,
    input  wire [NUM_WIDTH -1:0] X_Q,
    input  wire [NUM_WIDTH -1:0] Z_Q,
    input  wire [NUM_WIDTH -1:0] X_dif_PQ,
    input  wire [NUM_WIDTH -1:0] Z_dif_PQ,
    input  wire [NUM_WIDTH -1:0] N   ,
    input  wire [WORD_WIDTH-1:0] n   ,  // n = -N^{-1} mod 2^32

    // Output data
    output wire  [NUM_WIDTH -1:0] X_out,
    output wire  [NUM_WIDTH -1:0] Z_out
);

// Signals for add_sub_mod module
wire [NUM_WIDTH - 1:0] as_A  [`MODULES_NUM - 1:0];
wire [NUM_WIDTH - 1:0] as_B  [`MODULES_NUM - 1:0];
wire [NUM_WIDTH - 1:0] sum_N [`MODULES_NUM - 1:0];
wire [NUM_WIDTH - 1:0] dif_N [`MODULES_NUM - 1:0];

// Signals for mont_mul module
reg  [NUM_WIDTH - 1:0] mm_A  [`MODULES_NUM - 2:0];
reg  [NUM_WIDTH - 1:0] mm_B  [`MODULES_NUM - 2:0];
wire [NUM_WIDTH - 1:0] mm_P  [`MODULES_NUM - 2:0];

reg  [`MODULES_NUM - 2:0] mm_start;
wire [`MODULES_NUM - 2:0] mm_busy ;
wire [`MODULES_NUM - 2:0] mm_ready;

genvar Gi;

// Copies of input data signals
reg [NUM_WIDTH  - 1:0] X_P_reg;
reg [NUM_WIDTH  - 1:0] Z_P_reg;

reg [NUM_WIDTH  - 1:0] X_Q_reg;
reg [NUM_WIDTH  - 1:0] Z_Q_reg;

reg [NUM_WIDTH  - 1:0] X_dif_PQ_reg;
reg [NUM_WIDTH  - 1:0] Z_dif_PQ_reg;

reg [NUM_WIDTH  - 1:0] N_reg;
reg [WORD_WIDTH - 1:0] n_reg;

// Internal FSM signals
reg [`FSM_STATE_WIDTH - 1:0] fsm_state;

// Temp. signals for stages 1-5
wire [NUM_WIDTH - 1:0] t1;
wire [NUM_WIDTH - 1:0] t2;
wire [NUM_WIDTH - 1:0] t3;
wire [NUM_WIDTH - 1:0] t4;

reg  [NUM_WIDTH - 1:0] t5;
reg  [NUM_WIDTH - 1:0] t6;

wire [NUM_WIDTH - 1:0] t7;
wire [NUM_WIDTH - 1:0] t8;

// Squares of t7 and t8
reg  [NUM_WIDTH - 1:0] t7_sq;
reg  [NUM_WIDTH - 1:0] t8_sq;

// Values of P - Q
wire [NUM_WIDTH - 1:0] X_3;
wire [NUM_WIDTH - 1:0] Z_3;

// Preliminary values of P + Q
reg  [NUM_WIDTH - 1:0] X_4;
reg  [NUM_WIDTH - 1:0] Z_4;


// Instantination of `MODULES_NUM add_sub_mod and mont_mul modules
generate
if(1) begin: gen_montmul_and_addsub_modules
    for(Gi = 0; Gi < `MODULES_NUM; Gi = Gi + 1) begin: module_iter
        add_sub_mod #(
            .NUM_WIDTH(NUM_WIDTH)
        )
        addsub_i (
                .A     (as_A [Gi]),
                .B     (as_B [Gi]),
                .N     (N_reg    ),
                .sum_N (sum_N[Gi]),  // A + B (mod N)
                .dif_N (dif_N[Gi])   // A - B (mod N)
        );

        // we need only 2 mont_mul
        if(Gi < 2) begin
            mont_mul #(
                    .NUM_WIDTH (NUM_WIDTH ),
                    .WORD_WIDTH(WORD_WIDTH)
            )
            montmul_i (
                    .clk      (clk),
                    .rst      (rst),

                    .start_mul(mm_start[Gi]),
                    .busy     (mm_busy [Gi]),
                    .ready    (mm_ready[Gi]),
                    .A        (mm_A    [Gi]),
                    .B        (mm_B    [Gi]),
                    .N        (N_reg       ),
                    .n        (n_reg       ),
                    .P        (mm_P    [Gi]) // P = A * B * P^(-1) (mod N)
            );
        end
    end
end
endgenerate

// Logic for FSM states
always @(posedge clk) begin
    if(rst) begin
        fsm_state <= `WAIT;
    end

    else begin
        fsm_state <= ({`FSM_STATE_WIDTH {fsm_state == `INIT   }} & `STAGE_1)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `STAGE_1}} & `STAGE_2)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `STAGE_2}} & {`FSM_STATE_WIDTH {~(&mm_ready)}} & `STAGE_2)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `STAGE_2}} & {`FSM_STATE_WIDTH {  &mm_ready }} & `STAGE_3)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `STAGE_3}} & `STAGE_4)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `STAGE_4}} & {`FSM_STATE_WIDTH {~(&mm_ready)}} & `STAGE_4)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `STAGE_4}} & {`FSM_STATE_WIDTH {  &mm_ready }} & `STAGE_5)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `STAGE_5}} & `STAGE_6)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `STAGE_6}} & {`FSM_STATE_WIDTH {~(&mm_ready)}} & `STAGE_6)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `STAGE_6}} & {`FSM_STATE_WIDTH {  &mm_ready }} & `DONE   )  |
                     ({`FSM_STATE_WIDTH {fsm_state == `DONE   }} & `WAIT   )  |
                     ({`FSM_STATE_WIDTH {fsm_state == `WAIT   }} & {`FSM_STATE_WIDTH {~start_add}} & `WAIT) |
                     ({`FSM_STATE_WIDTH {fsm_state == `WAIT   }} & {`FSM_STATE_WIDTH { start_add}} & `INIT) ;
    end
end

// Control signals
assign
    busy  = (fsm_state >= `INIT) & (fsm_state <= `STAGE_6),
    ready =  fsm_state == `DONE;

// Fixing input data (initial state)
always @(posedge clk) begin
    if(rst) begin
        X_P_reg      <= {NUM_WIDTH  {1'b0}};
        Z_P_reg      <= {NUM_WIDTH  {1'b0}};

        X_Q_reg      <= {NUM_WIDTH  {1'b0}};
        Z_Q_reg      <= {NUM_WIDTH  {1'b0}};

        X_dif_PQ_reg <= {NUM_WIDTH  {1'b0}};
        Z_dif_PQ_reg <= {NUM_WIDTH  {1'b0}};

        N_reg        <= {NUM_WIDTH  {1'b0}};
        n_reg        <= {WORD_WIDTH {1'b0}};
    end

    else begin
        X_P_reg      <= fsm_state == `INIT ? X_P      : X_P_reg;
        Z_P_reg      <= fsm_state == `INIT ? Z_P      : Z_P_reg;

        X_Q_reg      <= fsm_state == `INIT ? X_Q      : X_Q_reg;
        Z_Q_reg      <= fsm_state == `INIT ? Z_Q      : Z_Q_reg;

        X_dif_PQ_reg <= fsm_state == `INIT ? X_dif_PQ : X_dif_PQ_reg;
        Z_dif_PQ_reg <= fsm_state == `INIT ? Z_dif_PQ : Z_dif_PQ_reg;

        N_reg        <= fsm_state == `INIT ? N        : N_reg   ;
        n_reg        <= fsm_state == `INIT ? n        : n_reg   ;
    end
end

// Mont_mul input logic #0
always @(posedge clk) begin
    if(rst) begin
        mm_A[0] <= {NUM_WIDTH {1'b0}};
        mm_B[0] <= {NUM_WIDTH {1'b0}};

        mm_start[0] <= 1'b0;
    end

    else begin
        mm_A[0] <= fsm_state == `STAGE_1 ? t1    :
                   fsm_state == `STAGE_3 ? t7    :
                   fsm_state == `STAGE_5 ? Z_3   : mm_A[0];
        mm_B[0] <= fsm_state == `STAGE_1 ? t4    :
                   fsm_state == `STAGE_3 ? t7    :
                   fsm_state == `STAGE_5 ? t7_sq : mm_B[0];

        mm_start[0] <= (fsm_state == `STAGE_1) | (fsm_state == `STAGE_3) | (fsm_state == `STAGE_5);
    end
end

// Mont_mul input logic #1
always @(posedge clk) begin
    if(rst) begin
        mm_A[1] <= {NUM_WIDTH {1'b0}};
        mm_B[1] <= {NUM_WIDTH {1'b0}};

        mm_start[1] <= 1'b0;
    end

    else begin
        mm_A[1] <= fsm_state == `STAGE_1 ? t2    :
                   fsm_state == `STAGE_3 ? t8    :
                   fsm_state == `STAGE_5 ? X_3   : mm_A[1];
        mm_B[1] <= fsm_state == `STAGE_1 ? t3    :
                   fsm_state == `STAGE_3 ? t8    :
                   fsm_state == `STAGE_5 ? t8_sq : mm_B[1];

        mm_start[1] <= (fsm_state == `STAGE_1) | (fsm_state == `STAGE_3) | (fsm_state == `STAGE_5);
    end
end

// Stage 1
// Logic for t1 and t2
assign
    as_A[0] = X_Q_reg,
    as_B[0] = Z_Q_reg;

assign
    t1 = sum_N[0],
    t2 = dif_N[0];

// Logic for t3 and t4
assign
    as_A[1] = X_P_reg,
    as_B[1] = Z_P_reg;

assign
    t3 = sum_N[1],
    t4 = dif_N[1];

// Stage 2 (logic for t5 and t6)
always @(posedge clk) begin
    if(rst) begin
        t5 <= {NUM_WIDTH {1'b0}};
        t6 <= {NUM_WIDTH {1'b0}};
    end

    else begin
        t5 <=  fsm_state == `INIT                   ? {NUM_WIDTH {1'b0}} :
              (fsm_state == `STAGE_2) & (&mm_ready) ? mm_P[0]            : t5;
        t6 <=  fsm_state == `INIT                   ? {NUM_WIDTH {1'b0}} :
              (fsm_state == `STAGE_2) & (&mm_ready) ? mm_P[1]            : t6;
    end
end

// Stage 3
// Logic for t7 and t8
assign
    as_A[2] = t5,
    as_B[2] = t6;

assign
    t7 = sum_N[2],
    t8 = dif_N[2];

// Stage 4 (logic for t7^2 and t8^2)
always @(posedge clk) begin
    if(rst) begin
        t7_sq <= {NUM_WIDTH {1'b0}};
        t8_sq <= {NUM_WIDTH {1'b0}};
    end

    else begin
        t7_sq <=  fsm_state == `INIT                   ? {NUM_WIDTH {1'b0}} :
                 (fsm_state == `STAGE_4) & (&mm_ready) ? mm_P[0]            : t7_sq;
        t8_sq <=  fsm_state == `INIT                   ? {NUM_WIDTH {1'b0}} :
                 (fsm_state == `STAGE_4) & (&mm_ready) ? mm_P[1]            : t8_sq;
    end
end

// Stage 5
// Logic for X_3 and Z_3
assign
    X_3 = X_dif_PQ_reg,
    Z_3 = Z_dif_PQ_reg;

// Stage 6 (logic for Z_2)
always @(posedge clk) begin
    if(rst) begin
        X_4 <= {NUM_WIDTH {1'b0}};
        Z_4 <= {NUM_WIDTH {1'b0}};
    end

    else begin
        X_4 <=  fsm_state == `INIT                   ? {NUM_WIDTH {1'b0}} :
               (fsm_state == `STAGE_6) & (&mm_ready) ? mm_P[0]            : X_4;
        Z_4 <=  fsm_state == `INIT                   ? {NUM_WIDTH {1'b0}} :
               (fsm_state == `STAGE_6) & (&mm_ready) ? mm_P[1]            : Z_4;
    end
end

//Done stage
assign
    X_out = X_4,
    Z_out = Z_4;


endmodule
