module SR_latch(input S, input R, output Q, output not_Q);
  nor(Q, R, not_Q);
  nor(not_Q, S, Q);
endmodule

module SR_debounce(input switch_on, input switch_off, output result);
  SR_latch sr (switch_on, switch_off, result);
endmodule
