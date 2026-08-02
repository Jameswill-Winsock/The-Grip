`default_nettype none

module cpu_clock_temp (
    input  logic clk_33mhz,
    input  logic reset,
    output logic cpu_clk
);

    logic [1:0] divider;

    always_ff @(posedge clk_33mhz or posedge reset) begin
        if (reset)
            divider <= 2'b00;
        else
            divider <= divider + 1'b1;
    end

    // 33 mhz / 4 = 8.25 mhz
    assign cpu_clk = divider[1];

endmodule

`default_nettype wire