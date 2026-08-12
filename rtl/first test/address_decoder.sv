`default_nettype none

module address_decoder(
input  logic        clk,
input  logic        reset,

input  logic [31:0] address_internal,
input  logic [3:0]  be_internal_n,
input  logic        add_decode_sel,

inout  wire  [31:0] rom_data,
output logic        rom_enable,
input  logic        rom_ready,

inout  wire  [31:0] ram_data,
output logic        ram_enable,
input  logic        ram_ready,

input  logic        m_io,
input  logic        d_c,
input  logic        w_r,

output logic        bad_request,
output logic        decode_hit,

output logic        response_ready,
output logic [31:0] response_data_read
);

logic [31:0] data_out;
logic [31:0] response_data_latched;

logic [3:0] current_state;
logic [3:0] next_state;

localparam IDLE             = 4'b0000;
localparam REQUEST_IN       = 4'b0001;
localparam REQUEST_ILLEGAL  = 4'b0010;
localparam SEND_RAM         = 4'b0011;
localparam SEND_ROM         = 4'b0100;
localparam WAIT_DECODE_RAM  = 4'b0101;
localparam WAIT_DECODE_ROM  = 4'b0110;
localparam DECODE_HIT_RAM   = 4'b0111;
localparam DECODE_HIT_ROM   = 4'b1000;
localparam CONFIRM_HIT      = 4'b1001;
localparam SEND_DATA        = 4'b1010;
localparam CLEANUP          = 4'b1011;

// for writes later
// rn neither rom nor ram needs the decoder to drive anything because we're only implementing ram reads
assign ram_data = ram_enable && !w_r ? 32'hzzzz_zzzz : 32'hzzzz_zzzz;

assign rom_data = 32'hzzzz_zzzz;

//state register
always_ff @(posedge clk or posedge reset) begin
    if (reset) begin
        current_state <= IDLE;
        response_data_latched <= 32'h0000_0000;
    end
    else begin
        current_state <= next_state;

        if ((current_state == WAIT_DECODE_RAM) && ram_ready)
            response_data_latched <= ram_data;

        if ((current_state == WAIT_DECODE_ROM) && rom_ready)
            response_data_latched <= rom_data;
    end
end


always_comb begin
next_state          = current_state;
decode_hit          = 1'b0;
bad_request         = 1'b0;
rom_enable          = 1'b0;
ram_enable          = 1'b0;
response_ready      = 1'b0;
response_data_read  = 32'h0000_0000;

case (current_state)

    // wait for cpu_bus to present a request
    IDLE: begin
        if (add_decode_sel)
            next_state = REQUEST_IN;
    end

    // determine which device owns the address
    REQUEST_IN: begin
        // for now we're only decoding MEMORY cycles.
        // M/IO = 1 means memory access
        if (!m_io) begin
            next_state = REQUEST_ILLEGAL;
        end

        // conventional ram
        // 00000000 - 0009FFFF
        else if (
            address_internal >= 32'h0000_0000 &&
            address_internal <= 32'h0009_FFFF
        ) begin
            next_state = SEND_RAM;
        end

        // bios low mirror
        // 000F0000 - 000FFFFF
        else if (
            address_internal >= 32'h000F_0000 &&
            address_internal <= 32'h000F_FFFF
        ) begin
            next_state = SEND_ROM;
        end

        // extended RAM
        // 00100000 - 03FFFFFF
        else if (
            address_internal >= 32'h0010_0000 &&
            address_internal <= 32'h03FF_FFFF
        ) begin
            next_state = SEND_RAM;
        end
        
        // bios high mirror
        // FFFF0000 - FFFFFFFF
        else if (
            address_internal >= 32'hFFFF_0000 &&
            address_internal <= 32'hFFFF_FFFF
        ) begin
            next_state = SEND_ROM;
        end
        
        // anything else is currently unmapped
        else begin
            next_state = REQUEST_ILLEGAL;
        end
    end

    // unmapped access
    // complete the transaction instead of hanging the cpu
    REQUEST_ILLEGAL: begin
        bad_request = 1'b1;
        decode_hit = 1'b1;
        next_state = SEND_DATA;
    end

    // select ram
    SEND_RAM: begin
        ram_enable = 1'b1;
        decode_hit = 1'b1;
        next_state = WAIT_DECODE_RAM;
    end

    // wait for fake ram/sdram controller to finish its request
    WAIT_DECODE_RAM: begin
        ram_enable = 1'b1;

        if (ram_ready)
            next_state = DECODE_HIT_RAM;
    end

    DECODE_HIT_RAM: begin
        response_data_read = response_data_latched;
        response_ready = 1'b1;
        next_state = CLEANUP;
    end

    // sel rom
    SEND_ROM: begin
        rom_enable = 1'b1;
        decode_hit = 1'b1;
        next_state = WAIT_DECODE_ROM;
    end

    WAIT_DECODE_ROM: begin
        rom_enable = 1'b1;

        if (rom_ready)
            next_state = DECODE_HIT_ROM;
    end

    DECODE_HIT_ROM: begin
        response_data_read = response_data_latched;
        response_ready = 1'b1;
        next_state = CLEANUP;
    end

    SEND_DATA: begin
        bad_request = 1'b1;
        response_data_read = 32'hFFFF_FFFF;
        response_ready = 1'b1;
        next_state = CLEANUP;
    end

    CLEANUP: begin
        // wait until cpu_bus removes its request before allowing another transaction
        if (!add_decode_sel)
            next_state = IDLE;
    end

    default: begin
        next_state = IDLE;
    end
endcase
end

endmodule

`default_nettype wire