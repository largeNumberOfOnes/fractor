// Defines for FSM
`define FSM_STATE_WIDTH 4
`define WAIT      0
`define INIT      1 // R_0 = P, R_1 = 2P
`define INIT_WAIT 2 // waiting for 2P
`define FETCH     3 // get k from SRAM
`define BIT_i_1   4 // 0 -> R_1 = R_0 + R_1; 1 -> R_0 = R_0 + R_1
`define BIT_i_2   5 // waiting for new R_0 / R_1
`define BIT_i_3   6 // 0 -> R_0 = 2R_0; 1 -> R_1 = 2R_1
`define BIT_i_4   7 // waiting for new R_0 / R_1
`define FIXING    8
`define DONE      9

module mont_ladder
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

    // Signals for k control
    input  wire             k_bit ,
    input  wire             k_val ,
    input  wire             k_last,
    output wire             k_req ,

    // Input data
    input  wire [NUM_WIDTH -1:0] X_P,
    input  wire [NUM_WIDTH -1:0] Z_P,
    input  wire [NUM_WIDTH -1:0] A24,
    input  wire [NUM_WIDTH -1:0] N  ,
    input  wire [WORD_WIDTH-1:0] n  ,  // n = -N^{-1} mod 2^32

    // Output data
    output reg  [NUM_WIDTH -1:0] X_out,
    output reg  [NUM_WIDTH -1:0] Z_out
);

// Signals for xADD
reg  pa_start;
wire pa_busy ;
wire pa_ready;

wire [NUM_WIDTH - 1:0] pa_X_P;
wire [NUM_WIDTH - 1:0] pa_Z_P;
wire [NUM_WIDTH - 1:0] pa_X_Q;
wire [NUM_WIDTH - 1:0] pa_Z_Q;
wire [NUM_WIDTH - 1:0] pa_X_dif_PQ;
wire [NUM_WIDTH - 1:0] pa_Z_dif_PQ;
wire [NUM_WIDTH - 1:0] pa_X_out;
wire [NUM_WIDTH - 1:0] pa_Z_out;

// Signals for xDBL
reg  pd_start;
wire pd_busy ;
wire pd_ready;

reg  [NUM_WIDTH - 1:0] pd_X_in;
reg  [NUM_WIDTH - 1:0] pd_Z_in;
wire [NUM_WIDTH - 1:0] pd_X_out;
wire [NUM_WIDTH - 1:0] pd_Z_out;

// Copies of input data signals
reg [NUM_WIDTH  - 1:0] X_P_reg;
reg [NUM_WIDTH  - 1:0] Z_P_reg;
reg [NUM_WIDTH  - 1:0] A24_reg;
reg [NUM_WIDTH  - 1:0] N_reg  ;
reg [WORD_WIDTH - 1:0] n_reg  ;

// Internal FSM signals
reg [`FSM_STATE_WIDTH - 1:0] fsm_state;

// Current bit of k
reg                          curr_k_bit;

// End of k bits
reg                          k_bit_last;

// Ready signal accumulator
// reg step_ready;

// Iterated variables
reg [NUM_WIDTH - 1:0] R_0_x;
reg [NUM_WIDTH - 1:0] R_0_z;

reg [NUM_WIDTH - 1:0] R_1_x;
reg [NUM_WIDTH - 1:0] R_1_z;

point_add #(
                    .NUM_WIDTH (NUM_WIDTH ),
                    .WORD_WIDTH(WORD_WIDTH)
            )
            xADD (
                    .clk      (clk),
                    .rst      (rst),

                    .start_add(pa_start   ),
                    .busy     (pa_busy    ),
                    .ready    (pa_ready   ),
                    .X_P      (pa_X_P     ),
                    .Z_P      (pa_Z_P     ),
                    .X_Q      (pa_X_Q     ),
                    .Z_Q      (pa_Z_Q     ),
                    .X_dif_PQ (pa_X_dif_PQ),
                    .Z_dif_PQ (pa_Z_dif_PQ),
                    .N        (N_reg      ),
                    .n        (n_reg      ),
                    .X_out    (pa_X_out   ),
                    .Z_out    (pa_Z_out   )
            );

