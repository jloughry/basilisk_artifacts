module debouncer (
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
reg [31:0] PB_cnt;
always @ (posedge clock)
if (switch_state == PB_sync_1)
  PB_cnt <= 0;
else
begin
  PB_cnt <= PB_cnt + 1;
  if(PB_cnt == 20'hfffff) switch_state <= ~switch_state; 
end

always @ (negedge clock) begin
  logical_switch_state <= !switch_state;
end
endmodule

module debounced_sw (
  input clock, input switch_high, input switch_low, output state,
);

  debouncer db (clock, switch_high, state);
endmodule
