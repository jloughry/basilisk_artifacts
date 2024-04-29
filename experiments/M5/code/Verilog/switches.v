`include "SR_debounce.v"

module debouncer(
  input clock,
  input spst_switch,
  output reg logical_switch_state
);

reg switch_state;
// Synchronize the switch input to the clock
reg PB_sync_0;
always @(posedge clock) PB_sync_0 <= spst_switch;
reg PB_sync_1;
always @(posedge clock) PB_sync_1 <= PB_sync_0;

// Debounce the switch
reg [15:0] PB_cnt;
always @ (posedge clock)
if (switch_state == PB_sync_1)
  PB_cnt <= 0;
else
begin
  PB_cnt <= PB_cnt + 1;
  if(PB_cnt == 8'hff) switch_state <= ~switch_state; 
end

always @ (negedge clock) begin
  logical_switch_state <= !switch_state;
end
endmodule

module switches (
  hwclk,

  reset_switch_high,
  reset_switch_low,

  run_stop_switch_high,
  run_stop_switch_low,

  clock_step_switch_high,
  clock_step_switch_low,

  fast_slow_switch_high,
  fast_slow_switch_low,

  cycle_switch_high,
  cycle_switch_low,

  aux_1_switch_high,
  aux_1_switch_low,

  aux_2_switch_high,
  aux_2_switch_low,
);

input hwclk;

input reset_switch_high;
input reset_switch_low;

input run_stop_switch_high;
input run_stop_switch_low;

input clock_step_switch_high;
input clock_step_switch_low;

input fast_slow_switch_high;
input fast_slow_switch_low;

input cycle_switch_high;
input cycle_switch_low;

input aux_1_switch_high;
input aux_1_switch_low;

input aux_2_switch_high;
input aux_2_switch_low;

wire [1:0] reset_switch_wires;
wire [1:0] run_stop_switch_wires;
wire [1:0] clock_step_switch_wires;
wire [1:0] fast_slow_switch_wires;
wire [1:0] cycle_switch_wires;
wire [1:0] aux_1_switch_wires;
wire [1:0] aux_2_switch_wires;

SB_IO #(
  .PIN_TYPE(6'b 0000_01),
  .PULLUP(1'b 1)
) reset_switch_wires[1:0] (
  .PACKAGE_PIN({reset_switch_high, reset_switch_low}),
  .D_IN_0(reset_switch_wires)
);

SB_IO #(
  .PIN_TYPE(6'b 0000_01),
  .PULLUP(1'b 1)
) run_stop_switch_wires[1:0] (
  .PACKAGE_PIN({run_stop_switch_high, run_stop_switch_low}),
  .D_IN_0(run_stop_switch_wires)
);

SB_IO #(
  .PIN_TYPE(6'b 0000_01),
  .PULLUP(1'b 1)
) clock_step_switch_wires[1:0] (
  .PACKAGE_PIN({clock_step_switch_high, clock_step_switch_low}),
  .D_IN_0(clock_step_switch_wires)
);

SB_IO #(
  .PIN_TYPE(6'b 0000_01),
  .PULLUP(1'b 1)
) fast_slow_switch_wires[1:0] (
  .PACKAGE_PIN({fast_slow_switch_high, fast_slow_switch_low}),
  .D_IN_0(fast_slow_switch_wires)
);

SB_IO #(
  .PIN_TYPE(6'b 0000_01),
  .PULLUP(1'b 1)
) cycle_switch_wires[1:0] (
  .PACKAGE_PIN({cycle_switch_high, cycle_switch_low}),
  .D_IN_0(cycle_switch_wires)
);

SB_IO #(
  .PIN_TYPE(6'b 0000_01),
  .PULLUP(1'b 1)
) aux_1_switch_wires[1:0] (
  .PACKAGE_PIN({aux_1_switch_high, aux_1_switch_low}),
  .D_IN_0(aux_1_switch_wires)
);

SB_IO #(
  .PIN_TYPE(6'b 0000_01),
  .PULLUP(1'b 1)
) aux_2_switch_wires[1:0] (
  .PACKAGE_PIN({aux_2_switch_high, aux_2_switch_low}),
  .D_IN_0(aux_2_switch_wires)
);

wire reset, run, clock_step, fast_slow, cycle;
wire aux_1, aux_2;

SR_debounce reset_db (reset_switch_wires[0], reset_switch_wires[1], reset);
SR_debounce run_db (run_stop_switch_wires[0], run_stop_switch_wires[1], run);
SR_debounce clock_step_db (clock_step_switch_wires[0], clock_step_switch_wires[1],
  clock_step);
SR_debounce fast_slow_db (fast_slow_switch_wires[0], fast_slow_switch_wires[1],
  fast_slow);
SR_debounce cycle_db (cycle_switch_wires[0], cycle_switch_wires[1], cycle);
SR_debounce aux1_db (aux_1_switch_wires[0], aux_1_switch_wires[1], aux_1);
SR_debounce aux2_db (aux_2_switch_wires[0], aux_2_switch_wires[1], aux_2);

wire cycle_db2;
debouncer cycle_switch_db (clk_32_kHz, cycle_switch_wires[0], cycle_db2);

/* 32.768 kHz clock (from 12.0000 MHz on the Lattice board) */
reg clk_32_kHz = 0;
reg [8:0] ctr_32_kHz = 0;
// parameter period_32_kHz = 92;
parameter period_32_kHz = 184;

always @ (posedge hwclk) begin
  ctr_32_kHz <= ctr_32_kHz + 1;
  if (ctr_32_kHz == period_32_kHz) begin
    clk_32_kHz <= ~clk_32_kHz;
    ctr_32_kHz <= 0;
  end
end

/* 1 Hz clock (useful for testing whether 32 kHz is sane w/o oscilloscope) */
reg clk_1_Hz = 0;
reg [15:0] ctr_1_Hz = 0;
parameter period_1_Hz = 32768;

always @ (posedge clk_32_kHz) begin
  ctr_1_Hz <= ctr_1_Hz + 1;
  if (ctr_1_Hz == period_1_Hz) begin
    clk_1_Hz <= ~clk_1_Hz;
    ctr_1_Hz <= 0;
  end
end

/* switchable clock to use everywhere */
reg clock;

always @ (posedge hwclk) begin
  if (clock_step && fast_slow) begin
    clock <= clk_32_kHz;
  end else begin
    if (clock_step && !fast_slow) begin
      clock <= clk_1_Hz;
    end else begin
      if (!clock_step) begin
        if (cycle_db2) begin // I borrowed this temporarily; // if (cycle) begin
          clock <= 1;
        end else begin
          clock <= 0;
        end
      end
    end
  end
end

endmodule
