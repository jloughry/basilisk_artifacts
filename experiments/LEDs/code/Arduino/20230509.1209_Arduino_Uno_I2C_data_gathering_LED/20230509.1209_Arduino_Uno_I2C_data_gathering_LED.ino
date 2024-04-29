#include <math.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "Adafruit_PWMServoDriver.h"

char wavelength_string[10] = "405nm";
char bus_voltage_string[10] = "undefined";
char pullup_value_string[10] = "undefined";

const bool bypass_laser_aiming_check = true;

const int SDA_bus_driver_pin = 6;
const int SCL_bus_driver_pin = 11;

const int relay_5V = 10;
const int relay_3_3V = 9;
const int relay_2_5V = 8;
const int relay_1_8V = 7;

const int relay_pullup_1k = 5;
const int relay_pullup_2_2k = 4;
const int relay_pullup_4_7k = 3;
const int relay_pullup_10k = 2;

#define voltage_measurement A3
#define red_led_indicator   A1
#define green_led_indicator A2
#define power_supply_measurement  A0

const int relay_laser = 12;
const int digital_bus_sense = 13;

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor * motor_x = AFMS.getStepper(200, 1); // NEMA 11
Adafruit_StepperMotor * motor_y = AFMS.getStepper(200, 2); // stepper motors

void all_relays_off(void);
void red_led_on();
void red_led_off();
void green_led_on();
void green_led_off();
void flash_leds();
void bus_voltage_5V(void);
void pullup_10k_ohms(void);
bool check_power_supplies(void);
bool check_aim(void);
double measure_bus_voltage(void);
double measure_power_supply(void);
void warm_up_ADCs(void);
void test_relay_bank(void);
void wait_for_keypress();

void setup() {
  pinMode(red_led_indicator, OUTPUT);
  pinMode(green_led_indicator, OUTPUT);
  
  pinMode(relay_5V, OUTPUT);
  pinMode(relay_3_3V, OUTPUT);
  pinMode(relay_2_5V, OUTPUT);
  pinMode(relay_1_8V, OUTPUT);
  
  pinMode(relay_pullup_1k, OUTPUT);  
  pinMode(relay_pullup_2_2k, OUTPUT);  
  pinMode(relay_pullup_4_7k, OUTPUT);  
  pinMode(relay_pullup_10k, OUTPUT);

  // For testing I2C bus for liveness.
  pinMode(SCL_bus_driver_pin, INPUT);
  pinMode(SDA_bus_driver_pin, INPUT);

  pinMode(voltage_measurement, INPUT); // used as analog input
  pinMode(power_supply_measurement, INPUT); // used as analog input
  pinMode(digital_bus_sense, INPUT); // used as a digital input

  for (int i=0; i<5; i++) {
    (void)analogRead(voltage_measurement);
    (void)analogRead(power_supply_measurement);
  }
  
  pinMode(relay_laser, OUTPUT);
  all_relays_off();
  red_led_off();
  green_led_off();

  Serial.begin(9600);

  // Initialize the motors right away since otherwise they can get stuck and overheat.
  if (!AFMS.begin()) {
    Serial.println("Could not find motor shield. Check wiring.");
    while(1) ;
  }
  motor_x->release();
  motor_y->release();

  // Wait for a keypress on the console.
  Serial.println("Enter to start.");
  wait_for_keypress();
  green_led_on();

  test_relay_bank();
  
  TWBR = ((F_CPU / 400000L) - 16) / 2; // Change the i2c clock to 400 kHz.
  
  motor_x->setSpeed(300); // RPM
  motor_y->setSpeed(300); // This is about as fast as it will go.
  Serial.println("Steppers ready.");

  Serial.print("Wavelength: ");
  Serial.println(wavelength_string);
  
  warm_up_ADCs();

  if (check_power_supplies()) {
    Serial.println("Power supplies good.");
  }
  else {
    Serial.println("Problem: check power supplies and reset.");
    green_led_off();
    while(1) ;
  }
  if (check_aim()) {
    Serial.println("Aiming is good.");
  }
  else {
    if (bypass_laser_aiming_check) {
      Serial.println("Laser aiming check bypassed.");
    }
    else {
      Serial.println("Problem: check laser aim and reset.");
      motor_x->release();
      motor_y->release(); // Release the motors so we can move the laser to re-aim.
      while(1) ;
    }
  }
}

