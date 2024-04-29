#include <Wire.h>
#include <SparkFun_Alphanumeric_Display.h>

const uint8_t blue_aiming_LED_0_pin = 5;
const uint8_t blue_aiming_LED_1_pin = 4;
const uint8_t blue_aiming_LED_2_pin = 3;
const uint8_t blue_aiming_LED_3_pin = 2;

const uint8_t yellow_aiming_LED_0_pin = 9;
const uint8_t yellow_aiming_LED_1_pin = 8;
const uint8_t yellow_aiming_LED_2_pin = 7;
const uint8_t yellow_aiming_LED_3_pin = 6;
const uint8_t pushbutton_switch_pin = 10;

#define SDA_sense_pin A1
#define SCL_sense_pin A0

HT16K33 display;

void scroll_onto_display(void);

bool all_displays_okay = false;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(yellow_aiming_LED_0_pin, OUTPUT);
  pinMode(yellow_aiming_LED_1_pin, OUTPUT);
  pinMode(yellow_aiming_LED_2_pin, OUTPUT);
  pinMode(yellow_aiming_LED_3_pin, OUTPUT);
  pinMode(blue_aiming_LED_0_pin, OUTPUT);
  pinMode(blue_aiming_LED_1_pin, OUTPUT);
  pinMode(blue_aiming_LED_2_pin, OUTPUT);
  pinMode(blue_aiming_LED_3_pin, OUTPUT);
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

bool button_pressed(void) {
  return !digitalRead(pushbutton_switch_pin);
}

void led_on(void) {
  digitalWrite(LED_BUILTIN, HIGH);
}

void led_off(void) {
  digitalWrite(LED_BUILTIN, LOW);
}

void yellow_aiming_LED_0_on(void) {
  digitalWrite(yellow_aiming_LED_0_pin, HIGH);
}

void yellow_aiming_LED_0_off(void) {
  digitalWrite(yellow_aiming_LED_0_pin, LOW);
}

void yellow_aiming_LED_1_on(void) {
  digitalWrite(yellow_aiming_LED_1_pin, HIGH);
}

void yellow_aiming_LED_1_off(void) {
  digitalWrite(yellow_aiming_LED_1_pin, LOW);
}

void yellow_aiming_LED_2_on(void) {
  digitalWrite(yellow_aiming_LED_2_pin, HIGH);
}

void yellow_aiming_LED_2_off(void) {
  digitalWrite(yellow_aiming_LED_2_pin, LOW);
}

void yellow_aiming_LED_3_on(void) {
  digitalWrite(yellow_aiming_LED_3_pin, HIGH);
}

void yellow_aiming_LED_3_off(void) {
  digitalWrite(yellow_aiming_LED_3_pin, LOW);
}

void blue_aiming_LED_0_on(void) {
  digitalWrite(blue_aiming_LED_0_pin, HIGH);
}

void blue_aiming_LED_0_off(void) {
  digitalWrite(blue_aiming_LED_0_pin, LOW);
}

void blue_aiming_LED_1_on(void) {
  digitalWrite(blue_aiming_LED_1_pin, HIGH);
}

void blue_aiming_LED_1_off(void) {
  digitalWrite(blue_aiming_LED_1_pin, LOW);
}

void blue_aiming_LED_2_on(void) {
  digitalWrite(blue_aiming_LED_2_pin, HIGH);
}

void blue_aiming_LED_2_off(void) {
  digitalWrite(blue_aiming_LED_2_pin, LOW);
}

void blue_aiming_LED_3_on(void) {
  digitalWrite(blue_aiming_LED_3_pin, HIGH);
}

void blue_aiming_LED_3_off(void) {
  digitalWrite(blue_aiming_LED_3_pin, LOW);
}

void all_LEDs_off(void) {
  led_off();
  yellow_aiming_LED_0_off();
  yellow_aiming_LED_1_off();
  yellow_aiming_LED_2_off();
  yellow_aiming_LED_3_off();
  blue_aiming_LED_0_off();
  blue_aiming_LED_1_off();
  blue_aiming_LED_2_off();
  blue_aiming_LED_3_off();
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
    blue_aiming_LED_3_on();
  }
  else {
    blue_aiming_LED_3_off();
  }

  if (SDA_voltage < decision_level_2) {
    blue_aiming_LED_2_on();
  }
  else {
    blue_aiming_LED_2_off();
  }

  if (SDA_voltage < decision_level_1) {
    blue_aiming_LED_1_on();
  }
  else {
    blue_aiming_LED_1_off();
  }

  if (SDA_voltage < decision_level_0) {
    blue_aiming_LED_0_on();
  }
  else {
    blue_aiming_LED_0_off();
  }

  if (SCL_voltage < decision_level_3) {
    yellow_aiming_LED_3_on();
  }
  else {
    yellow_aiming_LED_3_off();
  }

  if (SCL_voltage < decision_level_2) {
    yellow_aiming_LED_2_on();
  }
  else {
    yellow_aiming_LED_2_off();
  }

  if (SCL_voltage < decision_level_1) {
    yellow_aiming_LED_1_on();
  }
  else {
    yellow_aiming_LED_1_off();
  }

  if (SCL_voltage < decision_level_0) {
    yellow_aiming_LED_0_on();
  }
  else {
    yellow_aiming_LED_0_off();
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

void (* resetFunc)(void) = 0; // This is built-in name.

void loop() {
  cooperative_aiming_indicator();
  cooperative_blink();

/*
  if(button_pressed()) {
    scroll_onto_display();
  }
*/
  // I2C bus is quiet. The target is now ready to be attacked.
}
