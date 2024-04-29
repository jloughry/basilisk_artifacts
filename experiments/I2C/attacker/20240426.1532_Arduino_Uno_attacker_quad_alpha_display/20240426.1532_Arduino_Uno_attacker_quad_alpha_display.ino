const unsigned int green_led_indicator_pin = 5;
const unsigned int red_led_indicator_pin = 6;
const unsigned int laser_SCL_pin = 9;
const unsigned int laser_SDA_pin = 8;
const unsigned int speed_test_photodiode = 10;
const unsigned int pushbutton = 11;

const unsigned int I2C_bit_interval = 10;

void setup() {
  pinMode(red_led_indicator_pin, OUTPUT);
  pinMode(green_led_indicator_pin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(laser_SCL_pin, OUTPUT);
  pinMode(laser_SDA_pin, OUTPUT);
  pinMode(pushbutton, INPUT_PULLUP);
  pinMode(speed_test_photodiode, INPUT);

  all_lasers_off();
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

bool error_state = false;

void cooperative_error_indication(void) {
  const unsigned int blink_interval = 300;
  static enum states {red, green} state = red;
  static unsigned long time_of_last_state_change = 0;

  if (error_state) {
    unsigned long now = millis();

    if (now - time_of_last_state_change > blink_interval) {
      switch(state) {
        case red:
          red_led_off();
          green_led_on();
          state = green;
          break;
        case green:
          green_led_off();
          red_led_on();
          state = red;
          break;
        default:
          red_led_on();
          green_led_off();
          state = red;
          break;
      }
      time_of_last_state_change = now;
    }
  }
  else {
    red_led_off();
    green_led_off();
  }
}

// This function never returns.

void error_indication(void) {
  while(true) {
    red_led_on();
    green_led_off();
    delay(300);
    red_led_off();
    green_led_on();
    delay(300);
  }
}

bool button_pressed(void) {
  return !digitalRead(pushbutton);
}

void laser_SDA_on(void) {
  digitalWrite(laser_SDA_pin, HIGH);
}

void laser_SDA_off(void) {
  digitalWrite(laser_SDA_pin, LOW);
}

void laser_SCL_on(void) {
  digitalWrite(laser_SCL_pin, HIGH);
}

void laser_SCL_off(void) {
  digitalWrite(laser_SCL_pin, LOW);
}

void all_lasers_off(void) {
  laser_SDA_off();
  laser_SCL_off();
}

void all_lasers_on(void) {
  laser_SDA_on();
  laser_SCL_on();
}

void aim_lasers(void) {
  static enum states {off, on} state = off;
  switch(state) {
    case on:
      if (button_pressed()) {
        state = off;
        delay(200); // debouncing
      }
      while (!button_pressed()) {
        all_lasers_on();
        delay(I2C_bit_interval);
        all_lasers_off();
        delay(200);
      }
      state = off;
      delay(200); // debouncing
      break;
    case off:
      if (button_pressed()) {
        state = on;
        delay(200); // debouncing
      }
      break;    
    default:
      state = off;
      break;
  }
}

void SDA_low(void) {
  laser_SDA_on();
}

void SDA_high(void) {
  laser_SDA_off();
}

void SCL_low(void) {
  laser_SCL_on();
}

void SCL_high(void) {
  laser_SCL_off();
}

void wait(void) {
  delay(I2C_bit_interval);
}

void wait_half(void) {
  delay(I2C_bit_interval / 2);
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

void cooperative_blink(void) {
  const unsigned int blink_interval = 1000;
  static enum states {off, on} state = off;
  static unsigned long time_of_last_state_change = 0;
  unsigned long now = millis();

  if (now - time_of_last_state_change > blink_interval) {
    switch(state) {
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

uint16_t displayRAM[8];

void show_on_address(uint8_t addr) {
  I2C_start_condition();
  I2C_write_byte((addr << 1) | 0); // 0 for write;
  I2C_write_byte(0); // tell it memory address 0
  for (int i=0; i<8; i++) {
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
  clear_displayRAM();
  character_P(1);
  character_R(2);
  character_O(3);
  character_O(4);
  show_on_address(0x70);
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
  show_on_address(0x71);
}

void danger_laser(void) {
  clear_displayRAM();
  character_D(1);
  character_A(2);
  character_N(3);
  character_G(4);
  show_on_address(0x70);
  clear_displayRAM();
  character_E(1);
  character_R(2);
  // space
  character_L(4);;
  show_on_address(0x72);
  clear_displayRAM();
  character_A(1);
  character_S(2);
  character_E(3);
  character_R(4);
  show_on_address(0x73);
  clear_displayRAM();
  // space
  character_star(2);
  character_dash(3);
  character_dash(4);
  show_on_address(0x71);
}

void do_not_look_into(void) {
  clear_displayRAM();
  character_D(1);
  character_O(2);
  // space
  character_N(4);
  show_on_address(0x70);
  clear_displayRAM();
  character_O(1);
  character_T(2);
  // space
  character_L(4);;
  show_on_address(0x72);
  clear_displayRAM();
  character_O(1);
  character_O(2);
  character_K(3);
  // space
  show_on_address(0x73);
  clear_displayRAM();
  character_I(1);
  character_N(2);
  character_T(3);
  character_O(4);
  show_on_address(0x71);
}

void remotely_expolit(void) {
  clear_displayRAM();
  character_R(1);
  character_E(2);
  character_M(3);
  character_O(4);
  show_on_address(0x70);
  delay(1000);
  clear_displayRAM();
  character_T(1);
  character_E(2);
  character_L(3);
  character_Y(4);
  show_on_address(0x72);
  delay(1000);
  clear_displayRAM();
  // space
  character_E(2);
  character_X(3);
  character_P(4);
  // space
  show_on_address(0x73);
  delay(1000);
  clear_displayRAM();
  character_L(1);
  character_O(2);
  character_I(3);
  character_T(4);
  show_on_address(0x71);
  delay(1000);
  clear_displayRAM();
}

void loop() {
  cooperative_blink();
  cooperative_error_indication();
  if (button_pressed()) {
    error_state = !error_state;
    delay(400); // for debouncing
  }
  /*
  // aim_lasers();
  if (button_pressed()) {
    proof_of_concept();
  }
  */
}
