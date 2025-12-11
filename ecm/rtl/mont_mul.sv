// Defines for FSM
`define FSM_STATE_WIDTH 3
`define INIT    1
`define WORD_T1 2
`define WORD_T2 3
`define WORD_T3 4
`define WORD_T4 5
`define REDUCE  6
`define DONE    7
`define WAIT    0

module mont_mul
#(
    parameter NUM_WIDTH  = 256,
    parameter WORD_WIDTH = 32 ,
    parameter WORD_NUM   = NUM_WIDTH / WORD_WIDTH,
    parameter INDX_WIDTH = $clog2(WORD_NUM)
)
(
    input wire                   clk,
    input wire                   rst,

    // Control signals
    input  wire                  start_mul,
    output wire                  busy     ,
    output wire                  ready    ,

    // Input data
    input wire [NUM_WIDTH - 1:0] A,
    input wire [NUM_WIDTH - 1:0] B,
    input wire [NUM_WIDTH - 1:0] N,

    input wire [WORD_WIDTH -1:0] n,  // -N^(-1) (mod 2^32)

    // Output data
    output reg [NUM_WIDTH - 1:0] P
);

genvar Gi;

// Copies of input data signals
reg [WORD_WIDTH - 1:0] A_reg_arr [WORD_NUM - 1:0];

reg [NUM_WIDTH  - 1:0] B_reg;
reg [NUM_WIDTH  - 1:0] N_reg;
reg [WORD_WIDTH - 1:0] n_reg;

// Internal FSM signals
reg [`FSM_STATE_WIDTH - 1:0] fsm_state;

reg [INDX_WIDTH       - 1:0] word_indx;
reg [WORD_WIDTH       - 1:0] a_i      ;

reg [WORD_WIDTH       - 1:0] m    ;
reg [NUM_WIDTH           :0] accum;


// Logic for FSM states
always @(posedge clk) begin
    if(rst) begin
        fsm_state <= `WAIT;
    end

    else begin
        fsm_state <= ({`FSM_STATE_WIDTH {fsm_state == `INIT   }} & `WORD_T1)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `WORD_T1}} & `WORD_T2)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `WORD_T2}} & `WORD_T3)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `WORD_T3}} & `WORD_T4)  |
                     ({`FSM_STATE_WIDTH {fsm_state == `WORD_T4}} & {`FSM_STATE_WIDTH {~(word_indx == {INDX_WIDTH {1'b0}})}} & `WORD_T1) |
                     ({`FSM_STATE_WIDTH {fsm_state == `WORD_T4}} & {`FSM_STATE_WIDTH { (word_indx == {INDX_WIDTH {1'b0}})}} & `REDUCE ) |
                     ({`FSM_STATE_WIDTH {fsm_state == `REDUCE }} & `DONE   )  |
                     ({`FSM_STATE_WIDTH {fsm_state == `DONE   }} & `WAIT   )  |
                     ({`FSM_STATE_WIDTH {fsm_state == `WAIT   }} & {`FSM_STATE_WIDTH {~start_mul}} & `WAIT) |
                     ({`FSM_STATE_WIDTH {fsm_state == `WAIT   }} & {`FSM_STATE_WIDTH { start_mul}} & `INIT) ;
    end
end

// Control signals
assign
    busy  = (fsm_state >= `INIT) & (fsm_state <= `WORD_T4),
    ready = fsm_state == `DONE;

// Fixing input data (initial state)
always @(posedge clk) begin
    if(rst) begin
        B_reg <= {NUM_WIDTH  {1'b0}};
        N_reg <= {NUM_WIDTH  {1'b0}};
        n_reg <= {WORD_WIDTH {1'b0}};
    end

    else begin
        B_reg <= fsm_state == `INIT ? B : B_reg;
        N_reg <= fsm_state == `INIT ? N : N_reg;
        n_reg <= fsm_state == `INIT ? n : n_reg;
    end
end

// Creating array from A
generate
if(1) begin: gen_array_from_A
    for(Gi = 0; Gi < WORD_NUM; Gi = Gi + 1) begin: word_iter
        always @(posedge clk) begin
            if(rst) begin
                A_reg_arr[Gi] <= {WORD_WIDTH {1'b0}};
            end

            else begin
                    A_reg_arr[Gi] <= fsm_state == `INIT ? A[WORD_WIDTH * Gi + WORD_WIDTH - 1 : WORD_WIDTH * Gi] : A_reg_arr[Gi];
            end
        end
    end
end
endgenerate

// Logic for changing word index
always @(posedge clk) begin
    if(rst) begin
        word_indx <= {INDX_WIDTH {1'b0}};
    end

    else begin
        word_indx <= fsm_state == `INIT    ?             1'b1 :
                     fsm_state == `WORD_T4 ? word_indx + 1'b1 : word_indx;
    end
end

// Logic for current word extraction
always @(posedge clk) begin
    if(rst) begin
        a_i <= {WORD_WIDTH {1'b0}};
    end

    else begin
        a_i <= fsm_state == `INIT    ? A[WORD_WIDTH - 1:0]  :
               fsm_state == `WORD_T4 ? A_reg_arr[word_indx] : a_i;
    end
end

// Logic for temporary word
always @(posedge clk) begin
    if(rst) begin
        m <= {WORD_WIDTH {1'b0}};
    end

    else begin
        m <= fsm_state == `WORD_T2 ? accum[WORD_WIDTH - 1:0] * n_reg : m;
    end
end

// Logic for register-accumulator
always @(posedge clk) begin
    if(rst) begin
        accum <= {(NUM_WIDTH + 1) {1'b0}};
    end

    else begin
        accum <= ({(NUM_WIDTH + 1) {fsm_state == `WORD_T1}} & (accum + a_i * B_reg)) |
                 ({(NUM_WIDTH + 1) {fsm_state == `WORD_T2}} &  accum) |
                 ({(NUM_WIDTH + 1) {fsm_state == `WORD_T3}} & (accum +  m[WORD_WIDTH - 1:0] * N_reg)) |
                 ({(NUM_WIDTH + 1) {fsm_state == `WORD_T4}} & (accum >> WORD_WIDTH))    ;
    end
end

// Logic for output signal
always @(posedge clk) begin
    if(rst) begin
        P <= {NUM_WIDTH {1'b0}};
    end

    else begin
        P <= fsm_state == `REDUCE ? (accum[NUM_WIDTH - 1:0] >= N_reg ?
                                     accum[NUM_WIDTH - 1:0] -  N_reg :
                                     accum[NUM_WIDTH - 1:0])         : P;
    end
end

endmodule