void wait_for_keypress(void) {
  while (!Serial.available()) ;
  (void)Serial.read();
}

void LED_on(void) {
  digitalWrite(LED_BUILTIN, HIGH);
}

void LED_off(void) {
  digitalWrite(LED_BUILTIN, LOW);
}

void cooperative_blink(void) {
  const unsigned int blink_interval = 1000;
  static enum states {off, on} state = off;
  static unsigned long time_of_last_state_change = 0;
  unsigned long now = millis();

  if (now - time_of_last_state_change > blink_interval) {
    switch (state) {
      case off:
        LED_on();
        state = on;
        break;
      case on:
        LED_off();
        state = off;
        break;
      default:
        LED_off();
        state = off;
        break;
    }
    time_of_last_state_change = now;
  }
}

void red_led_on(void) {
  digitalWrite(red_led_indicator, HIGH);
}

void red_led_off(void) {
  digitalWrite(red_led_indicator, LOW);
}

void green_led_on(void) {
  digitalWrite(green_led_indicator, HIGH);
}

void green_led_off(void) {
  digitalWrite(green_led_indicator, LOW);
}

/*
 * This function is a little special; it never returns.
 */

void flash_leds(void) {
  while(1) {
    red_led_on();
    delay(150);
    red_led_off();
    green_led_on();
    delay(150);
    green_led_off();
  }
}

void bus_voltage_5V(void) {
  bus_voltage_3_3V_off();
  bus_voltage_2_5V_off();
  bus_voltage_1_8V_off();
  strcpy(bus_voltage_string, "5V");
  delay(250);
  digitalWrite(relay_5V, LOW);
  delay(250);
  (void)measure_bus_voltage();
}

void bus_voltage_5V_off(void) {
  digitalWrite(relay_5V, HIGH);
}

void bus_voltage_3_3V(void) {
  bus_voltage_5V_off();
  bus_voltage_2_5V_off();
  bus_voltage_1_8V_off();
  strcpy(bus_voltage_string, "3.3V");
  delay(250);
  digitalWrite(relay_3_3V, LOW);
  delay(250);
  (void)measure_bus_voltage();
}

void bus_voltage_3_3V_off(void) {
  digitalWrite(relay_3_3V, HIGH);
}

void bus_voltage_2_5V(void) {
  bus_voltage_5V_off();
  bus_voltage_3_3V_off();
  bus_voltage_1_8V_off();
  strcpy(bus_voltage_string, "2.5V");
  delay(250);
  digitalWrite(relay_2_5V, LOW);
  delay(250);
  (void)measure_bus_voltage();
}

void bus_voltage_2_5V_off(void) {
  digitalWrite(relay_2_5V, HIGH);
}

void bus_voltage_1_8V(void) {
  bus_voltage_5V_off();
  bus_voltage_3_3V_off();
  bus_voltage_2_5V_off();
  strcpy(bus_voltage_string, "1.8V");
  delay(250);
  digitalWrite(relay_1_8V, LOW);
  delay(250);
  (void)measure_bus_voltage();
}

void bus_voltage_1_8V_off(void) {
  digitalWrite(relay_1_8V, HIGH);
}

void pullup_1k_ohms(void) {
  pullup_2_2k_off();
  pullup_4_7k_off();
  pullup_10k_off();
  strcpy(pullup_value_string, "1kΩ");
  digitalWrite(relay_pullup_1k, LOW);
}

void pullup_1k_off(void) {
  digitalWrite(relay_pullup_1k, HIGH);
}

