#include "alarm.h"

static bool siren_active = false;
static uint32_t last_siren_toggle = 0;
static bool siren_state = false;

void alarm_init() {
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  
  // Turn off outputs and show default state
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, HIGH); // Locked state = Red LED ON
  noTone(PIN_BUZZER);
}

void alarm_beep_success() {
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, HIGH);
  tone(PIN_BUZZER, TONE_FREQ_SUCCESS);
  delay(200);
  noTone(PIN_BUZZER);
  // Keep green LED on (main loop controls when to turn it off on lock re-engagement)
}

void alarm_beep_error() {
  // Flash red LED and beep a low warning pitch
  tone(PIN_BUZZER, TONE_FREQ_ERROR);
  for (int i = 0; i < 3; i++) {
    digitalWrite(PIN_LED_RED, LOW);
    delay(100);
    digitalWrite(PIN_LED_RED, HIGH);
    delay(100);
  }
  noTone(PIN_BUZZER);
}

void alarm_start_siren() {
  siren_active = true;
  last_siren_toggle = millis();
  siren_state = false;
  Serial.println("[Alarm] EMERGENCY ALARM SIREN INITIATED!");
}

void alarm_stop_siren() {
  siren_active = false;
  noTone(PIN_BUZZER);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, HIGH); // Return to default locked indicator
  Serial.println("[Alarm] Siren silenced. System returning to standby.");
}

void alarm_handle() {
  if (siren_active) {
    // Alternate buzzer frequency and flash red/green LEDs for siren effect
    if (millis() - last_siren_toggle >= 200) {
      last_siren_toggle = millis();
      siren_state = !siren_state;
      if (siren_state) {
        tone(PIN_BUZZER, TONE_FREQ_ALARM_1);
        digitalWrite(PIN_LED_RED, HIGH);
        digitalWrite(PIN_LED_GREEN, LOW);
      } else {
        tone(PIN_BUZZER, TONE_FREQ_ALARM_2);
        digitalWrite(PIN_LED_RED, LOW);
        digitalWrite(PIN_LED_GREEN, HIGH);
      }
    }
  }
}
