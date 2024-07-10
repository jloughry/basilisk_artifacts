#include <Wire.h>
#include <SparkFun_Alphanumeric_Display.h>

const uint8_t blue_bargraph_aiming_LED_0_pin = 5;
const uint8_t blue_bargraph_aiming_LED_1_pin = 4;
const uint8_t blue_bargraph_aiming_LED_2_pin = 3;
const uint8_t blue_bargraph_aiming_LED_3_pin = 2;

const uint8_t yellow_bargraph_aiming_LED_0_pin = 9;
const uint8_t yellow_bargraph_aiming_LED_1_pin = 8;
const uint8_t yellow_bargraph_aiming_LED_2_pin = 7;
const uint8_t yellow_bargraph_aiming_LED_3_pin = 6;

const uint8_t pushbutton_switch_pin = 10;

#define SDA_sense_pin A1
#define SCL_sense_pin A0

HT16K33 display;

void scroll_onto_display(void);

bool all_displays_okay = false;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(yellow_bargraph_aiming_LED_0_pin, OUTPUT);
  pinMode(yellow_bargraph_aiming_LED_1_pin, OUTPUT);
  pinMode(yellow_bargraph_aiming_LED_2_pin, OUTPUT);
  pinMode(yellow_bargraph_aiming_LED_3_pin, OUTPUT);
  pinMode(blue_bargraph_aiming_LED_0_pin, OUTPUT);
  pinMode(blue_bargraph_aiming_LED_1_pin, OUTPUT);
  pinMode(blue_bargraph_aiming_LED_2_pin, OUTPUT);
  pinMode(blue_bargraph_aiming_LED_3_pin, OUTPUT);
  pinMode(pushbutton_switch_pin, INPUT_PULLUP);
  pinMode(SDA_sense_pin, INPUT);
  pinMode(SCL_sense_pin, INPUT);
  all_LEDs_off();
  Wire.begin();
  all_displays_okay = display.begin(0x71, 0x72, 0x73, 0x70);
  // while (!all_displays_okay) ; // freeze if there's a problem.
  display.setBrightness(1); // as dim as it gets
  scroll_onto_display();
}

enum button_actions {single_click, double_click, long_press} selected_behavior = single_click;

bool simple_button_pressed(void) {
  return !digitalRead(pushbutton_switch_pin);
}

void debounce(void) {
  delay(50);
}

bool button_pressed_or_long_press(void) {
  if (simple_button_pressed()) {
    unsigned long time_button_first_pressed = millis();
    debounce();
    while (simple_button_pressed()) {
      const unsigned long long_press_interval = 1000; // one second
      if ((millis() - time_button_first_pressed) > long_press_interval) {
        selected_behavior = long_press;
        return true;
      }
    }
    debounce();
    selected_behavior = single_click;
    // but wait a little longer in case we got a double click
    unsigned long started_waiting_for_double_click = millis();
    const unsigned long double_click_interval = 500; // half a second
    while ((millis() - started_waiting_for_double_click) < double_click_interval) {
      if (simple_button_pressed()) {
        selected_behavior = double_click;
        break;
      }
    }
    debounce();
    delay(750); // short pause to ensure double click is not followed by a spurious click
    return true;
  }
  return false;
}

void led_on(void) {
  digitalWrite(LED_BUILTIN, HIGH);
}

void led_off(void) {
  digitalWrite(LED_BUILTIN, LOW);
}

void yellow_bargraph_aiming_LED_0_on(void) {
  digitalWrite(yellow_bargraph_aiming_LED_0_pin, HIGH);
}

void yellow_bargraph_aiming_LED_0_off(void) {
  digitalWrite(yellow_bargraph_aiming_LED_0_pin, LOW);
}

void yellow_bargraph_aiming_LED_1_on(void) {
  digitalWrite(yellow_bargraph_aiming_LED_1_pin, HIGH);
}

void yellow_bargraph_aiming_LED_1_off(void) {
  digitalWrite(yellow_bargraph_aiming_LED_1_pin, LOW);
}

void yellow_bargraph_aiming_LED_2_on(void) {
  digitalWrite(yellow_bargraph_aiming_LED_2_pin, HIGH);
}

void yellow_bargraph_aiming_LED_2_off(void) {
  digitalWrite(yellow_bargraph_aiming_LED_2_pin, LOW);
}

void yellow_bargraph_aiming_LED_3_on(void) {
  digitalWrite(yellow_bargraph_aiming_LED_3_pin, HIGH);
}

void yellow_bargraph_aiming_LED_3_off(void) {
  digitalWrite(yellow_bargraph_aiming_LED_3_pin, LOW);
}

void blue_bargraph_aiming_LED_0_on(void) {
  digitalWrite(blue_bargraph_aiming_LED_0_pin, HIGH);
}

void blue_bargraph_aiming_LED_0_off(void) {
  digitalWrite(blue_bargraph_aiming_LED_0_pin, LOW);
}

void blue_bargraph_aiming_LED_1_on(void) {
  digitalWrite(blue_bargraph_aiming_LED_1_pin, HIGH);
}

void blue_bargraph_aiming_LED_1_off(void) {
  digitalWrite(blue_bargraph_aiming_LED_1_pin, LOW);
}

void blue_bargraph_aiming_LED_2_on(void) {
  digitalWrite(blue_bargraph_aiming_LED_2_pin, HIGH);
}

void blue_bargraph_aiming_LED_2_off(void) {
  digitalWrite(blue_bargraph_aiming_LED_2_pin, LOW);
}

void blue_bargraph_aiming_LED_3_on(void) {
  digitalWrite(blue_bargraph_aiming_LED_3_pin, HIGH);
}

