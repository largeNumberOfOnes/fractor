// Defines for FSM
`define FSM_STATE_WIDTH 4
`define WAIT    0
`define INIT    1
`define STAGE_1 2 // t1 = X - Z, t2 = X + Z, t1^2, t2^2
`define STAGE_2 3 // waiting for t1^2, t2^2
`define STAGE_3 4 // t3 = t2^2 - t1^2, X_2 = t1^2 * t2^2, t4 = A24 * t3
`define STAGE_4 5 // waiting for X_2, t4
`define STAGE_5 6 // t5 = t1^2 + t4, Z_2 = t3 * t5
`define STAGE_6 7 // waiting for Z_2
`define DONE    8

`define MODULES_NUM 3

module point_double
#(
    parameter NUM_WIDTH  = 256,
    parameter WORD_WIDTH = 32
)
(
    input  wire             clk,
    input  wire             rst,

    // Control signals
    input  wire             start_mul,
    output wire             busy     ,
    output wire             ready    ,

    // Input data
    input  wire [NUM_WIDTH -1:0] X_in,
    input  wire [NUM_WIDTH -1:0] Z_in,
    input  wire [NUM_WIDTH -1:0] A24 ,
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
reg [NUM_WIDTH  - 1:0] X_in_reg;
reg [NUM_WIDTH  - 1:0] Z_in_reg;
reg [NUM_WIDTH  - 1:0] A24_reg ;
reg [NUM_WIDTH  - 1:0] N_reg   ;
reg [WORD_WIDTH - 1:0] n_reg   ;

// Internal FSM signals
reg [`FSM_STATE_WIDTH - 1:0] fsm_state;

// Temp. signals for stages 1-5
wire [NUM_WIDTH - 1:0] t1;
wire [NUM_WIDTH - 1:0] t2;
wire [NUM_WIDTH - 1:0] t3;

reg  [NUM_WIDTH - 1:0] t4;

wire [NUM_WIDTH - 1:0] t5;

// Squares of t1 and t2
reg  [NUM_WIDTH - 1:0] t1_sq;
reg  [NUM_WIDTH - 1:0] t2_sq;

// Preliminary values of 2P
reg  [NUM_WIDTH - 1:0] X_2;
reg  [NUM_WIDTH - 1:0] Z_2;


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
                     ({`FSM_STATE_WIDTH {fsm_state == `STAGE_6}} & {`FSM_STATE_WIDTH {~mm_ready[0]}} & `STAGE_6)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `STAGE_6}} & {`FSM_STATE_WIDTH { mm_ready[0]}} & `DONE   )  |
                     ({`FSM_STATE_WIDTH {fsm_state == `DONE   }} & `WAIT   )  |
                     ({`FSM_STATE_WIDTH {fsm_state == `WAIT   }} & {`FSM_STATE_WIDTH {~start_mul}} & `WAIT) |
                     ({`FSM_STATE_WIDTH {fsm_state == `WAIT   }} & {`FSM_STATE_WIDTH { start_mul}} & `INIT) ;
    end
end

// Control signals
assign
    busy  = (fsm_state >= `INIT) & (fsm_state <= `STAGE_6),
    ready = fsm_state == `DONE;

// Fixing input data (initial state)
always @(posedge clk) begin
    if(rst) begin
        X_in_reg <= {NUM_WIDTH  {1'b0}};
        Z_in_reg <= {NUM_WIDTH  {1'b0}};
        A24_reg  <= {NUM_WIDTH  {1'b0}};
        N_reg    <= {NUM_WIDTH  {1'b0}};
        n_reg    <= {WORD_WIDTH {1'b0}};
    end

    else begin
        X_in_reg <= fsm_state == `INIT ? X_in : X_in_reg;
        Z_in_reg <= fsm_state == `INIT ? Z_in : Z_in_reg;
        A24_reg  <= fsm_state == `INIT ? A24  : A24_reg ;
        N_reg    <= fsm_state == `INIT ? N    : N_reg   ;
        n_reg    <= fsm_state == `INIT ? n    : n_reg   ;
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
                   fsm_state == `STAGE_3 ? t1_sq :
                   fsm_state == `STAGE_5 ? t3    : mm_A[0];
        mm_B[0] <= fsm_state == `STAGE_1 ? t1    :
                   fsm_state == `STAGE_3 ? t2_sq :
                   fsm_state == `STAGE_5 ? t5    : mm_B[0];

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
        mm_A[1] <= fsm_state == `STAGE_1 ? t2      :
                   fsm_state == `STAGE_3 ? A24_reg : mm_A[1];
        mm_B[1] <= fsm_state == `STAGE_1 ? t2      :
                   fsm_state == `STAGE_3 ? t3      : mm_B[1];

        mm_start[1] <= (fsm_state == `STAGE_1) | (fsm_state == `STAGE_3);
    end
end

// Stage 1
assign
    as_A[0] = X_in_reg,
    as_B[0] = Z_in_reg;

assign
    t1 = dif_N[0],
    t2 = sum_N[0];

// Stage 2 (logic for t1^2 and t2^2)
always @(posedge clk) begin
    if(rst) begin
        t1_sq <= {NUM_WIDTH {1'b0}};
        t2_sq <= {NUM_WIDTH {1'b0}};
    end

    else begin
        t1_sq <=  fsm_state == `INIT                   ? {NUM_WIDTH {1'b0}} :
                 (fsm_state == `STAGE_2) & (&mm_ready) ? mm_P[0]            : t1_sq;
        t2_sq <=  fsm_state == `INIT                   ? {NUM_WIDTH {1'b0}} :
                 (fsm_state == `STAGE_2) & (&mm_ready) ? mm_P[1]            : t2_sq;
    end
end

// Stage 3
assign
    as_A[1] = t2_sq,
    as_B[1] = t1_sq;

assign
    t3 = dif_N[1];

// Stage 4 (logic for X_2 and t4)
always @(posedge clk) begin
    if(rst) begin
        X_2 <= {NUM_WIDTH {1'b0}};
        t4  <= {NUM_WIDTH {1'b0}};
    end

    else begin
        X_2 <=  fsm_state == `INIT                   ? {NUM_WIDTH {1'b0}} :
               (fsm_state == `STAGE_4) & (&mm_ready) ? mm_P[0]            : X_2;
        t4  <=  fsm_state == `INIT                   ? {NUM_WIDTH {1'b0}} :
               (fsm_state == `STAGE_4) & (&mm_ready) ? mm_P[1]            : t4 ;
    end
end

// Stage 5
assign
    as_A[2] = t1_sq,
    as_B[2] = t4   ;

assign
    t5 = sum_N[2];

// Stage 6 (logic for Z_2)
always @(posedge clk) begin
    if(rst) begin
        Z_2 <= {NUM_WIDTH {1'b0}};
    end

    else begin
        Z_2 <=  fsm_state == `INIT                   ? {NUM_WIDTH {1'b0}} :
               (fsm_state == `STAGE_6) & mm_ready[0] ? mm_P[0]            : Z_2;
    end
end

//Done stage
/*
always @(posedge clk) begin
    if(rst) begin
        X_out <= {NUM_WIDTH {1'b0}};
        Z_out <= {NUM_WIDTH {1'b0}};
    end

    else begin
        X_out <= fsm_state == `DONE ? X_2 : X_out;
        Z_out <= fsm_state == `DONE ? Z_2 : Z_out;
    end
end
*/
assign
    X_out = X_2,
    Z_out = Z_2;


endmodule
