`timescale 1ns/1ps
`default_nettype none

module cpu_bus_tb;

    logic        clk_33mhz;
    logic        button_reset;

    logic        ads_n;
    logic [31:2] a;
    logic [3:0]  be_n;
    logic        m_io;
    logic        d_c;
    logic        w_r;

    logic        response_ready;
    logic [31:0] response_data_read;
    logic        req_valid;


    wire         cpu_clk;
    wire         rdy_n;
    wire         sys_reset;

    wire [31:0]  address_internal;
    wire [3:0]   be_internal;
    wire         add_decode_sel;


    tri [31:0] d;

    // simulated a0486 cpu can drive this during write transactions. remain disable during first read test.
    logic [31:0] cpu_data_out;
    logic        cpu_data_oe;

    assign d = cpu_data_oe
             ? cpu_data_out
             : 32'hZZZZ_ZZZZ;

    // 33 mhz input clock
    initial begin
        clk_33mhz = 1'b0;

        // approx 33 mhz, full period ≈ 30.303 ns
        forever #15.1515 clk_33mhz = ~clk_33mhz;
    end

    cpu_bus dut (
        .clk_33mhz         (clk_33mhz),
        .button_reset      (button_reset),

        .ads_n             (ads_n),
        .a                 (a),
        .be_n              (be_n),
        .m_io              (m_io),
        .d_c               (d_c),
        .w_r               (w_r),

        .d                 (d),

        .cpu_clk           (cpu_clk),
        .rdy_n             (rdy_n),
        .sys_reset         (sys_reset),

        .address_internal  (address_internal),
        .be_internal       (be_internal),
        .add_decode_sel    (add_decode_sel),

        .response_ready    (response_ready),
        .response_data_read(response_data_read),
        .decode_hit        (req_valid)
    );

    // generate one cpu read transaction
    task automatic cpu_read_request (
        input logic [31:0] address,
        input logic [3:0]  byte_enables_n
    );
        begin
            /*
            // change cpu output on falling edge so they stable before dut sample them on rising edge
             */

            @(negedge cpu_clk);

            a     = address[31:2];
            be_n  = byte_enables_n;

            m_io  = 1'b1;  // memory or io transaction (here memory)
            d_c   = 1'b0;  // data or code/control transaction (here c/c)
            w_r   = 1'b0;  // write/read transaction (here read)

            ads_n = 1'b0;

            // ADS# remain assert for one cpu clock.
            @(negedge cpu_clk);
            ads_n = 1'b1;
        end
    endtask

    // main test
    initial begin
        // safe init cpu bus state
        button_reset       = 1'b1;

        ads_n              = 1'b1;
        a                  = '0;
        be_n               = 4'b1111;

        m_io               = 1'b0;
        d_c                = 1'b0;
        w_r                = 1'b0;

        cpu_data_out       = 32'h0000_0000;
        cpu_data_oe        = 1'b0;

        req_valid          = 1'b0;
        response_ready     = 1'b0;
        response_data_read = 32'h0000_0000;

        // gen waveform
        $dumpfile("cpu_bus.vcd");
        $dumpvars(0, cpu_bus_tb);

        // hold fpga side reset button for small time
        repeat (4) @(posedge clk_33mhz);
        button_reset = 1'b0;

        $display("[%0t] waiting for 486 reset to finish", $time);

        // dut internally wait approx 1 ms.
        wait (sys_reset === 1'b0);

        $display("[%0t] 486 RESET release", $time);

        // give ze fsm enough CPU clock edges to move RESET_FULL -> IDLE
        repeat (2) @(posedge cpu_clk);

        // cpu request the reset vector
        $display("[%0t] cpu request FFFFFFF0.", $time);

        cpu_read_request(
            32'hFFFF_FFF0,
            4'b0000
        );

        // wait for bus controller to present saved request to address decoder
        wait (add_decode_sel === 1'b1);

        $display(
            "[%0t] decoder request: address=%08h BE#=%b",
            $time,
            address_internal,
            be_internal
        );

        // check if cpu request was capture correct
        if (address_internal !== 32'hFFFF_FFF0) begin
            $fatal(
                1,
                "wrong internal address: expected FFFFFFF0, got %08h",
                address_internal
            );
        end

        if (be_internal !== 4'b0000) begin
            $fatal(
                1,
                "wrong byte enables: expected 0000, got %b",
                be_internal
            );
        end

        // simulated address decoder say "yes you have come to correct house"
        @(negedge cpu_clk);
        req_valid = 1'b1;

        @(negedge cpu_clk);
        req_valid = 1'b0;

        // simulate some rom latency because no flash chip move fast like flash. even the flash is limit by speed of light.
        repeat (2) @(posedge cpu_clk);

        // boot rom return four NOP byte for now (will change it to proper program)
        @(negedge cpu_clk);

        response_data_read = 32'h9090_9090;
        response_ready     = 1'b1;

        @(negedge cpu_clk);
        response_ready     = 1'b0;

        /*
        // wait for fpga to assert RDY#
         */
        wait (rdy_n === 1'b0);

        // small delay let combinational outputs settle
        #1;

        $display(
            "[%0t] RDY# asserted, cpu data bus=%08h",
            $time,
            d
        );

        if (d !== 32'h9090_9090) begin
            $fatal(
                1,
                "wrong cpu read data: expected 90909090, got %08h",
                d
            );
        end

        // cpu sample data on this rising edge while RDY# low
        @(posedge cpu_clk);
        #1;

        // wait for cleanup
        wait (rdy_n === 1'b1);
        repeat (2) @(posedge cpu_clk);

        // after the read completes check if fpga release d31-d0
        if (d !== 32'hZZZZ_ZZZZ) begin
            $fatal(
                1,
                "cpu data bus was not released after transaction: %08h",
                d
            );
        end

        $display("");
        $display("pass, reset-vector transaction complete");
        $display("address: FFFFFFF0");
        $display("data:    90909090");
        $display("");

        #100;
        $finish;
    end

    // timeout protection

    initial begin
        // reset timer itself take approx 1 ms
        #2_000_000;

        $fatal(
            1,
            "timeout, fsm failed to complete transaction"
        );
    end

endmodule

`default_nettype wire