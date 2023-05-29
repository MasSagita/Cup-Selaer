#include <EEPROM.h>

#include "input_output.h"

int pw_speed; // variable speed power window

int speed_in, speed_out;

void (* resetFunc) (void) = 0;

int in_delay, set_in_delay;
int out_delay, set_out_delay;

void setup() {
  // put your setup code here, to run once:
  pw_speed = EEPROM.read(0);
  set_in_delay = EEPROM.read(1);
  set_out_delay = EEPROM.read(2);
  speed_in = EEPROM.read(3);
  speed_out = EEPROM.read(4);

  in_delay = set_in_delay * 100;
  out_delay = set_out_delay * 100;

  Serial.begin(115200);
  Serial.println("CUP SEALER");
  setup_input_output();
  homing_front();
}

int menu;
int cursor_menu;

int depan = 0, belakang = 1, atas = 2;

int counter_tes;

int test_menu = 0;

void loop() {
  digitalWrite(led_pin, !digitalRead(led_pin));
  refresh_screen(5);
  short_long_btn(2);
  test_menu = 0;

  lcd.setCursor(0, 0), lcd.print(F("    Stand By"));
  lcd.setCursor(5, 1), lcd.print(total_cup), lcd.print(F("cup"));
  if (is_long_pressing) setting_menu();

  counter_tes++;
  if (counter_tes > 10) counter_tes = 0;
  lcd.setCursor(14, 1), lcd.print(counter_tes);
  counter_cup();
  go_sealing();

}

// move inside
void go_sealing() {
  if (ls(atas) && ir_state() || test_menu == 1 && !button(3)) {
    led_blink(100, 100);
    lcd.clear();
    lcd.setCursor(5, 0), lcd.print(F("Waiting"));
    delay(in_delay);
    beep_once(50);
    // 90 adalah kecepatan yang ideal
    run_motor(speed_in, 1);
    lcd.clear();
    while (1) {
      led_blink(100, 100);
      lcd.setCursor(3, 0), lcd.print("Go Inside");
      if (ls(belakang)) run_motor(0, 1), beep_once(50), delay(500);
      if (!ls(atas)) {
        lcd.clear();
        break;
      }
    }
    while (1) {
      led_blink(100, 100);
      //      Serial.print(ls(belakang));
      //      Serial.print(" ");
      //      Serial.println(ls(atas));
      lcd.setCursor(4, 0), lcd.print(F("Sealing"));
      if (ls(atas) && ls(belakang)) {
        //        Serial.print(ls(belakang));
        //        Serial.print(" ");
        //        Serial.println(ls(atas));
        led_blink(100, 100);
        delay(out_delay);
        beep_once(100);
        lcd.clear();
        break;
      }
    }
    go_front();
  }
}

// move outside
void go_front() {
  while (1) {
    led_blink(100, 250);
    lcd.setCursor(3, 0), lcd.print(F("Go Outside"));
    if (ls(atas) && ls(belakang)) {
      run_motor(speed_out, 1);
    }
    if (ls(depan)) {
      lcd.clear();
      lcd.setCursor(0, 0), lcd.print(F("  Remove Gelas"));
      run_motor(0, 1), beep_once(250);
      if (!ir_state() || (menu >= 0 && menu <= 1 && cursor_menu == 1)) {
        break;
      }
    }
  }
}

// mengembalikan hopper ke posisi depan
void homing_front() {
  lcd.setCursor(0, 0), lcd.print(F("  Homing Front"));
  while (1) {
    refresh_screen(5);
    if (!ls(depan)) run_motor(speed_out, 1);
    if (ls(depan)) {
      lcd.setCursor(0, 0), lcd.print(F("  Remove Gelas"));
      run_motor(0, 1);
      delay(250), beep_once(100);
      if (!ir_state()) {
        lcd.setCursor(0, 0), lcd.print(F("  Done Homing"));
        delay(250), beep_once(250);
        break;
      }
    }
  }
}

