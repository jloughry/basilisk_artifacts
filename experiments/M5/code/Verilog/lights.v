module lights (
  LED0, LED1, LED2, LED3, LED4, LED5, LED6, LED7,

  run_lamp,
  halt_lamp,
  clock_lamp,
  aux_1_lamp,
  aux_2_lamp,

  acc_led_0,
  acc_led_1,
  acc_led_2,
  acc_led_3,
);

output LED0, LED1, LED2, LED3, LED4, LED5, LED6, LED7;

output run_lamp;
output halt_lamp;
output clock_lamp;
output aux_1_lamp;
output aux_2_lamp;

output acc_led_0;
output acc_led_1;
output acc_led_2;
output acc_led_3;

endmodule
