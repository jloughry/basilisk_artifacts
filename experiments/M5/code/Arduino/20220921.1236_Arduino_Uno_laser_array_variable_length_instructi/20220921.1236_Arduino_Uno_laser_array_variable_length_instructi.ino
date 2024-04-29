#define TACTILE_SWITCH A0
#define PHOTODIODE_ACC_3 A3
#define PHOTODIODE_ACC_2 A2
#define PHOTODIODE_ACC_1 A4
#define PHOTODIODE_ACC_0 A5

const int LASER_ARRAY_0 = 4;
const int LASER_ARRAY_1 = 5;
const int LASER_ARRAY_2 = 6;
const int LASER_ARRAY_3 = 7;
const int RED_LOCK_LED = 3;
const int GREEN_FOI_LED = LED_BUILTIN;

const unsigned int cycle_time = 7; // 10 ms for fast clock; 2000 for slow.

void wait_for_rising_edge_of_bit(unsigned int which_bit) {
  static enum bit_states {invalid, off, on} bit_state = invalid;
  int sensed = -1;
  bool found = false;

  while (!found) {
    switch (which_bit) {
      case 3:
        sensed = !digitalRead(PHOTODIODE_ACC_3);
        break;
      case 2:
        sensed = !digitalRead(PHOTODIODE_ACC_2);
        break;
      case 1:
        sensed = !digitalRead(PHOTODIODE_ACC_1);
        break;
      case 0:
        sensed = !digitalRead(PHOTODIODE_ACC_0);
        break;
      default:
        sensed = 0;
        break;
    }
  
    switch(bit_state) {
      case invalid:
        if (sensed) {
          bit_state = on;
        }
        else {
          bit_state = off;
        }
        break;
      case off:
        if (sensed) {
          bit_state = on; // And we found the leading edge.
          found = true;
        }
        break;
      case on:
        if (!sensed) {
          bit_state = off;
        }
        break;
      default:
        bit_state = invalid;
        break;
    }
  }
}

void lock_led_on(void) {
  digitalWrite(RED_LOCK_LED, HIGH);
}

void lock_led_off(void) {
  digitalWrite(RED_LOCK_LED, LOW);
}

void green_led_on(void) {
  digitalWrite(GREEN_FOI_LED, HIGH);
}

void green_led_off(void) {
  digitalWrite(GREEN_FOI_LED, LOW);
}

void all_lasers_off(void) {
  digitalWrite(LASER_ARRAY_0, LOW);
  digitalWrite(LASER_ARRAY_1, LOW);
  digitalWrite(LASER_ARRAY_2, LOW);
  digitalWrite(LASER_ARRAY_3, LOW);  
}

void setup() {
  pinMode(PHOTODIODE_ACC_3, INPUT);
  pinMode(PHOTODIODE_ACC_2, INPUT);
  pinMode(PHOTODIODE_ACC_1, INPUT);
  pinMode(PHOTODIODE_ACC_0, INPUT);

  pinMode(GREEN_FOI_LED, OUTPUT);
  pinMode(RED_LOCK_LED, OUTPUT);
  pinMode(TACTILE_SWITCH, INPUT_PULLUP);

  pinMode(LASER_ARRAY_0, OUTPUT);
  pinMode(LASER_ARRAY_1, OUTPUT);
  pinMode(LASER_ARRAY_2, OUTPUT);
  pinMode(LASER_ARRAY_3, OUTPUT);

  all_lasers_off();
}

void laser_3_on(void) {
  digitalWrite(LASER_ARRAY_3, HIGH);
}

void laser_3_off(void) {
  digitalWrite(LASER_ARRAY_3, LOW);
}

void laser_2_on(void) {
  digitalWrite(LASER_ARRAY_2, HIGH);
}

void laser_2_off(void) {
  digitalWrite(LASER_ARRAY_2, LOW);
}

void laser_1_on(void) {
  digitalWrite(LASER_ARRAY_1, HIGH);
}

void laser_1_off(void) {
  digitalWrite(LASER_ARRAY_1, LOW);
}

void laser_0_on(void) {
  digitalWrite(LASER_ARRAY_0, HIGH);
}

void laser_0_off(void) {
  digitalWrite(LASER_ARRAY_0, LOW);
}