point_double #(
                    .NUM_WIDTH (NUM_WIDTH ),
                    .WORD_WIDTH(WORD_WIDTH)
              )
              xDBL (
                    .clk      (clk),
                    .rst      (rst),

                    .start_mul(pd_start),
                    .busy     (pd_busy ),
                    .ready    (pd_ready),
                    .X_in     (pd_X_in ),
                    .Z_in     (pd_Z_in ),
                    .A24      (A24_reg ),
                    .N        (N_reg   ),
                    .n        (n_reg   ),
                    .X_out    (pd_X_out),
                    .Z_out    (pd_Z_out)
              );

// Logic for FSM states
always @(posedge clk) begin
    if(rst) begin
        fsm_state <= `WAIT;
    end

    else begin
        fsm_state <= ({`FSM_STATE_WIDTH {fsm_state == `INIT     }} & `INIT_WAIT  ) |
                     ({`FSM_STATE_WIDTH {fsm_state == `INIT_WAIT}} & {`FSM_STATE_WIDTH {~pd_ready}} & `INIT_WAIT) |
                     ({`FSM_STATE_WIDTH {fsm_state == `INIT_WAIT}} & {`FSM_STATE_WIDTH { pd_ready}} & `FETCH    ) |
                     ({`FSM_STATE_WIDTH {fsm_state == `FETCH    }} & {`FSM_STATE_WIDTH {~k_val   }} & `FETCH    ) |
                     ({`FSM_STATE_WIDTH {fsm_state == `FETCH    }} & {`FSM_STATE_WIDTH { k_val   }} & `BIT_i_1  ) |
                     ({`FSM_STATE_WIDTH {fsm_state == `BIT_i_1  }} &                                  `BIT_i_2) |
                     ({`FSM_STATE_WIDTH {fsm_state == `BIT_i_2  }} & {`FSM_STATE_WIDTH {~pa_ready}} & `BIT_i_2) |
                     ({`FSM_STATE_WIDTH {fsm_state == `BIT_i_2  }} & {`FSM_STATE_WIDTH { pa_ready}} & `BIT_i_3) |
                     ({`FSM_STATE_WIDTH {fsm_state == `BIT_i_3  }}                                  & `BIT_i_4) |
                     ({`FSM_STATE_WIDTH {fsm_state == `BIT_i_4  }} & {`FSM_STATE_WIDTH {~pd_ready}} & `BIT_i_4) |
                     ({`FSM_STATE_WIDTH {fsm_state == `BIT_i_4  }} & {`FSM_STATE_WIDTH { pd_ready}} & {`FSM_STATE_WIDTH {~k_bit_last}} & `FETCH ) |
                     ({`FSM_STATE_WIDTH {fsm_state == `BIT_i_4  }} & {`FSM_STATE_WIDTH { pd_ready}} & {`FSM_STATE_WIDTH { k_bit_last}} & `FIXING) |
                     ({`FSM_STATE_WIDTH {fsm_state == `FIXING   }}                                   & `DONE) |
							({`FSM_STATE_WIDTH {fsm_state == `DONE     }}                                   & `WAIT) |
                     ({`FSM_STATE_WIDTH {fsm_state == `WAIT     }} & {`FSM_STATE_WIDTH {~start_add}} & `WAIT) |
                     ({`FSM_STATE_WIDTH {fsm_state == `WAIT     }} & {`FSM_STATE_WIDTH { start_add}} & `INIT) ;
    end
end

// Control signals
assign
    busy  = (fsm_state >= `INIT) & (fsm_state <= `BIT_i_4),
    ready = fsm_state == `DONE;

// Fixing input data (initial state)
always @(posedge clk) begin
    if(rst) begin
        X_P_reg <= {NUM_WIDTH  {1'b0}};
        Z_P_reg <= {NUM_WIDTH  {1'b0}};
        A24_reg <= {NUM_WIDTH  {1'b0}};
        N_reg   <= {NUM_WIDTH  {1'b0}};
        n_reg   <= {WORD_WIDTH {1'b0}};
    end

    else begin
        X_P_reg <= fsm_state == `INIT ? X_P : X_P_reg;
        Z_P_reg <= fsm_state == `INIT ? Z_P : Z_P_reg;
        A24_reg <= fsm_state == `INIT ? A24 : A24_reg;
        N_reg   <= fsm_state == `INIT ? N   : N_reg  ;
        n_reg   <= fsm_state == `INIT ? n   : n_reg  ;
    end