void pullup_2_2k_ohms(void) {
  pullup_1k_off();
  pullup_4_7k_off();
  pullup_10k_off();
  strcpy(pullup_value_string, "2.2kΩ");
  digitalWrite(relay_pullup_2_2k, LOW);
}

void pullup_2_2k_off(void) {
  digitalWrite(relay_pullup_2_2k, HIGH);
}

void pullup_4_7k_ohms(void) {
  pullup_1k_off();
  pullup_2_2k_off();
  pullup_10k_off();
  strcpy(pullup_value_string, "4.7kΩ");
  digitalWrite(relay_pullup_4_7k, LOW);
}

void pullup_4_7k_off(void) {
  digitalWrite(relay_pullup_4_7k, HIGH);
}

void pullup_10k_ohms(void) {
  pullup_1k_off();
  pullup_2_2k_off();
  pullup_4_7k_off();
  strcpy(pullup_value_string, "10kΩ");
  digitalWrite(relay_pullup_10k, LOW);
}

void pullup_10k_off(void) {
  digitalWrite(relay_pullup_10k, HIGH);
}

void laser_on(void) {
  digitalWrite(relay_laser, LOW);
  red_led_on();
}

void laser_off(void) {
  digitalWrite(relay_laser, HIGH);
  red_led_off();
}

void all_relays_off() {
  bus_voltage_5V_off();
  bus_voltage_3_3V_off();
  bus_voltage_2_5V_off();
  bus_voltage_1_8V_off();
  pullup_1k_off();
  pullup_2_2k_off();
  pullup_4_7k_off();
  pullup_10k_off();
  laser_off();
}

void test_relay_bank(void) {
  // All this does is make a distinctive noise.
  const unsigned int wait = 300;
  digitalWrite(relay_5V, LOW);
  delay(wait);
  digitalWrite(relay_5V, HIGH);
  delay(wait);
  digitalWrite(relay_3_3V, LOW);
  delay(wait);
  digitalWrite(relay_3_3V, HIGH);
  delay(wait);
  digitalWrite(relay_2_5V, LOW);
  delay(wait);
  digitalWrite(relay_2_5V, HIGH);
  delay(wait);
  digitalWrite(relay_1_8V, LOW);
  delay(wait);
  digitalWrite(relay_1_8V, HIGH);
  delay(wait);
  digitalWrite(relay_pullup_1k, LOW);
  delay(wait);
  digitalWrite(relay_pullup_1k, HIGH);
  delay(wait);
  digitalWrite(relay_pullup_2_2k, LOW);
  delay(wait);
  digitalWrite(relay_pullup_2_2k, HIGH);
  delay(wait);
  digitalWrite(relay_pullup_4_7k, LOW);
  delay(wait);
  digitalWrite(relay_pullup_4_7k, HIGH);
  delay(wait);
  digitalWrite(relay_pullup_10k, LOW);
  delay(wait);
  digitalWrite(relay_pullup_10k, HIGH);
  delay(wait);
}