// Make lock and go global so others can see them.
bool lock = false;
bool go = false;

void cooperative_button_press_detector(void) {
  if (!digitalRead(TACTILE_SWITCH)) {
    go = true;
  }
}

void cooperative_lock_indicator(void) {
  if (lock) {
    lock_led_on();
  }
  else {
    lock_led_off();
  }
}

void wait_one_cycle(void) {
  delay(cycle_time);
}

void wait_n_cycles(unsigned int n) {
  // delay(cycle_time * n);
  
  for (int i=0; i<n; i++) {
    wait_one_cycle();
  }
}

void wait_one_instruction(void) {
  wait_n_cycles(12);
}

void wait_out_SR_instruction(void) {
  wait_one_cycle();
}

void wait_into_NOP_instruction(void) {
  wait_n_cycles(3);
}

void fire_lasers (int laser_3, int laser_2, int laser_1, int laser_0) {
  lock_led_on();
  if (laser_3) laser_3_on();
  if (laser_2) laser_2_on();
  if (laser_1) laser_1_on();
  if (laser_0) laser_0_on();
  wait_n_cycles(1);
  all_lasers_off();
  lock_led_off();
}

void loop() {
  cooperative_button_press_detector();

  if (go) {
    green_led_on();

    /* New target programme:
     *   9: LDA (0001)
     *  10: 13
     *  11: LDA (0001)
     *  12: 14
     *  13: JMP
     *  14: 9
     */

    // 9: LDA
    wait_for_rising_edge_of_bit(0); // accumulator contains 0001
    wait_out_SR_instruction();
    wait_into_NOP_instruction();
    fire_lasers(0, 1, 0, 0); // turn NOP into STA instruction
    wait_n_cycles(4);
    fire_lasers(1, 0, 0, 1); // make address be 9

    // 10: 7
    wait_for_rising_edge_of_bit(2);
    wait_out_SR_instruction();
    wait_into_NOP_instruction();
    fire_lasers(0, 1, 0, 0); // turn NOP into STA
    wait_n_cycles(4);
    fire_lasers(1, 0, 1, 0); // make address be 10
    wait_one_cycle();
    fire_lasers(0, 1, 1, 1); // set bits in ACC value to be 14

    // 11: another LDA instruction, using STA
    wait_for_rising_edge_of_bit(0);
    wait_out_SR_instruction();
    wait_into_NOP_instruction();
    fire_lasers(0, 1, 0, 0); // turn NOP into STA instruction
    wait_n_cycles(4);
    fire_lasers(1, 0, 1, 1); // make address be 11

    // 12: 14
    wait_for_rising_edge_of_bit(1);
    wait_out_SR_instruction();
    wait_into_NOP_instruction();
    fire_lasers(0, 1, 0, 0); // turn NOP into STA
    wait_n_cycles(4);
    fire_lasers(1, 1, 0, 0); // make address be 12
    wait_one_cycle();
    fire_lasers(1, 1, 1, 0); // set bits in ACC value to be 14

    // 13: JMP
    wait_for_rising_edge_of_bit(2);
    wait_out_SR_instruction();
    wait_into_NOP_instruction();
    fire_lasers(0, 1, 0, 0); // turn NOP into STA
    wait_n_cycles(4);
    fire_lasers(1, 1, 0, 1); // make address be 13
    wait_one_cycle();
    fire_lasers(0, 0, 1, 0); // set one more bit in ACC value.

    // 14: 9
    wait_for_rising_edge_of_bit(0);
    wait_out_SR_instruction();
    wait_into_NOP_instruction();
    fire_lasers(0, 1, 0, 0); // turn NOP into STA
    wait_n_cycles(4);
    fire_lasers(1, 1, 1, 0); // make address be 14
    wait_one_cycle();
    fire_lasers(1, 0, 0, 0); // set one more bit in ACC value.

    // Now force a branch to address 9.
    wait_for_rising_edge_of_bit(1); // accumulator contains 0010
    wait_out_SR_instruction();
    wait_into_NOP_instruction();
    fire_lasers(0, 1, 1, 0); // change NOP to JMP instruction
    wait_n_cycles(4);
    fire_lasers(1, 1, 0, 1); // change address to 1101
    
    green_led_off();
    go = false;
  }
}