end

// Logic for request of new k bits
assign
    k_req = (fsm_state == `FETCH);


// Logic for current k bit and signal of the end of k
always @(posedge clk) begin
    if(rst) begin
        curr_k_bit <= 1'b0;
        k_bit_last <= 1'b0;
    end

    else begin
        curr_k_bit <= (fsm_state == `FETCH) & k_val ? k_bit  : curr_k_bit;
        k_bit_last <= (fsm_state == `FETCH) & k_val ? k_last : k_bit_last;
    end
end

// Logic for xADD
assign
    pa_X_P = R_0_x,
    pa_Z_P = R_0_z;

assign
    pa_X_Q = R_1_x,
    pa_Z_Q = R_1_z;

assign
    pa_X_dif_PQ = X_P_reg,
    pa_Z_dif_PQ = Z_P_reg;

always @(posedge clk) begin
    if(rst) begin
        pa_start <= 1'b0;
    end

    else begin
        pa_start <= fsm_state == `BIT_i_1;
    end
end

// Logic for xDBL
always @(posedge clk) begin
    if(rst) begin
        pd_X_in  <= 1'b0;
        pd_Z_in  <= 1'b0;

        pd_start <= 1'b0;
    end

    else begin
        pd_X_in <= fsm_state == `INIT    ?                                      X_P :
                   fsm_state == `BIT_i_3 ? {NUM_WIDTH {curr_k_bit == 1'b0}} & R_0_x |
                                           {NUM_WIDTH {curr_k_bit == 1'b1}} & R_1_x : pd_X_in;
        pd_Z_in <= fsm_state == `INIT    ?                                      Z_P :
                   fsm_state == `BIT_i_3 ? {NUM_WIDTH {curr_k_bit == 1'b0}} & R_0_z |
                                           {NUM_WIDTH {curr_k_bit == 1'b1}} & R_1_z : pd_Z_in;

        pd_start <= (fsm_state == `BIT_i_3) | (fsm_state == `INIT);
    end
end

// Logic for R_0 and R_1
always @(posedge clk) begin
    if(rst) begin
        R_0_x <= {NUM_WIDTH {1'b0}};
        R_0_z <= {NUM_WIDTH {1'b0}};

        R_1_x <= {NUM_WIDTH {1'b0}};
        R_1_z <= {NUM_WIDTH {1'b0}};
    end

    else begin
        R_0_x <=  fsm_state == `INIT                                      ? X_P_reg  :
                 (fsm_state == `BIT_i_2) & pa_ready & (curr_k_bit == 1'b1)? pa_X_out :
                 (fsm_state == `BIT_i_4) & pd_ready & (curr_k_bit == 1'b0)? pd_X_out : R_0_x;
        R_0_z <=  fsm_state == `INIT                                      ? Z_P_reg  :
                 (fsm_state == `BIT_i_2) & pa_ready & (curr_k_bit == 1'b1)? pa_Z_out :
                 (fsm_state == `BIT_i_4) & pd_ready & (curr_k_bit == 1'b0)? pd_Z_out : R_0_z;

        R_1_x <= (fsm_state == `INIT_WAIT) & pd_ready                       ? pd_X_out :
                 (fsm_state == `BIT_i_2  ) & pa_ready & (curr_k_bit == 1'b0)? pa_X_out :
                 (fsm_state == `BIT_i_4  ) & pd_ready & (curr_k_bit == 1'b1)? pd_X_out : R_1_x;
        R_1_z <= (fsm_state == `INIT_WAIT) & pd_ready                       ? pd_Z_out :
                 (fsm_state == `BIT_i_2  ) & pa_ready & (curr_k_bit == 1'b0)? pa_Z_out :
                 (fsm_state == `BIT_i_4  ) & pd_ready & (curr_k_bit == 1'b1)? pd_Z_out : R_1_z;
    end
end

// Output signals	 
always @(posedge clk) begin
	if(rst) begin
		X_out <= {NUM_WIDTH {1'b0}};
		Z_out <= {NUM_WIDTH {1'b0}};
	end
	
	else begin
		X_out <= (fsm_state == `FIXING) ? R_0_x : X_out;
		Z_out <= (fsm_state == `FIXING) ? R_0_z : Z_out;
	end
end

endmodule