bool check_power_supplies(void) {
  bool measured_5V_ok = false;
  bool measured_3_3V_ok = false;
  bool measured_2_5V_ok = false;
  bool measured_1_8V_ok = false;
  double measured_5V = 0.0;
  double measured_3_3V = 0.0;
  double measured_2_5V = 0.0;
  double measured_1_8V = 0.0;
  const double threshold = 0.1; // how close the power voltages should be, in volts

  pullup_10k_ohms();

  bus_voltage_5V();
  delay(250);
  measured_5V = measure_power_supply();
  if (fabs(5.0 - measured_5V) < threshold) {
    measured_5V_ok = true;
  }

  bus_voltage_3_3V();
  delay(250);
  measured_3_3V = measure_power_supply();
  if (fabs(3.3 - measured_3_3V) < threshold) {
    measured_3_3V_ok = true;
  }

  bus_voltage_2_5V();
  delay(250);
  measured_2_5V = measure_power_supply();
  if (fabs(2.5 - measured_2_5V) < threshold) {
    measured_2_5V_ok = true;
  }

  bus_voltage_1_8V();
  delay(250);
  measured_1_8V = measure_power_supply();
  if (fabs(1.8 - measured_1_8V) < threshold) {
    measured_1_8V_ok = true;
  }

  all_relays_off();

  bool report_all_values = true;
  
  bool verdict = measured_5V_ok && measured_3_3V_ok && measured_2_5V_ok && measured_1_8V_ok;
  if (!verdict || report_all_values) {
    Serial.print("5V power supply measured at ");
    Serial.print(measured_5V);
    Serial.println("V");
    Serial.print("3.3V power supply measured at ");
    Serial.print(measured_3_3V);
    Serial.println("V");
    Serial.print("2.5V power supply measured at ");
    Serial.print(measured_2_5V);
    Serial.println("V");
    Serial.print("1.8V power supply measured at ");
    Serial.print(measured_1_8V);
    Serial.println("V");
  }
  return verdict;
}

bool check_aim(void) {
  bool is_laser_aim_ok = false;
  const double threshold = 2.0; // how far down the bus should be depressedm, in volts
  
  bus_voltage_3_3V();
  pullup_10k_ohms();
  laser_on();
  delay(500);
  if (measure_bus_voltage() < threshold) {
    is_laser_aim_ok = true;
  }

  if (is_laser_aim_ok) {
    all_relays_off(); // leave laser on and bus powered if re-aiming is needed.
  }
  return is_laser_aim_ok;
}

void release_SDA(void) {
  pinMode(SDA_bus_driver_pin, INPUT);
}

void assert_SDA_low(void) {
  pinMode(SDA_bus_driver_pin, OUTPUT);
  digitalWrite(SDA_bus_driver_pin, LOW);
}

void release_SCL(void) {
  pinMode(SCL_bus_driver_pin, INPUT);
}

void assert_SCL_low(void) {
  pinMode(SCL_bus_driver_pin, OUTPUT);
  digitalWrite(SCL_bus_driver_pin, LOW);
}

const unsigned long I2C_bit_interval = 1; // ms

void wait(void) {
  delay(I2C_bit_interval);
}

void wait_half(void) {
  delay(I2C_bit_interval / 2);
}

void I2C_start_condition(void) {
  // precondition: SDA and SCL are both high.
  assert_SDA_low();
  wait();
  assert_SCL_low();
  // postcondition: SDA and SCL are both low.
}

void I2C_stop_condition(void) {
  // precondition: SDA is high; SCL is low.
  assert_SDA_low();
  wait();
  release_SCL();
  wait();
  release_SDA();
  wait();
  // postcondition: SDA and SCL are both high.
}

// Write a bit to I2C bus
void I2C_send_bit(bool bit) {
  // precondition: SCL is low
  wait_half();
  // change SDA only when SCL is low.
  if (bit) {
    release_SDA();
  } else {
    assert_SDA_low();
  }
  wait_half();
  release_SCL();
  wait();
  assert_SCL_low();
  // postcondition: SCL is low
}

void I2C_wait_for_ACK(void) {
  // precondtion: SCL is low.
  // SDA should only change when SCL is low. 
  wait_half();
  release_SDA(); // release SDA
  wait_half();
  release_SCL();
  wait();
  assert_SCL_low();
  // postcondition: SCL is low.
}

// Write a byte to I2C bus. Return 0 if ack by the target.
bool I2C_send_byte(unsigned char byte) {
  unsigned bit;
  // precondition: SCL is low.
  for (bit = 0; bit < 8; ++bit) {
    I2C_send_bit((byte & 0x80) != 0);
    byte <<= 1;
  }
  I2C_wait_for_ACK();
  // postcondition: SCL is low.
}

