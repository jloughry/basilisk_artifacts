`include "SR_debounce.v"
`include "FF_debounce.v"

module switches_and_lights (run_lamp, clock_lamp, halt_lamp,
  aux_1_lamp, aux_2_lamp,
  acc_led_0, acc_led_1, acc_led_2, acc_led_3,
  hwclk,
  reset_switch_high, reset_switch_low,
  run_stop_switch_high, run_stop_switch_low,
  clock_step_switch_high, clock_step_switch_low,
  fast_slow_switch_high, fast_slow_switch_low,
  cycle_switch_high, cycle_switch_low,
  aux_1_switch_high, aux_1_switch_low,
  aux_2_switch_high, aux_2_switch_low,
  sequencer_mon_3, sequencer_mon_2, sequencer_mon_1, sequencer_mon_0,
  PC_monitor_3, PC_monitor_2, PC_monitor_1, PC_monitor_0,
  IR_monitor_3, IR_monitor_2, IR_monitor_1, IR_monitor_0,
  MAR_monitor_3, MAR_monitor_2, MAR_monitor_1, MAR_monitor_0,
  MDR_monitor_3, MDR_monitor_2, MDR_monitor_1, MDR_monitor_0,
  ALU_multiplexed_mon_3,
  ALU_multiplexed_mon_2,
  ALU_multiplexed_mon_1,
  ALU_multiplexed_mon_0,
  ALU_A_multiplexed_mon_gnd_1, // There is no 2 b/c no pins left on J2
  ALU_control_multiplexed_mon_gnd_3,
  ALU_result_multiplexed_mon_gnd_4,
  bus_conflict_mux_data_3,
  bus_conflict_mux_data_2,
  bus_conflict_mux_data_1,
  bus_conflict_mux_data_0,
  bus_conflict_mux_gnd_1,
  bus_conflict_mux_gnd_2,
  bus_conflict_mux_gnd_3,
  bus_conflict_mux_gnd_4,
  LED0, LED1, LED2, LED3, LED4, LED5, LED6, LED7,
  ACC_0, ACC_1, ACC_2, ACC_3,
  PC_0, PC_1, PC_2, PC_3,
  IR_0, IR_1, IR_2, IR_3,
  MAR_0, MAR_1, MAR_2, MAR_3,
  MDR_0, MDR_1, MDR_2, MDR_3,
  ALU_A_0, ALU_A_1, ALU_A_2, ALU_A_3,
  ALU_B_0, ALU_B_1, ALU_B_2, ALU_B_3,
  ALU_result_0, ALU_result_1, ALU_result_2, ALU_result_3,
);

  output run_lamp, clock_lamp, halt_lamp, aux_1_lamp, aux_2_lamp;
  output acc_led_0, acc_led_1, acc_led_2, acc_led_3;
  input hwclk;
  input reset_switch_high, reset_switch_low;
  input run_stop_switch_high, run_stop_switch_low;
  input clock_step_switch_high, clock_step_switch_low;
  input fast_slow_switch_high, fast_slow_switch_low;
  input cycle_switch_high, cycle_switch_low;
  input aux_1_switch_high, aux_1_switch_low;
  input aux_2_switch_high, aux_2_switch_low;
  output sequencer_mon_3, sequencer_mon_2, sequencer_mon_1, sequencer_mon_0;
  output PC_monitor_3, PC_monitor_2, PC_monitor_1, PC_monitor_0;
  output IR_monitor_3, IR_monitor_2, IR_monitor_1, IR_monitor_0;
  output MAR_monitor_3, MAR_monitor_2, MAR_monitor_1, MAR_monitor_0;
  output MDR_monitor_3, MDR_monitor_2, MDR_monitor_1, MDR_monitor_0;
  output ALU_multiplexed_mon_3;
  output ALU_multiplexed_mon_2;
  output ALU_multiplexed_mon_1;
  output ALU_multiplexed_mon_0;
  output ALU_A_multiplexed_mon_gnd_1; // There is no 2 b/c no pins left on J2
  output ALU_control_multiplexed_mon_gnd_3;
  output ALU_result_multiplexed_mon_gnd_4;
  output bus_conflict_mux_data_3;
  output bus_conflict_mux_data_2;
  output bus_conflict_mux_data_1;
  output bus_conflict_mux_data_0;
  output bus_conflict_mux_gnd_1;
  output bus_conflict_mux_gnd_2;
  output bus_conflict_mux_gnd_3;
  output bus_conflict_mux_gnd_4;
  output LED0, LED1, LED2, LED3, LED4, LED5, LED6, LED7;
  inout ACC_0, ACC_1, ACC_2, ACC_3;
  inout PC_0, PC_1, PC_2, PC_3;
  inout IR_0, IR_1, IR_2, IR_3;
  inout MAR_0, MAR_1, MAR_2, MAR_3;
  inout MDR_0, MDR_1, MDR_2, MDR_3;
  inout ALU_A_0, ALU_A_1, ALU_A_2, ALU_A_3;
  inout ALU_B_0, ALU_B_1, ALU_B_2, ALU_B_3;
  inout ALU_result_0, ALU_result_1, ALU_result_2, ALU_result_3;

  wire mute;

  assign mute = aux_2;

  // Reset is reset when the switch is DOWN.
  wire [1:0] reset_switch_wires;
  SB_IO #(
    .PIN_TYPE(6'b 0000_01),
    .PULLUP(1'b 1)
  ) reset_switch_wires[1:0] (
    .PACKAGE_PIN({reset_switch_low, reset_switch_high}),
    .D_IN_0(reset_switch_wires)
  );
  wire reset;
  wire sr_reset;
  // both kinds of debouncers on reset
  SR_debounce reset_srdb (reset_switch_wires[0], reset_switch_wires[1], sr_reset);
  debouncer reset_ffdb (hwclk, sr_reset, reset);

  wire [1:0] run_stop_switch_wires;
  SB_IO #(
    .PIN_TYPE(6'b 0000_01),
    .PULLUP(1'b 1)
  ) run_stop_switch_wires[1:0] (
    .PACKAGE_PIN({run_stop_switch_high, run_stop_switch_low}),
    .D_IN_0(run_stop_switch_wires)
  );
  wire run_stop;
  // SR_debounce run_stop_srdb (run_stop_switch_wires[0], run_stop_switch_wires[1],
  //  run_stop);
  debouncer run_stop_ffdb (hwclk, run_stop_switch_wires[1], run_stop);

  wire [1:0] clock_step_switch_wires;
  SB_IO #(
    .PIN_TYPE(6'b 0000_01),
    .PULLUP(1'b 1)
  ) clock_step_switch_wires[1:0] (
    .PACKAGE_PIN({clock_step_switch_high, clock_step_switch_low}),
    .D_IN_0(clock_step_switch_wires)
  );
  wire clock_step;
  // SR_debounce clock_step_srdb (clock_step_switch_wires[0],
  //   clock_step_switch_wires[1], clock_step);
  debouncer clock_step_ffdb (hwclk, clock_step_switch_wires[1], clock_step);

  wire [1:0] fast_slow_switch_wires;
  SB_IO #(
    .PIN_TYPE(6'b 0000_01),
    .PULLUP(1'b 1)
  ) fast_slow_switch_wires[1:0] (
    .PACKAGE_PIN({fast_slow_switch_high, fast_slow_switch_low}),
    .D_IN_0(fast_slow_switch_wires)
  );
  wire fast_slow;
  //  SR_debounce fast_slow_srdb (fast_slow_switch_wires[0],
  //    fast_slow_switch_wires[1], fast_slow);
  debouncer fast_slow_ffdb (hwclk, fast_slow_switch_wires[1], fast_slow);

  wire [1:0] cycle_switch_wires;
  SB_IO #(
    .PIN_TYPE(6'b 0000_01),
    .PULLUP(1'b 1)
  ) cycle_switch_wires[1:0] (
    .PACKAGE_PIN({cycle_switch_high, cycle_switch_low}),
    .D_IN_0(cycle_switch_wires)
  );
  // This one has both kinds of debouncer on it.
  wire cycle;
  wire sr_cycle;
  SR_debounce cycle_srdb (cycle_switch_wires[0], cycle_switch_wires[1], sr_cycle);
  debouncer cycle_ffdb (hwclk, sr_cycle, cycle);

  wire [1:0] aux_1_switch_wires;
  SB_IO #(
    .PIN_TYPE(6'b 0000_01),
    .PULLUP(1'b 1)
  ) aux_1_switch_wires[1:0] (
    .PACKAGE_PIN({aux_1_switch_high, aux_1_switch_low}),
    .D_IN_0(aux_1_switch_wires)
  );
  wire aux_1;
  // SR_debounce aux_1_srdb (aux_1_switch_wires[0], aux_1_switch_wires[1], aux_1);
  debouncer aux_1_ffdb (hwclk, aux_1_switch_wires[1], aux_1);

  wire [1:0] aux_2_switch_wires;
  SB_IO #(
    .PIN_TYPE(6'b 0000_01),
    .PULLUP(1'b 1)
  ) aux_2_switch_wires[1:0] (
    .PACKAGE_PIN({aux_2_switch_high, aux_2_switch_low}),
    .D_IN_0(aux_2_switch_wires)
  );
  wire aux_2;
  // SR_debounce aux_2_srdb (aux_2_switch_wires[0], aux_2_switch_wires[1], aux_2);
  debouncer aux_2_ffdb (hwclk, aux_2_switch_wires[1], aux_2);

  /* 32.768 kHz clock (from 12.0000 MHz on the Lattice board) */
  reg clk_32_kHz = 0;
  reg [8:0] ctr_32_kHz = 0;
  parameter period_32_kHz = 366;

  always @ (posedge hwclk) begin
    ctr_32_kHz <= ctr_32_kHz + 1;
    if (ctr_32_kHz == period_32_kHz) begin
      clk_32_kHz <= ~clk_32_kHz;
      ctr_32_kHz <= 0;
    end
  end

  /* 1 kHz clock */
  reg clk_1_kHz = 0;
  reg [31:0] ctr_1_kHz = 0;
  parameter period_1_kHz = 42000; // 12000 gives a pleasing 2 ms period.

  // 60000 gives a 10 ms period.

  always @ (posedge hwclk) begin
    ctr_1_kHz = ctr_1_kHz + 1;
    if (ctr_1_kHz == period_1_kHz) begin
      clk_1_kHz = ~clk_1_kHz;
      ctr_1_kHz = 0;
    end
  end

  /* 1 Hz clock */
  reg clk_1_Hz = 0;
  reg [31:0] ctr_1_Hz = 0;
  parameter period_1_Hz = 100; // was 1000

  always @ (posedge clk_1_kHz) begin
    ctr_1_Hz = ctr_1_Hz + 1;
    if (ctr_1_Hz == period_1_Hz) begin
      clk_1_Hz = ~clk_1_Hz;
      ctr_1_Hz = 0;
    end
  end

  /* Faster clock for visual appeal */
  reg clk_visual = 0;
  reg [31:0] ctr_visual = 0;
  parameter period_visual = 21000;

  always @ (posedge hwclk) begin
    ctr_visual = ctr_visual + 1;
    if (ctr_visual == period_visual) begin
      clk_visual = ~clk_visual;
      ctr_visual = 0;
    end
  end

  /* switchable clock to use everywhere */
  reg clock;

  always @ (posedge hwclk) begin
    if (run_stop) begin
      clock = clk_visual;
    end else begin
      if (clock_step && fast_slow) begin
        clock = clk_1_kHz;
      end else begin
        if (clock_step && !fast_slow) begin
          clock = clk_1_Hz;
        end else begin
          if (!clock_step) begin
            if (cycle) begin
              clock <= 1;
            end else begin
              clock <= 0;
            end
          end
        end
      end
    end
  end

  /****************************/

  reg [3:0] accumulator = 0;

  reg acc_listen = 0;
  reg acc_talk = 0;

  always @ (posedge clock) begin
    if (reset) begin
      accumulator = 0;
    end else if (acc_listen) begin
      accumulator = {ACC_3, ACC_2, ACC_1, ACC_0};
    end
  end

  assign ACC_3 = (acc_talk && accumulator[3]) ? accumulator[3] : 1'bz;
  assign ACC_2 = (acc_talk && accumulator[2]) ? accumulator[2] : 1'bz;
  assign ACC_1 = (acc_talk && accumulator[1]) ? accumulator[1] : 1'bz;
  assign ACC_0 = (acc_talk && accumulator[0]) ? accumulator[0] : 1'bz;

  assign acc_led_0 = accumulator[0];
  assign acc_led_1 = accumulator[1];
  assign acc_led_2 = accumulator[2];
  assign acc_led_3 = accumulator[3];

  /****************************/

  reg running = 1;

  assign run_lamp   = running && turn_signal;
  assign halt_lamp  = !running;
  assign clock_lamp = clock;
  assign aux_1_lamp = aux_1;
  assign aux_2_lamp = aux_2;

  reg turn_signal = 0;
  reg [31:0] turn_signal_timer = 0;
  always @ (posedge hwclk) begin
    if (turn_signal_timer > 12_000_000) begin
      turn_signal = ~turn_signal;
      turn_signal_timer = 0;
    end else begin
      turn_signal_timer = turn_signal_timer + 1;
    end
  end

  /****************************/

  reg [3:0] sequencer = 15; // start it just BEFORE zero
  reg skip_rest_of_sequence = 0;

  always @ (posedge clock) begin
    sequencer = sequencer + 1;
    if (sequencer > 14) begin
      sequencer = 0;
    end
    if (reset) begin
      sequencer = 0;
    end
    if (skip_rest_of_sequence) begin
      sequencer = 0;
    end
  end

  assign sequencer_mon_3 = sequencer[3] && mute;
  assign sequencer_mon_2 = sequencer[2] && mute;
  assign sequencer_mon_1 = sequencer[1] && mute;
  assign sequencer_mon_0 = sequencer[0] && mute;

  /****************************/

  reg [3:0] pc = 0;

  reg pc_listen = 0;
  reg pc_talk = 0;

  always @ (posedge clock) begin
    if (reset) begin
      pc = 0;
    end else if (pc_listen) begin
      pc = {PC_3, PC_2, PC_1, PC_0};
    end
  end

  assign PC_3 = (pc_talk && pc[3]) ? pc[3] : 1'bz;
  assign PC_2 = (pc_talk && pc[2]) ? pc[2] : 1'bz;
  assign PC_1 = (pc_talk && pc[1]) ? pc[1] : 1'bz;
  assign PC_0 = (pc_talk && pc[0]) ? pc[0] : 1'bz;

  assign PC_monitor_0 = pc[0] && mute;
  assign PC_monitor_1 = pc[1] && mute;
  assign PC_monitor_2 = 1; // pc[2] && mute;
  assign PC_monitor_3 = pc[3] && mute;

  /****************************/

  reg [3:0] ir = 0;

  reg ir_listen = 0;
  reg ir_talk = 0;

  always @ (posedge clock) begin
    if (reset) begin
      ir = 0;
    end else if (ir_listen) begin
      ir = {IR_3, IR_2, IR_1, IR_0};
    end
  end

  assign IR_3 = (ir_talk && ir[3]) ? ir[3] : 1'bz;
  assign IR_2 = (ir_talk && ir[2]) ? ir[2] : 1'bz;
  assign IR_1 = (ir_talk && ir[1]) ? ir[1] : 1'bz;
  assign IR_0 = (ir_talk && ir[0]) ? ir[0] : 1'bz;

  assign IR_monitor_0 = ir[0] && mute;
  assign IR_monitor_1 = ir[1] && mute;
  assign IR_monitor_2 = ir[2] && mute;
  assign IR_monitor_3 = ir[3] && mute;

  /****************************/

  reg [3:0] mar = 0;

  reg mar_listen = 0;
  reg mar_talk = 0;

  always @ (posedge clock) begin
    if (reset) begin
      mar = 0;
    end else if (mar_listen) begin
      mar = {MAR_3, MAR_2, MAR_1, MAR_0};
    end
  end

  assign MAR_3 = (mar_talk && mar[3]) ? mar[3] : 1'bz;
  assign MAR_2 = (mar_talk && mar[2]) ? mar[2] : 1'bz;
  assign MAR_1 = (mar_talk && mar[1]) ? mar[1] : 1'bz;
  assign MAR_0 = (mar_talk && mar[0]) ? mar[0] : 1'bz;

  assign MAR_monitor_0 = mar[0] && mute;
  assign MAR_monitor_1 = mar[1] && mute;
  assign MAR_monitor_2 = mar[2] && mute;
  assign MAR_monitor_3 = mar[3] && mute;

  /****************************/

  reg [3:0] mdr = 0;

  reg mdr_listen = 0;
  reg mdr_talk = 0;

  assign MDR_3 = (mdr_talk && mdr[3]) ? mdr[3] : 1'bz;
  assign MDR_2 = (mdr_talk && mdr[2]) ? mdr[2] : 1'bz;
  assign MDR_1 = (mdr_talk && mdr[1]) ? mdr[1] : 1'bz;
  assign MDR_0 = (mdr_talk && mdr[0]) ? mdr[0] : 1'bz;

  assign MDR_monitor_0 = mdr[0] && mute;
  assign MDR_monitor_1 = mdr[1] && mute;
  assign MDR_monitor_2 = mdr[2] && mute;
  assign MDR_monitor_3 = mdr[3] && mute;

  /****************************/

  // Opcodes:

  `define NOP 4'b0000
  `define LDA 4'b0001
  `define INC 4'b0010
  `define DEC 4'b0011
  `define STA 4'b0100
  `define BZ  4'b0101
  `define JMP 4'b0110
  `define SR  4'b0111

  `define LDI 4'b1001

  /****************************/

  // New memory:

  reg [3:0] rw_mem [3:0];
  reg mem_write = 0;
  reg mem_read = 0;

  reg [3:0] mem0 = `LDA; // start: LDA data
  reg [3:0] mem1 = 15;
  reg [3:0] mem2 = `BZ; // loop: BZ start
  reg [3:0] mem3 = 0;
  reg [3:0] mem4 = `SR;
  reg [3:0] mem5 = `NOP;
  reg [3:0] mem6 = `NOP;
  reg [3:0] mem7 = `JMP; // JMP loop
  reg [3:0] mem8 = 2;
  reg [3:0] mem9 = 0;                     // LDI
  reg [3:0] mem10 = 0;                    // 0011
  reg [3:0] mem11 = 0;                    // LDI
  reg [3:0] mem12 = 0;                    // 1100
  reg [3:0] mem13 = 0;                    // JMP
  reg [3:0] mem14 = 0;                    // 9
  reg [3:0] mem15 = 4'b1000; // data:

  always @ (posedge clock) begin
    if (reset) begin
      mdr = 0;
      mem0 = `LDA;
      mem1 = 15;
      mem2 = `BZ;
      mem3 = 0;
      mem4 = `SR;
      mem5 = `NOP;
      mem6 = `NOP;
      mem7 = `JMP;
      mem8 = 2;
      mem9 = 0;
      mem10 = 0;
      mem11 = 0;
      mem12 = 0;
      mem13 = 0;
      mem14 = 0;
      mem15 = 4'b1000;
    end else if (mdr_listen) begin
      mdr = {MDR_3, MDR_2, MDR_1, MDR_0};
    end else if (mem_write) begin
      case (mar)
        0 : mem0 = mdr;
        1 : mem1 = mdr;
        2 : mem2 = mdr;
        3 : mem3 = mdr;
        4 : mem4 = mdr;
        5 : mem5 = mdr;
        6 : mem6 = mdr;
        7 : mem7 = mdr;
        8 : mem8 = mdr;
        9 : mem9 = mdr;
       10 : mem10 = mdr;
       11 : mem11 = mdr;
       12 : mem12 = mdr;
       13 : mem13 = mdr;
       14 : mem14 = mdr;
       15 : mem15 = mdr;
      endcase
    end else begin // memory read
      case (mar)
        0 : mdr = mem0;
        1 : mdr = mem1;
        2 : mdr = mem2;
        3 : mdr = mem3;
        4 : mdr = mem4;
        5 : mdr = mem5;
        6 : mdr = mem6;
        7 : mdr = mem7;
        8 : mdr = mem8;
        9 : mdr = mem9;
       10 : mdr = mem10;
       11 : mdr = mem11;
       12 : mdr = mem12;
       13 : mdr = mem13;
       14 : mdr = mem14;
       15 : mdr = mem15;
      endcase
    end
  end

  assign LED0 = mem14[3];
  assign LED1 = mem14[2];
  assign LED2 = mem14[1];
  assign LED3 = mem14[0];
  assign LED4 = mem15[3];
  assign LED5 = mem15[2];
  assign LED6 = mem15[1];
  assign LED7 = mem15[0];

  // Ersatz memory was read-only.

/*
  always @ (posedge clock) begin
    if (reset) begin
      mdr = 0;
    end else begin
      case (mar)
        0 : mdr = `LDA; // start:
        1 : mdr = 15;
        2 : mdr = `BZ;
        3 : mdr = 0;    // (start)
        4 : mdr = aux_1 ? `DEC : `SR;
        5 : mdr = `JMP;
        6 : mdr = 2;    // (loop)
        7 : mdr = 0;
        8 : mdr = 0;
        9 : mdr = 0;
       10 : mdr = 0;
       11 : mdr = 0;
       12 : mdr = 0;
       13 : mdr = 0;
       14 : mdr = 0;
       15 : mdr = aux_1 ? 4'b1010 : 4'b1000; // himem:
      endcase
    end
  end
*/

  /****************************/

  reg [3:0] alu_a = 0;
  reg [3:0] alu_b = 1; // Force it to 1 to solve a nasty hardware problem.
  reg [3:0] alu_result;
  reg [3:0] alu_control = 0;
  reg zero_flag = 0;
  reg [3:0] alu_error = 0;
  reg alu_a_talk = 0; // not needed; ALU_A will never talk on the bus
  reg alu_b_talk = 0; // not needed; ALU_B will never talk on the bus
  reg alu_result_talk = 0;
  reg alu_a_listen = 0;
  reg alu_b_listen = 0;
  reg alu_result_listen = 0; // not needed: ALU_result will never listen.

  // Fully combinatorial ALU runs faster

  `define ALU_command_add 1
  `define ALU_command_sub 2
  `define ALU_command_sr 3

  always @ (*) begin
    if (reset) begin
      alu_result = 0;
    end else begin
      case (alu_control)
        `ALU_command_add : alu_result = alu_a + alu_b;
        `ALU_command_sub : alu_result = alu_a - alu_b;
        `ALU_command_sr : alu_result = alu_a >> alu_b;
      endcase
    end
  end

  always @ (posedge clock) begin
    if (reset) begin
      alu_a = 0;
    end else if (alu_a_listen) begin
      alu_a <= {ALU_A_3, ALU_A_2, ALU_A_1, ALU_A_0};
    end
  end

  assign ALU_A_3 = (alu_a_talk && alu_a[3]) ? alu_a[3] : 1'bz; // Not needed:
  assign ALU_A_2 = (alu_a_talk && alu_a[2]) ? alu_a[2] : 1'bz; // ALU_A will
  assign ALU_A_1 = (alu_a_talk && alu_a[1]) ? alu_a[1] : 1'bz; // never talk
  assign ALU_A_0 = (alu_a_talk && alu_a[0]) ? alu_a[0] : 1'bz; // on the bus.
  assign ALU_B_3 = (alu_b_talk && alu_b[3]) ? alu_b[3] : 1'bz; // Not needed:
  assign ALU_B_2 = (alu_b_talk && alu_b[2]) ? alu_b[2] : 1'bz; // ALU_B will
  assign ALU_B_1 = (alu_b_talk && alu_b[1]) ? alu_b[1] : 1'bz; // never talk
  assign ALU_B_0 = (alu_b_talk && alu_b[0]) ? alu_b[0] : 1'bz; // on the bus.
  assign ALU_result_3 = (alu_result_talk && alu_result[3]) ? alu_result[3] : 1'bz;
  assign ALU_result_2 = (alu_result_talk && alu_result[2]) ? alu_result[2] : 1'bz;
  assign ALU_result_1 = (alu_result_talk && alu_result[1]) ? alu_result[1] : 1'bz;
  assign ALU_result_0 = (alu_result_talk && alu_result[0]) ? alu_result[0] : 1'bz;

  /* multiplexed display for ALU registers (and memory telltales now) */

  reg [2:0] multiplexed_row = 0;
  reg [3:0] multiplexed_output = 0;
  reg [3:0] multiplexed_select = 0;
  always @ (posedge clk_32_kHz) begin
    case (multiplexed_row)
      0: begin
        multiplexed_output = aux_1 ? mem10 : alu_a;
        multiplexed_select = 4'b1110; // don't forget these are active low
      end
      1: begin
        multiplexed_output = alu_b;
        multiplexed_select = 4'b1101;
      end
      2: begin
        multiplexed_output = aux_1 ? mem11 : alu_control;
        multiplexed_select = 4'b1011;
      end
      3: begin
        multiplexed_output = aux_1 ? mem12 : alu_result;
        multiplexed_select = 4'b0111;
      end
    endcase
    multiplexed_row = multiplexed_row + 1;
      if (multiplexed_row > 4) begin
        multiplexed_row = 0;
      end
  end

  assign ALU_multiplexed_mon_3 = multiplexed_output[3] && mute;
  assign ALU_multiplexed_mon_2 = multiplexed_output[2] && mute;
  assign ALU_multiplexed_mon_1 = multiplexed_output[1] && mute;
  assign ALU_multiplexed_mon_0 = multiplexed_output[0] && mute;
  assign ALU_A_multiplexed_mon_gnd_1 = multiplexed_select[0];
  // ALU_B doesn't have an LED right now b/c no pins on J2.
  assign ALU_control_multiplexed_mon_gnd_3 = multiplexed_select[2];
  assign ALU_result_multiplexed_mon_gnd_4 = multiplexed_select[3];

  /* multiplexed display for bus conflict monitor */

  reg [2:0] bus_conflict_mux_row = 0;
  reg [3:0] bus_conflict_mux_output = 0;
  reg [3:0] bus_conflict_mux_select = 0;
  always @ (negedge clk_32_kHz) begin // hopefully this will make it reliable
    case (bus_conflict_mux_row)
      0: begin
        bus_conflict_mux_output = {pc_talk, ir_talk, mar_talk, mdr_talk};
        bus_conflict_mux_select = 4'b1110; // don't forget these are active low
      end
      1: begin
        bus_conflict_mux_output = {alu_a_talk, alu_b_talk, alu_result_talk,
          acc_talk};
        bus_conflict_mux_select = 4'b1101;
      end
      2: begin
        bus_conflict_mux_output = {pc_listen, ir_listen, mar_listen, mdr_listen};
        bus_conflict_mux_select = 4'b1011;
      end
      3: begin
        bus_conflict_mux_output = {alu_a_listen, alu_b_listen, alu_result_listen,
          acc_listen};
        bus_conflict_mux_select = 4'b0111;
      end
    endcase
    bus_conflict_mux_row = bus_conflict_mux_row + 1;
      if (bus_conflict_mux_row > 4) begin
        bus_conflict_mux_row = 0;
      end
  end

  assign bus_conflict_mux_data_3 = bus_conflict_mux_output[3] && mute;
  assign bus_conflict_mux_data_2 = bus_conflict_mux_output[2] && mute;
  assign bus_conflict_mux_data_1 = bus_conflict_mux_output[1] && mute;
  assign bus_conflict_mux_data_0 = bus_conflict_mux_output[0] && mute;
  assign bus_conflict_mux_gnd_1 = bus_conflict_mux_select[0];
  assign bus_conflict_mux_gnd_2 = bus_conflict_mux_select[1];
  assign bus_conflict_mux_gnd_3 = bus_conflict_mux_select[2];
  assign bus_conflict_mux_gnd_4 = bus_conflict_mux_select[3];

  /****************************/

  // Control unit:

  always @ (posedge clock) begin
    if (reset) begin
      running = 1;
    end
    if (skip_rest_of_sequence) begin
      skip_rest_of_sequence = 0;
    end
    if (running) begin
      case (sequencer)
        0: begin
          pc_talk = 1;
          mar_listen = 1;
          if (skip_rest_of_sequence) begin
            skip_rest_of_sequence = 0;
          end
        end
        1: begin
          pc_talk = 0;
          mar_listen = 0;
        end // adding another cycle for memory read
        2: begin
          mdr_talk = 1;
          ir_listen = 1;
        end
        3: begin
          mdr_talk = 0;
          ir_listen = 0;
          // increment PC.
          pc_talk = 1;
          alu_a_listen = 1;
          // alu_b register is always 1 now.
          alu_control = `ALU_command_add;
        end
        4: begin
          pc_talk = 0;
          alu_a_listen = 0;
          alu_result_talk = 1;
          pc_listen = 1;
        end
        5: begin
          alu_result_talk = 0;
          pc_listen = 0; // PC has now been incremented.
          case (ir)
            `NOP: begin // Let it have one cycle for something to grab hold of.
              end
            `LDA: begin // (go get the target of the load)
                pc_talk <= 1;
                mar_listen <= 1;
              end
            `LDI: begin // new LOAD IMMEDIATE instruction
                pc_talk <= 1;
                mar_listen <= 1;
              end
            `STA: begin // go get the target of the store
                pc_talk <= 1;
                mar_listen <= 1;
              end
            `BZ: begin // BZ (if accumulator zero, then go get address)
              if (accumulator == 0) begin
                pc_talk <= 1;
                mar_listen <= 1;
              end else begin // otherwise, increment PC again
                pc_talk = 1;
                alu_a_listen = 1;
                alu_control = `ALU_command_add;
              end
            end
            `DEC: begin // (the accumulator)
              acc_talk = 1;
              alu_a_listen = 1;
              alu_control = `ALU_command_sub;
            end
            `INC: begin // (the accumulator)
              acc_talk = 1;
              alu_a_listen = 1;
              alu_control = `ALU_command_add;
            end
            `JMP: begin // (to address in next memory word)
              pc_talk = 1;
              mar_listen = 1;
            end
            `SR: begin
              acc_talk = 1;
              alu_a_listen = 1;
              alu_control = `ALU_command_sr;
            end
            default: begin
              running = 0; // illegal instruction decoded
            end
          endcase
        end
        6: begin
          case (ir)
            `NOP: begin // Now we're finished with NOP.
              skip_rest_of_sequence = 1;
              end
            `LDA: begin // (add another cycle for memory read)
              pc_talk = 0;
              mar_listen = 0;
            end
            `LDI: begin // (add another cycle for memory read)
              pc_talk = 0;
              mar_listen = 0;
            end
            `STA: begin // (add another cycle for memory read)
              pc_talk = 0;
              mar_listen = 0;
            end
            `BZ: begin
              if (accumulator == 0) begin
                pc_talk = 0;
                mar_listen = 0;
              end else begin
                pc_talk = 0;
                alu_a_listen = 0;
              end
            end
            `DEC: begin
              acc_talk = 0;
              alu_a_listen = 0;
              alu_result_talk = 1;
              acc_listen = 1;
            end
            `INC: begin
              acc_talk = 0;
              alu_a_listen = 0;
              alu_result_talk = 1;
              acc_listen = 1;
            end
            `JMP: begin
              pc_talk = 0;
              mar_listen = 0;
            end
            `SR: begin
              acc_talk = 0;
              alu_a_listen = 0;
              alu_result_talk = 1;
              acc_listen = 1;
            end
            default: begin
            end
          endcase
        end
        7: begin
          case (ir)
            `LDA: begin
              mdr_talk <= 1;
              mar_listen <= 1;
            end
            `LDI: begin
              mdr_talk = 1;
              acc_listen = 1;
            end
            `STA: begin
              mdr_talk <= 1;
              mar_listen <= 1;
            end
            `BZ: begin
              if (accumulator == 0) begin
                pc_listen <= 1;
                mdr_talk <= 1;
              end else begin
                alu_result_talk = 1;
                pc_listen = 1;
              end
            end
            `DEC: begin
              alu_result_talk = 0;
              acc_listen = 0; // Done with DEC; no need to incrment PC.
              skip_rest_of_sequence = 1;
            end
            `INC: begin
              alu_result_talk = 0;
              acc_listen = 0; // Done with INC no need to incrment PC.
              skip_rest_of_sequence = 1;
            end
            `JMP : begin
              mdr_talk = 1;
              pc_listen = 1;
            end
            `SR: begin
              alu_result_talk = 0;
              acc_listen = 0; // done with SR; no need to increment PC again
              skip_rest_of_sequence = 1;
            end
            default: begin
            end
          endcase
        end
        8: begin
          case (ir)
            `LDA: begin // (add another cycle for memory read)
              mdr_talk = 0;
              mar_listen = 0;
            end
            `LDI: begin
              mdr_talk = 0;
              acc_listen = 0; // Now increment PC again.
            end
            `STA: begin
              mdr_talk = 0;
              mar_listen = 0;
            end
            `BZ: begin
              if (accumulator == 0) begin
                pc_listen = 0; // We just updated PC, so no need to increment it.
                mdr_talk = 0; // done with BZ instruction
              end else begin
                alu_result_talk = 0;
                pc_listen = 0; // done with BZ instruction
              end
              skip_rest_of_sequence = 1;
            end
            `JMP: begin
              mdr_talk = 0;
              pc_listen = 0; // done with JMP; no need to increment PC now.
              skip_rest_of_sequence = 1;
            end
            default: begin
            end
          endcase
        end
        9: begin
          case (ir)
            `LDA: begin
              mdr_talk = 1;
              acc_listen = 1;
            end
            `LDI: begin
              pc_talk = 1;
              alu_a_listen = 1;
              alu_control = `ALU_command_add;
            end
            `STA: begin
              acc_talk = 1;
              mdr_listen = 1;
            end
            default: begin
            end
          endcase
        end
        10: begin
          case (ir)
            `LDA: begin
              mdr_talk = 0;
              acc_listen = 0; // now increment PC again
              pc_talk = 1;
              alu_a_listen = 1;
              alu_control = `ALU_command_add;
            end
            `LDI: begin
              pc_talk = 0;
              alu_a_listen = 0;
              alu_result_talk = 1;
              pc_listen = 1;
            end
            `STA: begin
              acc_talk = 0;
              mdr_listen = 0;
              mem_write = 1;
            end
            default: begin
            end
          endcase
        end
        11: begin
          case (ir)
            `LDA: begin
              pc_talk = 0;
              alu_a_listen = 0;
              alu_result_talk = 1;
              pc_listen = 1;
            end
            `LDI: begin
              alu_result_talk = 0;
              pc_listen = 0;
              skip_rest_of_sequence = 1;
            end
            `STA: begin
              mem_write = 0;
              pc_talk = 1;
              alu_a_listen = 1;
              alu_control = `ALU_command_add;
            end
            default: begin
            end
          endcase
        end
        12: begin
          case (ir)
            `LDA: begin
              alu_result_talk = 0;
              pc_listen = 0;
              skip_rest_of_sequence = 1;
            end
            `STA: begin
              pc_talk = 0;
              alu_a_listen = 0;
              alu_result_talk = 1;
              pc_listen = 1;
            end
            default: begin
            end
          endcase
        end
        13: begin
          case (ir)
            `STA: begin
              alu_result_talk = 0;
              pc_listen = 0;
              skip_rest_of_sequence = 1;
            end
            default: begin
            end
          endcase
        end
        default: begin
        end
      endcase
    end
  end
endmodule
