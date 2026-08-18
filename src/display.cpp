#include "display.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Instantiate the LCD object
static LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

void display_init() {
  // Initialize communication and turn on LCD backlight
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Locker Booting...");
  delay(1000);
}

void display_welcome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  LOCKER SECURE ");
  lcd.setCursor(0, 1);
  lcd.print("Enter PIN: ");
}

void display_input(int char_count) {
  // Clear the second line after "Enter PIN: " (starts at col 11)
  lcd.setCursor(11, 1);
  for (int i = 0; i < PIN_LENGTH; i++) {
    if (i < char_count) {
      lcd.print('*');
    } else {
      lcd.print(' ');
    }
  }
}

void display_access_granted() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" ACCESS GRANTED ");
  lcd.setCursor(0, 1);
  lcd.print(" Locker Opened! ");
}

void display_access_denied() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" ACCESS DENIED! ");
  lcd.setCursor(0, 1);
  lcd.print(" Try Again...   ");
}

void display_lockout(uint32_t remaining_ms) {
  uint32_t seconds = (remaining_ms + 999) / 1000; // Round up
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" SYSTEM LOCKED! ");
  lcd.setCursor(0, 1);
  lcd.print("Wait: ");
  lcd.print(seconds);
  lcd.print(" seconds ");
}

void display_new_pin_prompt() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" CHANGE PASSWORD");
  lcd.setCursor(0, 1);
  lcd.print("New PIN: ");
}

void display_pin_changed_success() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" PASSWORD SET!  ");
  lcd.setCursor(0, 1);
  lcd.print(" Locker Locking ");
}

void display_clear() {
  lcd.clear();
}