void turn_SparkFun_relay_off(uint8_t addr) {
  I2C_start_condition();
  I2C_send_byte((addr << 1) | 0); // 0 for write
  I2C_send_byte(0); // 0 means turn off
  I2C_stop_condition();
}

void turn_SparkFun_relay_on(uint8_t addr) {
  I2C_start_condition();
  I2C_send_byte((addr << 1) | 0); // 0 for write
  I2C_send_byte(1); // 1 means turn on
  I2C_stop_condition();
}

void test_I2C_bus(void) {
  bus_voltage_3_3V();
  pullup_10k_ohms();
  for (int i=0; i<2; i++) {
    turn_SparkFun_relay_on(0x18); // really ought to query relay state here
    delay(1000);
    turn_SparkFun_relay_off(0x18); // and here to be sure
    delay(1000);
  }
  Serial.println("I2C bus is live.");
}

int run_number = 1;

double x_position = 0.0; // mm
double y_position = 0.0; // mm
double bus_voltage = 0.0; // V
double pullup_value = 0.0; // ohms

// These are the configurable parameters:
const double raster_width = 5.0; // mm
const double raster_height = 5.0; // mm
const int number_of_samples = 50;

const int steps_per_mm = 200; // determined by screw pitch in the linear stage
const int samples_per_line = number_of_samples;
const int number_of_lines = number_of_samples;
const int steps_between_samples = raster_width * steps_per_mm / samples_per_line;

void warm_up_ADCs(void) {
  Serial.println("Warming up the ADCs.");
  for (int i=0; i<5; i++) {
    (void)analogRead(voltage_measurement);
    delay(100);
    (void)analogRead(voltage_measurement);
    delay(100);
  }
}

double measure_bus_voltage(void) {
  const unsigned number_of_samples_averaged = 5;
  int sensor_value_accumulator = 0;
  for (int i=0; i<number_of_samples_averaged; i++) {
    sensor_value_accumulator += analogRead(voltage_measurement);
  }
  double sensor_value = sensor_value_accumulator / number_of_samples_averaged;
  double voltage = (sensor_value + 0.5) * 5.0 / 1024.0;
  return voltage;
}

double measure_power_supply(void) {
  const unsigned number_of_samples_averaged = 5;
  int sensor_value_accumulator = 0;
  for (int i=0; i<number_of_samples_averaged; i++) {
    sensor_value_accumulator += analogRead(power_supply_measurement);
  }
  double sensor_value = sensor_value_accumulator / number_of_samples_averaged;
  double voltage = (sensor_value + 0.5) * 5.0 / 1024.0;
  return voltage;
}

// In the following stepper motor commands, FORWARD and REVERSE are swapped because wires are reversed.

void move_to_raster_upper_left_corner(void) {
  // precondition: laser is centered over target
  // postcondition: laser is at upper left corner of raster scan
  laser_off();
  motor_x->step((int)(0.5 * raster_width * steps_per_mm), FORWARD, MICROSTEP); // -1mm
  x_position = -1.0 * (raster_width / 2.0);
  motor_y->step((int)(0.5 * raster_width * steps_per_mm), FORWARD, MICROSTEP); // -1mm
  y_position = -1.0 * (raster_height / 2.0);
  motor_x->release();
  motor_y->release();
}

void return_to_center(void) {
  // precondition: we are at the lower left corner of raster scan.
  // postcondition: the laser is centered over the target.
  laser_off();
  motor_x->step((int)(0.5 * raster_width * steps_per_mm), BACKWARD, MICROSTEP); // +1mm
  x_position = 0.0;
  motor_y->step((int)(0.5 * raster_width * steps_per_mm), FORWARD, MICROSTEP); // -1mm
  y_position = 0.0;
  motor_x->release();
  motor_y->release();
}

void horizontal_retrace(void) {
  // precondition: we are at the end of a scan line
  motor_x->step((int)(raster_width * steps_per_mm), FORWARD, MICROSTEP);
  x_position = -1.0 * (raster_width / 2.0);
  motor_x->release();
}

