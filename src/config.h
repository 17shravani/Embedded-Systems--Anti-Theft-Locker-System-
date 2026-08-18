#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// SYSTEM THRESHOLDS & SETTINGS
// ==========================================
#define SERIAL_BAUD 115200          // Serial monitor communication baud rate
#define PIN_LENGTH 4                // Length of user access password (excluding null terminator)
#define DEFAULT_PIN "1234"          // Initial standard user password
#define MASTER_PIN "9999"           // Master administrative override PIN
#define DURESS_PIN "1122"           // Silent Panic Duress code (unlocks + silent police alert)
#define MAX_WRONG_ATTEMPTS 3        // Maximum unsuccessful inputs allowed before alarm
#define LOCKOUT_DURATION_MS 30000   // Lockout time in milliseconds (30 seconds)
#define AUTO_LOCK_DELAY_MS 5000     // Delay before locker auto-locks in milliseconds (5 seconds)

// ==========================================
// HARDWARE PIN DEFINITIONS (ESP32)
// ==========================================
#define PIN_TAMPER_SENSOR 34        // SW-420 Vibration / Chassis Shock Sensor

// I2C LiquidCrystal Pins
#define I2C_SDA 21
#define I2C_SCL 22
#define LCD_ADDR 0x27

// 4x4 Keypad Row & Column Pins
#define KEYPAD_R1 13
#define KEYPAD_R2 12
#define KEYPAD_R3 14
#define KEYPAD_R4 27
#define KEYPAD_C1 26
#define KEYPAD_C2 25
#define KEYPAD_C3 33
#define KEYPAD_C4 32

// Outputs & Actuators
#define PIN_SERVO 18
#define PIN_BUZZER 19
#define PIN_LED_GREEN 2
#define PIN_LED_RED 4

// ==========================================
// PIN DEFINITIONS FOR ARDUINO UNO FALLBACK
// (If compiled on Arduino UNO, use these pins)
// ==========================================
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega2560__)
  #undef KEYPAD_R1
  #undef KEYPAD_R2
  #undef KEYPAD_R3
  #undef KEYPAD_R4
  #undef KEYPAD_C1
  #undef KEYPAD_C2
  #undef KEYPAD_C3
  #undef KEYPAD_C4
  #undef PIN_SERVO
  #undef PIN_BUZZER
  #undef PIN_LED_GREEN
  #undef PIN_LED_RED

  #define KEYPAD_R1 9
  #define KEYPAD_R2 8
  #define KEYPAD_R3 7
  #define KEYPAD_R4 6
  #define KEYPAD_C1 5
  #define KEYPAD_C2 4
  #define KEYPAD_C3 3
  #define KEYPAD_C4 2
  
  #define PIN_SERVO 10
  #define PIN_BUZZER 11
  #define PIN_LED_GREEN 12
  #define PIN_LED_RED 13
#endif

// ==========================================
// AUDIO FREQUENCIES (HZ)
// ==========================================
#define TONE_FREQ_SUCCESS 1500      // Success beep frequency
#define TONE_FREQ_ERROR 500         // Error beep frequency
#define TONE_FREQ_ALARM_1 800       // Alarm tone 1 (siren)
#define TONE_FREQ_ALARM_2 1200      // Alarm tone 2 (siren)

#endif // CONFIG_H
