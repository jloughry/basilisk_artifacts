#define TACTILE_SWITCH A0
#define EXTERNAL_HEADER_LASER A1
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

const unsigned int cycle_time = 8; // 10 ms for fast clock; 2000 for slow.

// Make state global so other can see it too.
enum FOI_states {off, on} FOI_state = off;

void cooperative_detect_rising_edge_of_FOI(void) {
  static unsigned long time_rising_edge_detected;
  unsigned long length_of_FOI = 0;

  if (!digitalRead(PHOTODIODE_ACC_3)) {
    if (off == FOI_state) {
      time_rising_edge_detected = millis(); // now
      FOI_state = on;
    }
  }
  else {
    if (on == FOI_state) {
      unsigned long now = millis();
      length_of_FOI = now - time_rising_edge_detected;
      FOI_state = off;
      Serial.print("length_of_FOI = ");
      Serial.print(length_of_FOI);
      Serial.println(" ms");
    }
  }
  return;
}

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

void laser_off(void) {
  digitalWrite(EXTERNAL_HEADER_LASER, LOW);
}

void laser_on(void) {
  digitalWrite(EXTERNAL_HEADER_LASER, HIGH);
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
  Serial.begin(9600);
  
  pinMode(EXTERNAL_HEADER_LASER, OUTPUT);
  laser_off();

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

void cooperative_test_all_lasers(void) {
  static unsigned int time_of_last_state_transition = 0;
  static unsigned int now;
  const unsigned int interval = 500;
  static int test_laser = 0;

  now = millis();
  if (now - time_of_last_state_transition < interval) {
    return;
  }
  else {
      switch (test_laser) {
      case 0:
        laser_0_on();
        laser_1_off();
        laser_2_off();
        laser_3_off();
        test_laser = 1;
        break;
      case 1:
        laser_0_off();
        laser_1_on();
        laser_2_off();
        laser_3_off();
        test_laser = 2;
        break;
      case 2:
        laser_0_off();
        laser_1_off();
        laser_2_on();
        laser_3_off();      
        test_laser = 3;
        break;
      case 3:
        laser_0_off();
        laser_1_off();
        laser_2_off();
        laser_3_on();      
        test_laser = 0;
        break;
      default:
        all_lasers_off();
        test_laser = 0;
        break;
    }
    time_of_last_state_transition = now;
  }
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

void wait_n_cycles(unsigned int n) {
  delay(cycle_time * n);
}

void wait_one_instruction(void) {
  wait_n_cycles(12);
}
void fire_lasers (int laser_3, int laser_2, int laser_1, int laser_0) {
  if (laser_3) laser_3_on();
  if (laser_2) laser_2_on();
  if (laser_1) laser_1_on();
  if (laser_0) laser_0_on();
  wait_n_cycles(1);
  all_lasers_off();
}

void loop() {
  unsigned int instruction = 0;
  unsigned int cycle = 0;
 
  // digitalWrite(LED_BUILTIN, !digitalRead(PHOTODIODE_ACC_3));

  cooperative_button_press_detector();
  // cooperative_lock_indicator();

  if (go) {
    green_led_on();
    
    // First telltale: write 0010 to address 1011.
    wait_for_rising_edge_of_bit(1);
    lock_led_on();
    wait_n_cycles(7); // seven cycles to end of SR instruction
    wait_n_cycles(3); // three cycles into NOP instruction
    fire_lasers(0, 1, 0, 0); // turn NOP into STA (0100)
    wait_n_cycles(4);
    fire_lasers(1, 0, 1, 1); // change address to 1011
    lock_led_off();

    // Second telltale: write 0110 to address 1100 (!ended up w/JMP)
    wait_for_rising_edge_of_bit(2);
    lock_led_on();
    wait_n_cycles(10);
    fire_lasers(0, 1, 0, 0); // Change NOP to STA
    wait_n_cycles(4);
    fire_lasers(1, 1, 0, 0); // Change address to 1100 in STA cycle 5.
    wait_n_cycles(1);
    fire_lasers(0, 0, 1, 0); // Change accumulator to 0110 in STA cycle 9.
    lock_led_off();

    // Third telltale: write 1011 to address 1101.
    wait_for_rising_edge_of_bit(0);
    lock_led_on();
    wait_n_cycles(10);
    fire_lasers(0, 1, 0, 0); // change NOP to STA
    wait_n_cycles(4);
    fire_lasers(1, 1, 0, 1); // change address to 1101
    wait_n_cycles(1);
    fire_lasers(1, 0, 1, 0); // change value from ACC (1000) to 1011.
    lock_led_off();

    // Now force a branch to address 1011.

    // This isn't working yet.

/*
    wait_n_cycles(4); // wait out the STA instruction.
    wait_n_cycles(3); // and then 4 cycles into the JMP instruction following
    fire_lasers(1, 0, 0, 1);
*/

    wait_for_rising_edge_of_bit(1); // accumulator contains 0010
    lock_led_on();
    wait_n_cycles(7); // wait out the SR instruction
    wait_n_cycles(3);
    fire_lasers(0, 1, 1, 0); // change NOP to JMP instruction
    wait_n_cycles(4);
    fire_lasers(1, 0, 1, 1); // change address to 1011
    lock_led_off();
    
    green_led_off();
    go = false;
  }
  
  // cooperative_detect_rising_edge_of_FOI();
  // cooperative_test_all_lasers();
}