void legend(void) {
  Serial.print("Scanned area is ");
  Serial.print(raster_width);
  Serial.print(" by ");
  Serial.print(raster_height);
  Serial.print(" mm. Resolution is ");
  Serial.print((int)(raster_width / number_of_samples * 1000));
  Serial.println(" µm.");
  
  Serial.println();
  Serial.print("Legend: ");
  Serial.print("run_number");
  Serial.print(",");
  Serial.print("wavelength"); // nm
  Serial.print(",");
  Serial.print("bus_voltage"); // V
  Serial.print(",");
  Serial.print("pullup_value"); // ohms
  Serial.print(",");
  Serial.print("x_position");
  Serial.print(",");
  Serial.print("y_position");
  Serial.print(",");
  Serial.print("measured_bus_voltage");
  Serial.print(",");
  Serial.println("logic_level");
  Serial.println();
}

void line_scan(void) {
  // precondition: we are at the beginning of a scan line.
  // postcondition: we are at the end of a scan line.
  laser_on();
  for (int sample = 0; sample < samples_per_line; sample++) {
    Serial.print(run_number);
    Serial.print(",");
    Serial.print(wavelength_string);
    Serial.print(",");
    Serial.print(bus_voltage_string);
    Serial.print(",");
    Serial.print(pullup_value_string);
    Serial.print(",");
    Serial.print(y_position); // transpose X and Y here because it was wired backwards
    Serial.print(",");
    Serial.print(x_position); // transpose X and Y to compendate.
    Serial.print(",");
    Serial.print(measure_bus_voltage()); // actually make the measurement
    Serial.print(",");
    Serial.println(digitalRead(digital_bus_sense));
    delay(100);
    motor_x->step(steps_between_samples, BACKWARD, MICROSTEP);
    x_position += (raster_width / samples_per_line);
  }
  laser_off();
}

void report(void) {
  // precondition: laser is centered on target
  // postcondition: laser is centered on target
  move_to_raster_upper_left_corner();
  for (int line = 0; line < number_of_lines; line++) {
    line_scan();
    motor_y->step(steps_between_samples, BACKWARD, MICROSTEP);
    y_position += (raster_height / number_of_lines);
    delay(100);
    horizontal_retrace();
  }
  return_to_center();
  ++run_number;
  
  Serial.println();
}

void gather_all_data(void) {
  legend();  

  bus_voltage_5V();
  pullup_1k_ohms();
  report();

  bus_voltage_5V();
  pullup_2_2k_ohms();
  report();

  bus_voltage_5V();
  pullup_4_7k_ohms();
  report();

  bus_voltage_5V();
  pullup_10k_ohms();
  report();

  bus_voltage_3_3V();
  pullup_1k_ohms();
  report();

  bus_voltage_3_3V();
  pullup_2_2k_ohms();
  report();

  bus_voltage_3_3V();
  pullup_4_7k_ohms();
  report();

  bus_voltage_3_3V();
  pullup_10k_ohms();
  report();

  bus_voltage_2_5V();
  pullup_1k_ohms();
  report();

  bus_voltage_2_5V();
  pullup_2_2k_ohms();
  report();

  bus_voltage_2_5V();
  pullup_4_7k_ohms();
  report();

  bus_voltage_2_5V();
  pullup_10k_ohms();
  report();

  bus_voltage_1_8V();
  pullup_1k_ohms();
  report();

  bus_voltage_1_8V();
  pullup_2_2k_ohms();
  report();

  bus_voltage_1_8V();
  pullup_4_7k_ohms();
  report();

  bus_voltage_1_8V();
  pullup_10k_ohms();
  report();

  all_relays_off();

  Serial.println("End of run.");
}

void loop() {
  // cooperative_blink();
  // test_I2C_bus();

  gather_all_data();
  flash_leds(); // never returns
}