void blue_bargraph_aiming_LED_3_off(void) {
  digitalWrite(blue_bargraph_aiming_LED_3_pin, LOW);
}

void all_LEDs_off(void) {
  led_off();
  yellow_bargraph_aiming_LED_0_off();
  yellow_bargraph_aiming_LED_1_off();
  yellow_bargraph_aiming_LED_2_off();
  yellow_bargraph_aiming_LED_3_off();
  blue_bargraph_aiming_LED_0_off();
  blue_bargraph_aiming_LED_1_off();
  blue_bargraph_aiming_LED_2_off();
  blue_bargraph_aiming_LED_3_off();
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

// This function should be called from loop():

void cooperative_aiming_indicator(void) {
  float SDA_voltage = 3.3 * (float)analogRead(SDA_sense_pin) / 1024.0;
  float SCL_voltage = 3.3 * (float)analogRead(SCL_sense_pin) / 1024.0;
  const float decision_level_3 = 3.0; // volts
  const float decision_level_2 = 2.0; // volts
  const float decision_level_1 = 1.0; // volts
  const float decision_level_0 = 0.5; // volts

  if (SDA_voltage < decision_level_3) {
    blue_bargraph_aiming_LED_3_on();
  }
  else {
    blue_bargraph_aiming_LED_3_off();
  }

  if (SDA_voltage < decision_level_2) {
    blue_bargraph_aiming_LED_2_on();
  }
  else {
    blue_bargraph_aiming_LED_2_off();
  }

  if (SDA_voltage < decision_level_1) {
    blue_bargraph_aiming_LED_1_on();
  }
  else {
    blue_bargraph_aiming_LED_1_off();
  }

  if (SDA_voltage < decision_level_0) {
    blue_bargraph_aiming_LED_0_on();
  }
  else {
    blue_bargraph_aiming_LED_0_off();
  }

  if (SCL_voltage < decision_level_3) {
    yellow_bargraph_aiming_LED_3_on();
  }
  else {
    yellow_bargraph_aiming_LED_3_off();
  }

  if (SCL_voltage < decision_level_2) {
    yellow_bargraph_aiming_LED_2_on();
  }
  else {
    yellow_bargraph_aiming_LED_2_off();
  }

  if (SCL_voltage < decision_level_1) {
    yellow_bargraph_aiming_LED_1_on();
  }
  else {
    yellow_bargraph_aiming_LED_1_off();
  }

  if (SCL_voltage < decision_level_0) {
    yellow_bargraph_aiming_LED_0_on();
  }
  else {
    yellow_bargraph_aiming_LED_0_off();
  }

  return;
}

void scroll_onto_display(void) {
  const int speed = 110; // ms; smaller is faster

  display.print("               N");
  delay(speed);
  display.print("              NO");
  delay(speed);
  display.print("             NOR");
  delay(speed);
  display.print("            NORM");
  delay(speed);
  display.print("           NORMA");
  delay(speed);
  display.print("          NORMAL");
  delay(speed);
  display.print("         NORMAL ");
  delay(speed);
  display.print("        NORMAL O");
  delay(speed);
  display.print("       NORMAL OP");
  delay(speed);
  display.print("      NORMAL OPE");
  delay(speed);
  display.print("     NORMAL OPER");
  delay(speed);
  display.print("    NORMAL OPERA");
  delay(speed);
  display.print("   NORMAL OPERAT");
  delay(speed);
  display.print("  NORMAL OPERATI");
  delay(speed);
  display.print(" NORMAL OPERATIO");
  delay(speed);
  display.print("NORMAL OPERATION"); 
}

void keep_scrolling(void) {
  const int speed = 110; // ms; smaller is faster

  display.print("ORMAL OPERATION ");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("RMAL OPERATION  "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("MAL OPERATION   "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("AL OPERATION    "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("L OPERATION     "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print(" OPERATION      "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("OPERATION       "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("PERATION        "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("ERATION         "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("RATION          "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("ATION           "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("TION            "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("ION             "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("ON              "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("N               "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("                "); 
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("               N");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("              NO");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("             NOR");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("            NORM");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("           NORMA");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("          NORMAL");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("         NORMAL ");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("        NORMAL O");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("       NORMAL OP");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("      NORMAL OPE");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("     NORMAL OPER");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("    NORMAL OPERA");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("   NORMAL OPERAT");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("  NORMAL OPERATI");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print(" NORMAL OPERATIO");
  delay(speed);
  if (simple_button_pressed()) {
    return;
  }
  display.print("NORMAL OPERATION"); 
  delay(speed);
}


void continuous_scroll(void) {
  // keep scrolling until the button is pressed, but only after it been released.
  while (simple_button_pressed()) {
    scroll_onto_display();
  }
  debounce();
  while (!simple_button_pressed()) {
    keep_scrolling();
  }
  debounce();
  return;
}

void (* resetFunc)(void) = 0; // This is built-in name.

void redisplay_timeout(void) {
  const unsigned long redisplay_timeout_interval = 60000; // one minute
  unsigned long now = millis();
  static unsigned long time_of_last_redisplay = 0;

  if (now - time_of_last_redisplay > redisplay_timeout_interval) {
    scroll_onto_display();
    time_of_last_redisplay = now;
  }
}

void loop() {
  cooperative_aiming_indicator();
  cooperative_blink();

  if (button_pressed_or_long_press()) {
    switch (selected_behavior) {
      case single_click:
        scroll_onto_display();
        break;
      case double_click:
        continuous_scroll();
      default:
        break;
    }
  }

  if(simple_button_pressed()) {
    scroll_onto_display();
  }

  redisplay_timeout();

  // I2C bus is quiet. The target is now ready to be attacked.
}
