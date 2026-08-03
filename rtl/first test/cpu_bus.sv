`default_nettype none
module cpu_bus(
    input logic clk_33mhz,
    input logic button_reset,
    input logic ads_n,
    input logic [31:2] a,
    input logic [3:0] be_n,
    input logic m_io,
    input logic d_c,
    input logic w_r,
    input logic req_valid,
    inout wire [31:0] d,
    output wire cpu_clk,
    output logic rdy_n,
    output logic sys_reset,
    output logic [31:0] address_internal,
    output logic [3:0] be_internal,
    output logic add_decode_sel,
    // output logic rom_selected        # leave this to address decoder to choose and select
    input logic response_ready,
    input logic [31:0] response_data_read,
    input logic decode_hit // included here for switching fsm states, as opposed to thought earlier when i didnt need, THIS IS FROM THE ADDRESS DECODER TO TELL US WHETHER OUR ADDRESS REQUEST IS VALID OR NOT

);

logic [31:0] data_out;
logic [3:0] saved_be_n;
logic [31:0] saved_address;
logic data_output_enable;
logic [1:0] divider;

always_ff @(posedge clk_33mhz or posedge button_reset) begin
    if (button_reset)
        divider <= 2'b00;
    else
        divider <= divider + 1'b1;
end
assign cpu_clk = divider[1];

logic [2:0] current_state;
logic [2:0] next_state;
logic [15:0] cycle_count;

localparam RESET_FULL   = 3'b000;
localparam IDLE         = 3'b001;
localparam REQUEST      = 3'b010;
localparam WAIT_RESPONSE = 3'b011;
localparam ACK_AND_RECV = 3'b100;
localparam SEND         = 3'b101;
localparam CLEANUP      = 3'b110;

logic saved_m_io;
logic saved_d_c;
logic saved_w_r;


// cpu_clk is 33 MHz divided by four, or 8.25 MHz (for now)
always_ff @(posedge clk_33mhz or posedge button_reset) begin
    if (button_reset) begin
        cycle_count <= 16'd0;
        sys_reset   <= 1'b1;
    end
    else if (cycle_count < 16'd33_000) begin
        cycle_count <= cycle_count + 1'b1;
        sys_reset   <= 1'b1;
    end
    else begin
        sys_reset <= 1'b0;
    end
end


// after ADS# is sampled cpu is allowed to change address and control signals for later bus activity
// therefore all fields needed for this transaction must be saved here

always_ff @(posedge cpu_clk or posedge button_reset) begin
    if (button_reset) begin
        current_state <= RESET_FULL;

        saved_address <= 32'd0;
        saved_be_n    <= 4'hf;
        saved_m_io    <= 1'b0;
        saved_d_c     <= 1'b0;
        saved_w_r     <= 1'b0;
        data_out      <= 32'd0;
    end
    else begin
        current_state <= next_state;
        // latch the complete bus request when ADS# is asserted
        // a[31:2] does not include the two byte-address bits so append 2'b00 to create a byte address.
        if ((current_state == IDLE) && !ads_n) begin
            saved_address <= {a[31:2], 2'b00};
            saved_be_n    <= be_n;
            saved_m_io    <= m_io;
            saved_d_c     <= d_c;
            saved_w_r     <= w_r;
        end
        // response_data_read is captured before RDY# is asserted, thus the data is stable for the entire SEND state.
        if ((current_state == WAIT_RESPONSE) && response_ready) begin
            data_out      <= response_data_read;
        end
    end
end



// cpu data bus must be driven only during a read response, at all other times this module releases d[31:0]
assign d = data_output_enable ? data_out : 32'hzzzz_zzzz;


// next state and output logic, aka the part that makes the mealy machine a mealy machine 
// defaults describe the inactive bus state:
//   RDY# inactive = 1
//   data bus released
//   no address decoder request
always_comb begin
    next_state = current_state;
    rdy_n              = 1'b1;
    data_output_enable = 1'b0;
    add_decode_sel     = 1'b0;
    address_internal = saved_address;
    be_internal      = saved_be_n;

    case (current_state)
        RESET_FULL: begin
            // sys_reset itself is generated in the clk_33mhz block
            // stay here until that reset interval has finished
            if (!sys_reset)
                next_state = IDLE;
        end


        IDLE: begin
            if (!ads_n) begin

                next_state = REQUEST;
            end
            else begin
                next_state = IDLE;
            end
        end

        REQUEST: begin
            /*
            priority casez (be_n)
                4'b???0: begin encoded = 2'd0; end
                4'b??01: begin encoded = 2'd1; end
                4'b?011: begin encoded = 2'd2; end
                4'b0111: begin encoded = 2'd3; end
                default: begin encoded = 2'd0; end
            endcase

            address_internal[7:0] <= be_n[0] && 1'b1 ? {a[7:2], encoded[1:0]} : 8'd0;
            address_internal[15:8] <= be_n[1] && 1'b1 ? a[15:8] : 8'd0;
            address_internal[23:16] <= be_n[1] && 1'b1 ? a[23:16] : 8'd0;
            address_internal[31:24] <= be_n[1] && 1'b1 ? a[31:24] : 8'd0;
            */
            // and after i went through the trouble of writing all this logic too..... llms are fucking unreliable for asking literally any question even when you hand them the info on a silver fucking platter
            // fuck my chud life

            // keep the request asserted until the address decoder says it has a valid/selected target
            add_decode_sel = 1'b1;
            if (decode_hit)
                next_state = WAIT_RESPONSE;
            else
                next_state = REQUEST;
        end


        WAIT_RESPONSE: begin
            if (response_ready) begin
                next_state = ACK_AND_RECV;
            end
            else begin
                next_state = WAIT_RESPONSE;
            end
        end


        ACK_AND_RECV: begin
            //response data was registered on the clock edge that moved the fsm out of WAIT_RESPONSE
            next_state = SEND;
        end


        SEND: begin
            // RDY# is active low
            // data_out was capture before entering this state so cpu see stable valid data while RDY# low
            rdy_n              = 1'b0;
            if (!saved_w_r) begin
                data_output_enable = 1'b1;
            end
            next_state = CLEANUP;
        end

        CLEANUP: begin
            rdy_n              = 1'b1;
            data_output_enable = 1'b0;
            if (ads_n) begin
                next_state = IDLE;
            end
        end

        default: begin
            next_state = RESET_FULL;
        end

    endcase

end

endmodule

`default_nettype wire