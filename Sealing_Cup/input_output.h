#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);

//char panah
byte customChar[] = {
  B00000,
  B00100,
  B00010,
  B11111,
  B00010,
  B00100,
  B00000,
  B00000
};

// pin adc button
int btn_analog_pin = A0;

// pin sensor Limit Switch
const int ls_pin[3] = {A1, A2, A3};

// pin sensor infrared
int ir_pin = A7;

// pin led dan buzzer
const int led_pin = 13;
const int buz_pin = 12;

// motor power window pin
const int motor_pwm_pin = 6;
const int motor_left_pin = 7;
const int motor_right_pin = 8;

// setup io
void setup_input_output() {
  lcd.init();
  pinMode(btn_analog_pin, INPUT_PULLUP);
  
  pinMode(ls_pin[0], INPUT);
  pinMode(ls_pin[1], INPUT_PULLUP);
  pinMode(ls_pin[2], INPUT_PULLUP);
  
  pinMode(ir_pin, INPUT);
  pinMode(led_pin, OUTPUT);
  pinMode(buz_pin, OUTPUT);
  
  pinMode(motor_pwm_pin, OUTPUT);
  pinMode(motor_left_pin, OUTPUT);
  pinMode(motor_right_pin, OUTPUT);
  
  for (int i = 0; i < 8; i++) {
    digitalWrite(buz_pin, !digitalRead(buz_pin));
    digitalWrite(led_pin, !digitalRead(led_pin));
    delay(35);
  }
  lcd.createChar(0, customChar);
  lcd.backlight();
  lcd.setCursor(0, 0), lcd.print("   CUP SEALER");
  delay(2000);
  lcd.clear();
}

//fungsi run motor power window
void run_motor(int spd, int brk) {
  if (spd == 0 && brk == 1) {
    analogWrite(motor_pwm_pin, 255);
    digitalWrite(motor_left_pin, 0);
    digitalWrite(motor_right_pin, 0);
  }
  if (spd == 0 && brk == 0) {
    analogWrite(motor_pwm_pin, 0);
    digitalWrite(motor_left_pin, 0);
    digitalWrite(motor_right_pin, 0);
  }
  if (spd > 0) {
    analogWrite(motor_pwm_pin, spd);
    digitalWrite(motor_left_pin, 1);
    digitalWrite(motor_right_pin, 0);
  }
  if (spd < 0) {
    analogWrite(motor_pwm_pin, -spd);
    digitalWrite(motor_left_pin, 0);
    digitalWrite(motor_right_pin, 1);
  }
}

// fungsi blinking led
unsigned long prev_millis_led;
int led_state = LOW;
void led_blink(int on_time, int off_time) {
  unsigned long current_millis_led = millis();
  if ((led_state == 1) && (current_millis_led - prev_millis_led >= on_time)) {
    led_state = 0;
    prev_millis_led = current_millis_led;
  }
  else if ((led_state == 0) && (current_millis_led - prev_millis_led >= off_time)) {
    led_state = 1;
    prev_millis_led = current_millis_led;
  }
  digitalWrite(led_pin, led_state);
  //if(ch == 1) digitalWrite(buzzer_pin, led_state);
}

// fungsi beep buzzer 1 kali
void beep_once(int interval) {
  digitalWrite(buz_pin, HIGH);
  delay(interval);
  digitalWrite(buz_pin, LOW);
  delay(interval);
}

// fungsi baca sensor ir
bool ir_state() {
  boolean x;
  int ir_val = analogRead(ir_pin);
  if (ir_val < 50) x = 1;
  else x = 0;
  return x;
}

// infrared untuk counter cup
int last_state_ir = LOW;
int ir_counter = 0;
int total_cup = 0;
void counter_cup() {
  if (!last_state_ir && ir_state()) {
    ir_counter += 1;
    total_cup += 1;
  }
  last_state_ir = ir_state();
}

// fungsi baca sensor limit switch
int adc_ls[3];
boolean ls(int pin) {
  boolean x;
  for (int i = 0; i < 3; i++) {
    adc_ls[i] = analogRead(ls_pin[i]);
  }
  if (adc_ls[pin] < 100) x = 1;
  else x = 0;  
  return x;
}

//Push Button ADC
// Settingan sealer cup
int kalibrasi_btn;
bool mode_button_usb = true; // true while usb on
boolean button(int i) {
  if (mode_button_usb) {
    if (i == 3) kalibrasi_btn = -70;
    else kalibrasi_btn = -18;
  }
  if (!mode_button_usb) kalibrasi_btn = 5;
  boolean y = 1;
  int adcButton = analogRead(btn_analog_pin);
  if (i == 0) {
    if (adcButton >= 95 + kalibrasi_btn && adcButton <= 105 + kalibrasi_btn) y = 0;
    else y = 1;
  }
  if (i == 1) {
    if (adcButton >= 110 + kalibrasi_btn && adcButton <= 125 + kalibrasi_btn) y = 0;
    else y = 1;
  }
  if (i == 2) {
    if (adcButton >= 135 + kalibrasi_btn && adcButton <= 145 + kalibrasi_btn) y = 0;
    else y = 1;
  }
  if (i == 3) {
    if (adcButton >= 350 + kalibrasi_btn && adcButton <= 370 + kalibrasi_btn) y = 0;
    else y = 1;
  }
  // button 3 + button 2
  if (i == 32) {
    if (adcButton >= 380 + kalibrasi_btn && adcButton <= 400 + kalibrasi_btn) y = 0;
    else y = 1;
  }
  return y;
}


// fungsi dan variable long press button
const int short_press_time = 500;
const int long_press_time = 800;

int last_state_rtry = LOW;  // previous state
int current_state_rtry;     // current reading
unsigned long pressed_time = 0;
unsigned long realesed_time = 0;
bool is_pressing = false;
bool is_long_pressing = false;

bool long_press_btn = false;
bool short_press_btn = false;

void short_long_btn(int pin) {
  current_state_rtry = button(pin);
  if (last_state_rtry == HIGH && current_state_rtry == LOW) {
    pressed_time = millis();
    is_pressing = true;
    is_long_pressing = false;
  } else if (last_state_rtry == LOW && current_state_rtry == HIGH) {
    is_pressing = false;
    realesed_time = millis();
    long press_duration = realesed_time - pressed_time;
    if (press_duration < short_press_time) {
      long_press_btn = false;
      short_press_btn = true;
      Serial.print("short press ");
      Serial.println(pressed_time);
    }
  }
  if (is_pressing == true && is_long_pressing == false) {
    long press_duration = millis() - pressed_time;
    if (press_duration > long_press_time) {
      long_press_btn = true;
      short_press_btn = false;
      Serial.print("long press");
      Serial.println(pressed_time);
      is_long_pressing = true;
    }
  }
  last_state_rtry = current_state_rtry;
}

// fungsi refresh lcd 
int refresh;
void refresh_screen(int intervalRefresh) {
  if (++refresh > intervalRefresh) {
    lcd.clear();
    refresh = 0;
  }
}

