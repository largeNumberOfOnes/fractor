module ecm_real_sram_top (
    input  wire        CLOCK_50,
    input  wire [0:0]  KEY  ,      // KEY[0] - Reset (Active Low)
    input  wire [0:0]  KEY_1,       // SW[0] - Start Trigger
	 
	 output wire        rx,
	 output wire        tx,
    
    // SRAM Interface (оставляем заглушки, чтобы квартус не ругался на пины)
    inout  wire [15:0] SRAM_DQ,
    output wire [19:0] SRAM_ADDR,
    output wire        SRAM_LB_N,
    output wire        SRAM_UB_N,
    output wire        SRAM_CE_N,
    output wire        SRAM_OE_N,
    output wire        SRAM_WE_N,
    
    // Indicators
    output wire [8:0]  LEDG,
    output wire [17:0] LEDR,
    output wire [6:0]  HEX0,
    output wire [6:0]  HEX1,
    output wire [6:0]  HEX2,
    output wire [6:0]  HEX3
);

    // --- Clock generation ---
    //reg [4:0] clk_cnt;
    //always @(posedge CLOCK_50) begin
    //    clk_cnt <= clk_cnt + 1;
    //end
    wire sys_clk = CLOCK_50; 

    // --- System Signals ---
    wire sys_rst = ~KEY[0]; // Active High Reset
    wire start_btn;

    syn_button start_button (
                                .clock   (sys_clk  ),
                                .reset   (KEY  [0] ),
                                .button  (KEY_1[0] ),
                                .pressing(start_btn)
    );

    // --- State Machine ---
    localparam S_IDLE    = 0;
    localparam S_COMPUTE = 1;
    localparam S_DONE    = 2;
    
    reg [1:0] state;
    reg       start_pulse;

    always @(posedge sys_clk or posedge sys_rst) begin
        if (sys_rst) begin
            state <= S_IDLE;
            start_pulse <= 0;
        end else begin
            start_pulse <= 0;
            
            case (state)
                S_IDLE: begin
                    if (start_btn) begin
                        state <= S_COMPUTE;
                        start_pulse <= 1; // Даем пинок лестнице
                    end
                end

                S_COMPUTE: begin
                    // Ждем, пока лестница скажет ready (ladder_done)
                    if (ladder_done) state <= S_DONE;
                end
                
                S_DONE: begin
                    // Висим, радуемся
                    if (start_btn == 0) begin
								state <= (cnt == 127) ? S_IDLE : S_DONE;
						  end
					 end
            endcase
        end
    end

    // =================================================================
    // ЗАМЕНА BROADCASTER (ТЕСТОВЫЙ ГЕНЕРАТОР K)
    // =================================================================
    
    // Задаем тестовый скаляр K = 5 (101 bin).
    // Старший бит (MSB) = индекс 2.
    // Последовательность битов будет: 1 -> 0 -> 1 -> STOP
    
    localparam [7:0]   START_BIT_IDX = 8'd2; // Индекс самого старшего бита
    localparam [255:0] MY_TEST_K     = 256'd5; 

    reg [7:0] current_bit_idx;
    reg       gen_k_val;

    // Логика "генератора":
    always @(posedge sys_clk) begin
        if (sys_rst || (state == S_IDLE)) begin
            current_bit_idx <= START_BIT_IDX;
            gen_k_val       <= 1'b0;
        end 
        else begin
            // Если Ладдер просит бит (ladder_req), мы поднимаем Valid на следующем такте
            if (ladder_req) begin
                gen_k_val <= 1'b1;
            end else begin
                gen_k_val <= 1'b0;
            end

            // Смена индекса: если мы отдали валидный бит, готовим следующий
            if (gen_k_val) begin
                if (current_bit_idx > 0) 
                    current_bit_idx <= current_bit_idx - 1'b1;
            end
        end
    end

    // Формируем провода
    wire gen_k_bit;
    wire gen_k_last;

    assign gen_k_bit  = MY_TEST_K[current_bit_idx];
    
    // Сигнал Last поднимается, когда мы отдаем бит с индексом 0
    assign gen_k_last = (current_bit_idx == 0) && gen_k_val;


    // =================================================================
    // ПОДКЛЮЧЕНИЕ ЛЕСТНИЦЫ
    // =================================================================
    
    wire ladder_req; // Это выход из лестницы, вход в наш генератор
    wire ladder_busy, ladder_done;
    wire [255:0] res_X, res_Z;
    
    reg [255:0] saved_X;
    reg [255:0] saved_Z;

    always @(posedge sys_clk) begin
        if (sys_rst) begin
            saved_X <= 0;
            saved_Z <= 0;
        end 
		  
		  else begin
            saved_X <= ladder_done ? res_X : saved_X;
            saved_Z <= ladder_done ? res_Z : saved_Z;
        end
    end


    // Тестовые вектора (те же, что были)
    wire [255:0] test_N   = 256'd23;
    wire [255:0] test_X   = 256'd2;
    wire [255:0] test_Z   = 256'd1;
    wire [255:0] test_A24 = 256'd1;
    wire [31:0]  test_n   = 32'h1642C859;

    mont_ladder #(.NUM_WIDTH(256), .WORD_WIDTH(32)) u_ladder (
        .clk      (sys_clk), 
        .rst      (sys_rst),
        .start_add(start_pulse),   // Запуск от нашего FSM
        .busy     (ladder_busy), 
        .ready    (ladder_done),   // Сигнал готовности
        
        // Подключаем к нашему генератору
        .k_req    (ladder_req),    // Лестница просит
        .k_val    (gen_k_val),     // Мы отвечаем
        .k_bit    (gen_k_bit),     // Значение бита
        .k_last   (gen_k_last),    // Флаг последнего бита
        
        .X_P(test_X), .Z_P(test_Z), .A24(test_A24), .N(test_N), .n(test_n),
        .X_out(res_X), .Z_out(res_Z)
    );

    // =================================================================
    // ИНДИКАЦИЯ И ЗАГЛУШКИ
    // =================================================================

    // Отключаем SRAM (Hi-Z)
    assign SRAM_DQ   = 16'bz;
    assign SRAM_ADDR = 20'b0;
    assign SRAM_WE_N = 1'b1; 
    assign SRAM_OE_N = 1'b1;
    assign SRAM_CE_N = 1'b1;
    assign SRAM_LB_N = 1'b1;
    assign SRAM_UB_N = 1'b1;
	 
	 reg ladder_done_r;
	 
	 always @(posedge sys_clk) begin
		if(sys_rst) begin
			ladder_done_r <= 1'b0;
		end
		
		else begin
			ladder_done_r <= ladder_done ? ladder_done : ladder_done_r;
		end
	 end
	 
	 reg ladder_done_prev;
	 
	 always @(posedge sys_clk) begin
		if(sys_rst) begin
			ladder_done_prev <= 1'b0;
		end
		
		else begin
			ladder_done_prev <= ladder_done;
		end
	 end

    // Светодиоды
    assign LEDG[0] = (state == S_IDLE);
    assign LEDG[1] = ladder_busy;        // Горит, пока считает
    assign LEDG[2] = ladder_done_r;        // Загорится в конце
    assign LEDG[3] = gen_k_val;          // Мигнет 3 раза (передача битов)

    // Debug выходы
    assign LEDR[0] = gen_k_val;
    assign LEDR[1] = gen_k_bit;
    assign LEDR[2] = ladder_req;
    
    // Результат на HEX
    hex_to_seg h0 (.hex(saved_X[3:0]), .s_seg(HEX0));
    hex_to_seg h1 (.hex(saved_X[7:4]), .s_seg(HEX1));
	 hex_to_seg h2 (.hex(saved_Z[3:0]), .s_seg(HEX2));
    hex_to_seg h3 (.hex(saved_Z[7:4]), .s_seg(HEX3));
	 
	 // Recieve
	 reg [7:0] data_in ;
	 reg       data_val;
	 
	 // Send
	 reg [7:0] data_out;
	 reg       send    ;
	 
	 reg [6:0] cnt;
	 
	 wire [559:0] message;
	 
	 assign message = {8'h5, 8'h5, 8'ha, 8'ha, 16'd512, saved_X, saved_Z};
	 
	 wire [6:0] cnt_shft;
	 
	 assign cnt_shft = cnt >> 3;
	 
	 always @(posedge sys_clk) begin
			if(sys_rst) begin
				data_out <= 0 ;
				send     <= 0 ;
				cnt      <= 69;
			end
			
			else begin
				data_out <= (state == S_DONE) ? message[cnt_shft + 7 : cnt_shft] : data_out;
				send     <= (state == S_DONE) ? idle : 1'b0;
				cnt      <= (state == S_DONE) ? cnt - 1'b1 : cnt;
			end
	 end

	 
	 UARTrx RCV (
			.clock     (sys_clk ),
			.reset     (sys_rst ),
			.rx        (rx      ),
			.data      (data_in ),
			.data_valid(data_val)
	 );
	 
	 UARTtx TRS (
			.clock (sys_clk ),
			.reset (sys_rst ),
			.data  (data_out),
			.send  (send    ),
			.tx    (tx      ),
			.idle  (idle    )
	 );

endmodule
