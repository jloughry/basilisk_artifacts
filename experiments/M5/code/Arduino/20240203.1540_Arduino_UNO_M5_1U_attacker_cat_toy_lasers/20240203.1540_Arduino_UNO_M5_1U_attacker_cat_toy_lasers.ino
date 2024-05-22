const unsigned int NUMBER_OF_LASERS = 4;

const uint8_t photodiode_pin[NUMBER_OF_LASERS] = {10, 9, 2, 3}; // 0--3
const uint8_t laser_pin[NUMBER_OF_LASERS] = {4, 8, 7, 12}; // 0--3
const uint8_t LASER_FIRING_PIN = 6;

#define PUSHBUTTON_SWITCH_PIN A0
#define RED_LED_INDICATOR_PIN A4
#define GREEN_LED_INDICATOR_PIN A5

unsigned long cycle_time = 0; // microseconds; global so everyone can see it.
unsigned long laser_firing_time = 0; // microseconds
unsigned long overlap = 0; // microseconds

// All time measurements, except for trivial use of delay(), are in microseconds.

enum attacks {blinker, increment} selected_attack = blinker;

void setup() {
  for (int i=0; i<4; i++) {
    pinMode(photodiode_pin[i], INPUT);
    pinMode(laser_pin[i], OUTPUT);
  }

  pinMode(LASER_FIRING_PIN, OUTPUT);

  pinMode(11, INPUT); // beamsplitter photodiodes
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  
  pinMode(PUSHBUTTON_SWITCH_PIN, INPUT_PULLUP);
  pinMode(RED_LED_INDICATOR_PIN, OUTPUT);
  pinMode(GREEN_LED_INDICATOR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  all_lasers_off();
  all_indicators_off();
  indicate_laser_off();

  Serial.begin(9600);
  Serial.println("Ready.");
}

/*
 * This function response to single or double click differently.
 */

bool button_pressed(void) {
  if (!digitalRead(PUSHBUTTON_SWITCH_PIN)) {
    delay(100); // short pause for debouncing
    while (!digitalRead(PUSHBUTTON_SWITCH_PIN)) ; // wait until button is released
    delay(100); // short pause for debouncing
    selected_attack = blinker;
    // But keep watching for a little while in case there's another click.
    unsigned long now = micros();
    while (micros() < now + 250000) { // quarter of a second
      if (!digitalRead(PUSHBUTTON_SWITCH_PIN)) {
        delay(100); // short pause for debouncing
        // got a double click
        while (!digitalRead(PUSHBUTTON_SWITCH_PIN)) ; // wait until button is released
        selected_attack = increment;
        delay(100); // short pause for debouncing
        break;
      }
    }
    return true;
  }
  return false;
}

// This is a blocking function!
void wait_for_button(void) {
  while (!button_pressed()) ;  
}

void cooperative_test_buttons(void) {
  if (button_pressed()) {
    switch(selected_attack) {
      case blinker:
        steady_red();
        break;
      case increment:
        flashing_red();
        break;
      default:
        halt_with_error_indication();
        break;
    }
  }
}

void green_led_on(void) {
  digitalWrite(GREEN_LED_INDICATOR_PIN, HIGH);
}

void green_led_off(void) {
  digitalWrite(GREEN_LED_INDICATOR_PIN, LOW);
}

void red_led_on(void) {
  digitalWrite(RED_LED_INDICATOR_PIN, HIGH);
}

void red_led_off(void) {
  digitalWrite(RED_LED_INDICATOR_PIN, LOW);
}

void yellow_led_on(void) {
  digitalWrite(LED_BUILTIN, HIGH);
}

void yellow_led_off(void) {
  digitalWrite(LED_BUILTIN, LOW);
}

void all_indicators_on(void) {
  red_led_on();
  green_led_on();
  yellow_led_on();
}

/*
 * This function never returns.
 */
 
void halt_with_error_indication(void) {
  red_led_off();
  green_led_off();
  for(;;) {
    const unsigned long speed = 120;
    green_led_off();
    red_led_on();
    delay(speed);
    red_led_off();
    green_led_on();
    delay(speed);
  }
}

void all_indicators_off(void) {
  red_led_off();
  green_led_off();
  yellow_led_off();
}

// This function should be called from loop().

void cooperative_blink(void) {
  const unsigned int blink_interval = 1000; // milliseconds
  static enum states {off, on} state = off;
  static unsigned long time_of_last_state_change = 0;
  unsigned long now = millis();

  if (now - time_of_last_state_change > blink_interval) {
    switch(state) {
      case off:
        yellow_led_on();
        state = on;
        break;
      case on:
        yellow_led_off();
        state = off;
        break;
      default:
        yellow_led_off();
        state = off;
        break;
    }
    time_of_last_state_change = now;
  }
}

typedef enum indicator_states {off, flashing, on} indicator;
indicator green_indicator = off;
indicator red_indicator = off;

void steady_green(void) {
  green_led_on();
  green_indicator = on;
}

void steady_red(void) {
  red_led_on();
  red_indicator = on;
}

void flashing_green(void) {
  green_indicator = flashing;
}

void stop_flashing_green(void) {
  green_indicator = off;
}

void flashing_red(void) {
  red_indicator = flashing;
}

void stop_flashing_red(void) {
  red_indicator = off;
}

// This function should be called from loop(). It implements MIL-STD-1472 flashing indicators.

void  cooperative_synchronized_flashing_indicator_mechanism(void) {
  const unsigned duty_cycle_in_percent = 50;
  const unsigned flash_rate_in_Hz = 2;
  const unsigned long on_time = 1000 / flash_rate_in_Hz * duty_cycle_in_percent / 100; // milliseconds
  const unsigned long off_time = 1000 / flash_rate_in_Hz * ((float)(100 - duty_cycle_in_percent) / 100); // milliseconds
  static enum states {off, on} state = on;
  static unsigned long time_of_last_state_change = 0;
  unsigned long now = millis();
  switch(state) {
    case off:
      if (now - time_of_last_state_change > off_time) {
        if (red_indicator == flashing) {
          red_led_on();
        }
        if (green_indicator == flashing) {
          green_led_on();
        }
        state = on;
        time_of_last_state_change = now;
      }
      break;
    case on:
      if (now - time_of_last_state_change > on_time) {
        if (red_indicator == flashing) {
          red_led_off();
        }
        if (green_indicator == flashing) {
          green_led_off();
        }
        state = off;
        time_of_last_state_change = now;
      }
      break;
    default:
      all_indicators_off();
      state = off;
      time_of_last_state_change = now;
      break;
  }
}

void laser_on(uint8_t which) {
  digitalWrite(laser_pin[which], HIGH);
}

void laser_off(uint8_t which) {
  digitalWrite(laser_pin[which], LOW);
}

void all_lasers_off(void) {
  for (int i=0; i<4; i++) {
    laser_off(i);
  }
}

void indicate_laser_off(void) {
  digitalWrite(LASER_FIRING_PIN, HIGH);
}

void indicate_laser_on(void) {
  digitalWrite(LASER_FIRING_PIN, LOW);
}

bool beamsplitter_photodiode(uint8_t which) {
  switch (which) {
    case 3:
      return !digitalRead(11);
      break;
    case 2:
      return !digitalRead(A1);
      break;
    case 1:
      return !digitalRead(A2);
      break;
    case 0:
      return !digitalRead(A3);
      break;
    default:
     return false;
     break;
  }
}

void wait_for_laser_to_come_on(uint8_t which) {  
  while (!beamsplitter_photodiode(which)) ;
}

void wait_for_laser_to_go_off(uint8_t which) { 
  while (beamsplitter_photodiode(which)) ;
}

// Lasers fire for one cycle_time. This is a blocking function.

void fire_lasers(int bit_3, int bit_2, int bit_1, int bit_0, unsigned long firing_time) {
  const unsigned long laser_safety_shutdown_time = 1500000; // microseconds; a second and a half
  unsigned long now = micros();
  
  if (bit_3) laser_on(3);
  if (bit_2) laser_on(2);
  if (bit_1) laser_on(1);
  if (bit_0) laser_on(0);

  // If the beamsplitter photo diodes are not hooked up, these trivially succeed:

  if (bit_3) wait_for_laser_to_come_on(3);
  if (bit_2) wait_for_laser_to_come_on(2);
  if (bit_1) wait_for_laser_to_come_on(1);
  if (bit_0) wait_for_laser_to_come_on(0);

  indicate_laser_on();
  
  // That pretty much guarantees if a laser is supposed to be on, it's on.

  // If I interleaved the checks, it would make worst-case performance of this fn be about 600 microseconds.
  // That's probably acceptable, given that I only need a 3 ms cycle time...it would make it reliable.  

  unsigned long laser_on_time = now;
  
  while (micros() < now + firing_time) {
    if ((micros() - laser_on_time) > laser_safety_shutdown_time) {
      all_lasers_off();
      halt_with_error_indication();
    }
  }
  
  all_lasers_off();

  wait_for_laser_to_go_off(3);
  wait_for_laser_to_go_off(2);
  wait_for_laser_to_go_off(1);
  wait_for_laser_to_go_off(0);

  indicate_laser_off();
  
  return;
}

bool photodiode(uint8_t which_bit) {
  return !digitalRead(photodiode_pin[which_bit]);
}

void identify_photodiodes(void) {
  Serial.print(photodiode(3));
  Serial.print(", ");
  Serial.print(photodiode(2));
  Serial.print(", ");
  Serial.print(photodiode(1));
  Serial.print(", ");
  Serial.println(photodiode(0));
}

void identify_beamsplitter_photodiodes(void) {
  Serial.print(beamsplitter_photodiode(3));
  Serial.print(", ");
  Serial.print(beamsplitter_photodiode(2));
  Serial.print(", ");
  Serial.print(beamsplitter_photodiode(1));
  Serial.print(", ");
  Serial.println(beamsplitter_photodiode(0));
}

// Global so I can access it from another function:

unsigned long rising_edge_time = 0;
unsigned long falling_edge_time = 0;

// This is a blocking function.

void wait_for_rising_edge_of_bit(uint8_t which_bit) {
  int bit_value = 1;
  
  // Firstly, wait for the bit to be off.
  while (bit_value) {
    bit_value = !digitalRead(photodiode_pin[which_bit]);
  }
  // Secondly, wait for the bit to come on.
  while (!bit_value) {
    bit_value = !digitalRead(photodiode_pin[which_bit]);
  }
  rising_edge_time = micros();;
  return; // We are at the rising edge of the bit.
}

// This is a blocking function.

void wait_for_falling_edge_of_bit(uint8_t which_bit) {
  int bit_value = 1;
  
  // Firstly, wait for the bit to be on.
  while (!bit_value) {
    bit_value = !digitalRead(photodiode_pin[which_bit]);
  }
  // Secondly, wait for the bit to go off.
  while (bit_value) {
    bit_value = !digitalRead(photodiode_pin[which_bit]);
  }
  falling_edge_time = micros();;
  return; // We are at the falling edge of the bit.
}


// This is a blocking function.

unsigned long measure_cycle_time(void) {
  unsigned long start_time = 0;
  unsigned long stop_time = 0;
  unsigned long elapsed_time = 0;
  const unsigned long number_of_cycles_between_accumulator_bits = 49;
  unsigned long measured_cycle_time = 0;

  wait_for_rising_edge_of_bit(1);
  start_time = micros();
  wait_for_rising_edge_of_bit(0);
  stop_time = micros();
  elapsed_time = stop_time - start_time;
  measured_cycle_time = elapsed_time / number_of_cycles_between_accumulator_bits;
  return measured_cycle_time;
}

// Returns the instantaneous value of the accumulator display.

int accumulator(void) {
  bool bit_3 = photodiode(3);
  bool bit_2 = photodiode(2);
  bool bit_1 = photodiode(1);
  bool bit_0 = photodiode(0);
  int value = 0;

  if (bit_3) {
    value += 8;
  }
  if (bit_2) {
    value += 4;
  }
  if (bit_1) {
    value += 2;
  }
  if (bit_0) {
    value += 1;
  }
  return value;
}

unsigned long scheduled_first_firing_time = 0;
unsigned long actual_first_firing_time = 0;
unsigned long scheduled_second_firing_time = 0;
unsigned long actual_second_firing_time = 0;
unsigned long scheduled_third_firing_time = 0;
unsigned long actual_third_firing_time = 0;

unsigned long scheduled_first_stopping_time = 0;
unsigned long actual_first_stopping_time = 0;
unsigned long scheduled_second_stopping_time = 0;
unsigned long actual_second_stopping_time = 0;
unsigned long scheduled_third_stopping_time = 0;
unsigned long actual_third_stopping_time = 0;

void report_timing_violations (char * fn_name) {
  Serial.print("in ");
  Serial.println(fn_name);
  Serial.print("  scheduled_first_firing_time = ");
  Serial.print(scheduled_first_firing_time);
  Serial.print("; actual_first_firing_time = ");
  Serial.print(actual_first_firing_time);
  Serial.print("; scheduled_first_stopping_time = ");
  Serial.print(scheduled_first_stopping_time);
  Serial.print("; actual_first_stopping_time = ");
  Serial.print(actual_first_stopping_time);
  Serial.print("; pulse width = ");
  Serial.print(actual_first_stopping_time - actual_first_firing_time);
  Serial.println(" microseconds.");

  Serial.print("  scheduled_second_firing_time = ");
  Serial.print(scheduled_second_firing_time);
  Serial.print("; actual_second_firing_time = ");
  Serial.print(actual_second_firing_time);
  Serial.print("; scheduled_second_stopping_time = ");
  Serial.print(scheduled_second_stopping_time);
  Serial.print("; actual_second_stopping_time = ");
  Serial.print(actual_second_stopping_time);
  Serial.print("; pulse width = ");
  Serial.print(actual_second_stopping_time - actual_second_firing_time);
  Serial.println(" microseconds.");

  Serial.print("  scheduled_third_firing_time = ");
  Serial.print(scheduled_third_firing_time);
  Serial.print("; actual_third_firing_time = ");
  Serial.print(actual_third_firing_time);
  Serial.print("; scheduled_third_stopping_time = ");
  Serial.print(scheduled_third_stopping_time);
  Serial.print("; actual_third_stopping_time = ");
  Serial.print(actual_third_stopping_time);
  Serial.print("; pulse width = ");
  Serial.print(actual_third_stopping_time - actual_third_firing_time);
  Serial.println(" microseconds.");
}

void general_force_memory(uint8_t address, uint8_t value) {
  int addr_bit[4];
  int data_bit[4];

  for (int i = 3; i >= 0; i--) {
    int bitmask = 1 << i;

    addr_bit[i]  = (address & bitmask) ? 1 : 0;
    data_bit[i] = (value & bitmask)    ? 1 : 0;
  }

  overlap = 0.1 * cycle_time;
  laser_firing_time = overlap + cycle_time + overlap; // 0.1 works with start of 4.8

  wait_for_rising_edge_of_bit(0); // accumulator contains 0001
  const unsigned cycles_between_accumulator_bits = 49;
  unsigned long calculated_marker = rising_edge_time + (cycle_time * cycles_between_accumulator_bits);

  unsigned long theoretical_first_firing_time = 0;
  unsigned long theoretical_second_firing_time = 0;
  unsigned long theoretical_third_firing_time = 0;
  unsigned long laser_cycle_time = cycle_time;

  // wait 4.5 cycles, then fire lasers to turn NOP to STA
  theoretical_first_firing_time = calculated_marker + (4.5 * cycle_time);
  scheduled_first_firing_time = theoretical_first_firing_time - overlap;

  // wait 5 cycles, then fire lasers for address
  theoretical_second_firing_time = theoretical_first_firing_time + laser_cycle_time + (5 * cycle_time);
  scheduled_second_firing_time = theoretical_second_firing_time - overlap;
  
  // wait 1 cycle, then fire lasers for data
  theoretical_third_firing_time = theoretical_second_firing_time + laser_cycle_time + (1 * cycle_time);
  scheduled_third_firing_time = theoretical_third_firing_time - overlap;
  
  while (micros() < scheduled_first_firing_time) ;
  fire_lasers(0,1,0,0, laser_firing_time); // turn NOP into STA in IR

  while(micros() < scheduled_second_firing_time) ;
  fire_lasers(addr_bit[3], addr_bit[2], addr_bit[1], addr_bit[0], laser_firing_time);

  while (micros() < scheduled_third_firing_time) ;
  fire_lasers(data_bit[3], data_bit[2], data_bit[1], data_bit[0], laser_firing_time);

  return;
}

void clean_force_memory_odd (uint8_t address, uint8_t value) {
  int addr_bit[4];
  int data_bit[4];

  for (int i = 3; i >= 0; i--) {
    int bitmask = 1 << i;

    addr_bit[i]  = (address & bitmask) ? 1 : 0;
    data_bit[i] = (value & bitmask)    ? 1 : 0;
  }

  overlap = 0.1 * cycle_time;
  laser_firing_time = overlap + cycle_time + overlap; // 0.1 works with start of 4.8

  unsigned long theoretical_first_firing_time = 0;
  unsigned long theoretical_second_firing_time = 0;
  unsigned long theoretical_third_firing_time = 0;
  unsigned long laser_cycle_time = cycle_time;

  wait_for_rising_edge_of_bit(0); // accumulator contains 0001

  // We want to hit the trailing edge of when ir_listen = 1 in the microcode.

  // wait 4.5 cycles, then fire lasers to turn NOP to STA
  theoretical_first_firing_time = rising_edge_time + (4.5 * cycle_time);
  scheduled_first_firing_time = theoretical_first_firing_time - overlap;
  scheduled_first_stopping_time = scheduled_first_firing_time + laser_firing_time + overlap;

  // wait 5 cycles, then fire lasers for address
  theoretical_second_firing_time = theoretical_first_firing_time + laser_cycle_time + (5 * cycle_time);
  scheduled_second_firing_time = theoretical_second_firing_time - overlap;
  scheduled_second_stopping_time = scheduled_second_firing_time + laser_firing_time + overlap;
  
  // wait 1 cycle, then fire lasers for data
  theoretical_third_firing_time = theoretical_second_firing_time + laser_cycle_time + (1 * cycle_time);
  scheduled_third_firing_time = theoretical_third_firing_time - overlap;
  scheduled_third_stopping_time = scheduled_third_firing_time + laser_firing_time + overlap;
  
  while (micros() < scheduled_first_firing_time) ;
  actual_first_firing_time = micros();
  fire_lasers(0,1,0,0, laser_firing_time); // turn NOP into STA in IR
  actual_first_stopping_time = micros();

  while(micros() < scheduled_second_firing_time) ;
  actual_second_firing_time = micros();
  fire_lasers(addr_bit[3], addr_bit[2], addr_bit[1], addr_bit[0], laser_firing_time);
  actual_second_stopping_time = micros();

  while (micros() < scheduled_third_firing_time) ;
  actual_third_firing_time = micros();
  fire_lasers(data_bit[3], data_bit[2], data_bit[1], data_bit[0], laser_firing_time);
  actual_third_stopping_time = micros();

  report_timing_violations("clean_force_memory_odd()");
}

void clean_force_memory_even (uint8_t address, uint8_t value) {
  int addr_bit[4];
  int data_bit[4];

  for (int i = 3; i >= 0; i--) {
    int bitmask = 1 << i;

    addr_bit[i]  = (address & bitmask) ? 1 : 0;
    data_bit[i] = (value & bitmask)    ? 1 : 0;
  }

  overlap = 0.1 * cycle_time;
  laser_firing_time = overlap + cycle_time + overlap;

  unsigned long theoretical_first_firing_time = 0;
  unsigned long theoretical_second_firing_time = 0;
  unsigned long theoretical_third_firing_time = 0;
  unsigned long theoretical_laser_firing_time = cycle_time;

  wait_for_rising_edge_of_bit(2); // accumulator contains 0100

  unsigned long now = micros();
  
  // wait 4.5 cycles, then fire lasers to turn NOP to STA
  scheduled_first_firing_time = now + (4.5 * cycle_time);
  scheduled_first_stopping_time = scheduled_first_firing_time + laser_firing_time;

  // wait 4.5 cycles, then fire lasers to force the new address on the bus.
  scheduled_second_firing_time = scheduled_first_firing_time + laser_firing_time + (4.5 * cycle_time);
  scheduled_second_stopping_time = scheduled_second_firing_time + laser_firing_time;
  
  // wait 1 cycle, then fire lasers to force data on bus.
  scheduled_third_firing_time = scheduled_second_firing_time + laser_firing_time + cycle_time;
  scheduled_third_stopping_time = scheduled_third_firing_time + laser_firing_time;

  while (micros() < scheduled_first_firing_time) ;
  actual_first_firing_time = micros();
  fire_lasers(0,1,0,0, laser_firing_time);
  actual_first_stopping_time = micros();

  while(micros() < scheduled_second_firing_time) ;
  actual_second_firing_time = micros();
  fire_lasers(addr_bit[3], addr_bit[2], addr_bit[1], addr_bit[0], laser_firing_time);
  actual_second_stopping_time = micros();

  while (micros() < scheduled_third_firing_time) ;
  actual_third_firing_time = micros();
  fire_lasers(data_bit[3], data_bit[2], data_bit[1], data_bit[0], laser_firing_time);
  actual_third_stopping_time = micros();

  report_timing_violations("clean_force_memory_even()");
}

void clean_force_branch(uint8_t address) {
  int addr_bit[4];

  for (int i = 3; i >= 0; i--) {
    int bitmask = 1 << i;

    addr_bit[i]  = (address & bitmask) ? 1 : 0;
  }

  laser_firing_time = cycle_time;
  wait_for_rising_edge_of_bit(0); // accumulator contains 0001

  // wait 4.5 cycles, then fire lasers to change NOP to JMP
  scheduled_first_firing_time = rising_edge_time + (4.5 * cycle_time);
  scheduled_first_stopping_time = scheduled_first_firing_time + laser_firing_time;

  // wait 5 cycles, then fire lasers
  scheduled_second_firing_time = scheduled_first_firing_time + laser_firing_time + (5 * cycle_time);
  scheduled_second_stopping_time = scheduled_second_firing_time + laser_firing_time;

  // third firing cycle not used here
  scheduled_third_firing_time = 0;
  scheduled_third_stopping_time = 0;
  
  while (micros() < scheduled_first_firing_time) ;
  actual_first_firing_time = micros();
  fire_lasers(0,1,1,0, laser_firing_time);
  actual_first_stopping_time = micros();

  while (micros() < scheduled_second_firing_time) ;
  actual_second_firing_time = micros();
  fire_lasers(addr_bit[3], addr_bit[2], addr_bit[1], addr_bit[0], laser_firing_time);
  actual_second_stopping_time = micros();

  report_timing_violations("clean_force_branch()");
}

void general_force_branch(uint8_t address) {
  int addr_bit[4];

  for (int i = 3; i >= 0; i--) {
    int bitmask = 1 << i;

    addr_bit[i]  = (address & bitmask) ? 1 : 0;
  }

  laser_firing_time = cycle_time;

  wait_for_rising_edge_of_bit(0); // accumulator contains 0001
  const unsigned cycles_between_accumulator_bits = 49;
  unsigned long calculated_marker = rising_edge_time + (cycle_time * cycles_between_accumulator_bits);

  // wait 4.5 cycles, then fire lasers to change NOP to JMP
  scheduled_first_firing_time = calculated_marker + (4.5 * cycle_time);

  // wait 5 cycles, then fire lasers
  scheduled_second_firing_time = scheduled_first_firing_time + laser_firing_time + (5 * cycle_time);
  
  while (micros() < scheduled_first_firing_time) ;
  fire_lasers(0,1,1,0, laser_firing_time);

  while (micros() < scheduled_second_firing_time) ;
  fire_lasers(addr_bit[3], addr_bit[2], addr_bit[1], addr_bit[0], laser_firing_time);

  return;
}

void attack_blinker(void) {
  /* New target programme:
   9: LDI (1000)
  10: 0011
  11: LDI (1000)
  12: 1100
  13: JMP (0110)
  14: 9
  */

  const int NOP_INSTRUCTION = 0;
  const int LDA_INSTRUCTION = 1;
  const int INC_INSTRUCTION = 2;
  const int DEC_INSTRUCTION = 3;
  const int STA_INSTRUCTION = 4;
  const int BZ_INSTRUCTION  = 5;
  const int JMP_INSTRUCTION = 6;
  const int SR_INSTRUCTION  = 7;
  const int LDI_INSTRUCTION = 9;

  const int BIT_PATTERN_1001 = 9;
  const int BIT_PATTERN_0110 = 6;

  // IMPORTANT NOTE: wait_for_rising_edge_of_bit(3) does not work.

  all_indicators_off();
  steady_red();

  // known working attack.
  clean_force_memory_odd ( 9, LDI_INSTRUCTION);
  clean_force_memory_odd (10, BIT_PATTERN_1001);
  clean_force_memory_odd (11, LDI_INSTRUCTION);
  clean_force_memory_even (12, BIT_PATTERN_0110);
  clean_force_memory_even (13, JMP_INSTRUCTION);
  clean_force_memory_odd (14, 9); // starting address of this program
  clean_force_branch(9);

  red_led_off();
}

void attack_increment(void) {
  /* New attack programme:
    9: INC (0010)
   10: JMP (0110)
   11: 9   (1001)
  */

  const int INC_INSTRUCTION = 2;
  const int JMP_INSTRUCTION = 6;

  all_indicators_off();
  steady_red();

  general_force_memory(9, INC_INSTRUCTION);
  general_force_memory(10, JMP_INSTRUCTION);
  general_force_memory(11, 9);
  general_force_branch(9);

  red_led_off();
}

void attack(void) {
  switch(selected_attack) {
    case blinker:
      attack_blinker();
      break;
    case increment:
      attack_increment();
      break;
    default:
      halt_with_error_indication();
      break;
  }
}

/*
 * This function is intended to be called from loop().
 */

bool phase_lock = false;

void cooperative_phase_locked_loop(void) {
  static enum states {empty, alpha, beta, gamma, delta, other} state = other;
  static unsigned long time_of_last_state_change = 0;
  static unsigned long cycle_time_age = 0;
  const unsigned long decision_level = 10; // after this many times through, conclude we have lock.
  unsigned long too_long = 2500000; // microseconds; change this to 120_000_000 for slow clock.
  const unsigned cycles_between_accumulator_bits = 49;
  static unsigned long time_first_bit_seen = 0;
  static unsigned long time_second_bit_seen = 0;
  unsigned long new_estimated_cycle_time = 0;
  unsigned long now = micros();

  switch(state) {
    case other:
      // waiting for the accumulator to become blank.
      switch(accumulator()) {
        case 0:
          state = empty;
          time_of_last_state_change = now;
          break;
        default:
          phase_lock = false;
          cycle_time_age = 0;
          break;
      }
      break;
    case empty: // 0000
      switch(accumulator()) {
        case 0:
          break;
        case 8:
          state = alpha;
          time_of_last_state_change = now;
          break;
        default:
          state = other;
          break;
      }
      break;
    case alpha: // 1000
      switch(accumulator()) {
        case 0:
          break;
        case 12: // 1100
        case 8:
          break;
        case 4:
          time_first_bit_seen = micros();
          state = beta;
          time_of_last_state_change = now;
          break;
        default:
          state = other;
          phase_lock = false;
          cycle_time_age = 0;
          break;
      }
      break;
    case beta: // 0100
      switch(accumulator()) {
        case 0:
          break;
        case 6: // 0110
        case 4:
          break;
        case 2:
          time_second_bit_seen = micros();
          new_estimated_cycle_time = (time_second_bit_seen
            - time_first_bit_seen) / cycles_between_accumulator_bits;
          cycle_time = new_estimated_cycle_time;
          cycle_time_age++;
          state = gamma;
          time_of_last_state_change = now;
          break;
        default:
          state = other;
          phase_lock = false;
          cycle_time_age = 0;
          break;
      }
      break;
    case gamma: // 0010
      switch(accumulator()) {
        case 0:
          break;
        case 3: // 0011
        case 2:
          break;
        case 1:
          state = delta;
          time_of_last_state_change = now;
          break;
        default:
          state = other;
          phase_lock = false;
          cycle_time_age = 0;
          break;
      }
      break;
    case delta: // 0001
      switch(accumulator()) {
        case 1:
          break;
        case 0:
          state = empty;
          time_of_last_state_change = now;
          if (cycle_time_age > decision_level) {
            phase_lock = true;
          }
          break;
        default:
          state = other;
          phase_lock = false;
          cycle_time_age = 0;
          break;
      }
      break;
  }
  if (phase_lock) {
    if (now - time_of_last_state_change > too_long) {
      state = other;
      time_of_last_state_change = now;
      phase_lock = false;
      cycle_time_age = 0;
    }
  }
  return;
}

void loop() {
  unsigned long now = millis();
  static unsigned long initial_delay = now + 4000; // ms; four seconds from now

  cooperative_blink();
  cooperative_synchronized_flashing_indicator_mechanism();

  // if (now > initial_delay) {
    cooperative_phase_locked_loop();
  // }
  
  if (phase_lock) {
    if (button_pressed()) {
      attack();
    }
    steady_green();
  }
  else {
    flashing_green();
  }
} 