// menu untuk setting parameter mesin
void setting_menu() {
  menu = 0, cursor_menu = 0;
  beep_once(50);
  lcd.clear();
  lcd.setCursor(0, 0), lcd.print("  Setting Menu");
  delay(1000);
  while (1) {
    led_blink(100, 1000);
    short_long_btn(2); //button 2 = button enter
    refresh_screen(5);

    if (!button(2) && cursor_menu == 0) menu += 1, beep_once(25), delay(100);

    if (menu > 6) menu = 0;
    if (menu < 0) menu = 6;

    lcd.setCursor(14, 1), lcd.print(menu);

    if (menu >= 5 && menu <= 6) {
      if (!button(0) || !button(1)) cursor_menu += 1, beep_once(25), delay(100);
      if (cursor_menu > 1) cursor_menu = 0;
      if (cursor_menu < 0) cursor_menu = 1;
      if (cursor_menu == 0) lcd.setCursor(0, 0), lcd.write(0);
      if (cursor_menu == 1) lcd.setCursor(0, 1), lcd.write(0);
    }

    if (menu != 5 && menu != 6) lcd.setCursor(0, 1), lcd.write(0);

    // setting kecepatan motor ke belakang
    if (menu == 0) {
      test_menu = 1;
      lcd.setCursor(1, 0), lcd.print("Speed Go In");
      if (!button(0)) speed_in--, beep_once(25);
      if (!button(1)) speed_in++, beep_once(25);
      if (speed_in < 0) speed_in = 0;
      if (speed_in > 255) speed_in = 255;
      lcd.setCursor(1, 1), lcd.print(speed_in), lcd.print("pwm");
      if (!button(3)) go_sealing();
    }
    // setting kecepatan motor ke depan
    if (menu == 1) {
      test_menu = 1;
      lcd.setCursor(1, 0), lcd.print("Speed Go Out");
      if (!button(0)) speed_out--, beep_once(25);
      if (!button(1)) speed_out++, beep_once(25);
      if (speed_out < 0) speed_out = 0;
      if (speed_out > 255) speed_out = 255;
      lcd.setCursor(1, 1), lcd.print(speed_out), lcd.print("pwm");
      if (!button(3)) go_sealing();
    }
    // setting waktu tunggu sebelum cup menuju ke belakang
    if (menu == 2) {
      lcd.setCursor(1, 0), lcd.print("Delay IN");
      if (!button(0)) set_in_delay -= 1, beep_once(25);
      if (!button(1)) set_in_delay += 1, beep_once(25);
      if (set_in_delay < 0) set_in_delay = 0;
      if (set_in_delay > 100) set_in_delay = 100;
      in_delay = set_in_delay * 100;
      lcd.setCursor(1, 1), lcd.print(in_delay), lcd.print("ms");
    }
    // setting waktu tunggu sebelum cup menuju ke depan
    if (menu == 3) {
      lcd.setCursor(1, 0), lcd.print("Delay OUT");
      if (!button(0)) set_out_delay -= 1, beep_once(25);
      if (!button(1)) set_out_delay += 1, beep_once(25);
      if (set_out_delay < 0) set_out_delay = 0;
      if (set_out_delay > 70) set_out_delay = 70;
      out_delay = set_out_delay * 100;
      lcd.setCursor(1, 1), lcd.print(out_delay), lcd.print("ms");
    }
    // cek sensor
    if (menu == 4) {
      lcd.setCursor(1, 0), lcd.print(F("IR "));
      lcd.setCursor(4, 0), lcd.print(F("LD "));
      lcd.setCursor(7, 0), lcd.print(F("LB "));
      lcd.setCursor(10, 0), lcd.print(F("LA "));
      lcd.setCursor(1, 1), lcd.print(ir_state());
      lcd.setCursor(4, 1), lcd.print(ls(depan));
      lcd.setCursor(7, 1), lcd.print(ls(belakang));
      lcd.setCursor(10, 1), lcd.print(ls(atas));
    }
    // kembali ke standby
    if (menu == 5) {
      lcd.setCursor(1, 0), lcd.print("Selesai?");
      lcd.setCursor(1, 1), lcd.print("Mulai");
      if (cursor_menu == 1 && button(2) == 0) {
        while (1) {
          lcd.clear();
          lcd.setCursor(0, 0), lcd.print("     MULAI");
          beep_once(200), beep_once(50);
          cursor_menu = 0, menu = 0;
          break;
        }
        break;
      }
    }
    // save setting ke eeprom
    if (menu == 6) {
      lcd.setCursor(1, 0), lcd.print("Selesai?");
      lcd.setCursor(1, 1), lcd.print("Save & Reset");
      if (cursor_menu == 1 && button(2) == 0) {
        beep_once(250);
        delay(1000), lcd.clear();
        lcd.setCursor(0, 0), lcd.print("  Save & Reset");
        EEPROM.write(0, pw_speed);
        EEPROM.write(1, set_in_delay);
        EEPROM.write(2, set_out_delay);
        EEPROM.write(3, speed_in);
        EEPROM.write(4, speed_out);
        for (int i = 0; i < 16; i++) {
          lcd.setCursor(i, 1), lcd.print(F("."));
          delay(100);
        }
        resetFunc();
      }
    }
  }
}

