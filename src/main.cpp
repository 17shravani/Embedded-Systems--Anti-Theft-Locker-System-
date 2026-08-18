#include "config.h"
#include "auth.h"
#include "display.h"
#include "lock.h"
#include "alarm.h"
#include <Keypad.h>

#if defined(ESP32)
#include <WiFi.h>
#endif

// ==========================================
// STATE MACHINE DEFINITION
// ==========================================
enum SystemState {
  STATE_LOCKED,
  STATE_UNLOCKED,
  STATE_LOCKOUT,
  STATE_PIN_CHANGE
};

static SystemState current_state = STATE_LOCKED;
static char input_buffer[PIN_LENGTH + 1] = {0};
static int buffer_index = 0;
static uint32_t last_lockout_update = 0;
static bool welcome_displayed = false;

// ==========================================
// KEYPAD CONFIGURATION
// ==========================================
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {KEYPAD_R1, KEYPAD_R2, KEYPAD_R3, KEYPAD_R4};
byte colPins[COLS] = {KEYPAD_C1, KEYPAD_C2, KEYPAD_C3, KEYPAD_C4};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ==========================================
// WI-FI SIMULATION (ESP32 only)
// ==========================================
void setup_wifi() {
#if defined(ESP32)
  Serial.println("\n[Network] Booting ESP32 Wi-Fi interface...");
  Serial.println("[Network] Connecting to Wokwi-GUEST virtual access point...");
  
  WiFi.begin("Wokwi-GUEST", "");
  
  // Non-blocking connection timeout loop (approx 3 seconds)
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 15) {
    delay(200);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[Network] Connected successfully!");
    Serial.print("[Network] IP Address allocated: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[Network] Virtual AP unreachable. Operating in offline security mode.");
  }
#else
  Serial.println("[Network] Wi-Fi features disabled (requires ESP32 MCU).");
#endif
}

void trigger_remote_alert(const char* event_type, const char* details) {
  Serial.print("\n>>> [REMOTE ALERT] Event: ");
  Serial.print(event_type);
  Serial.print(" | Detail: ");
  Serial.println(details);

#if defined(ESP32)
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(">>> [REMOTE ALERT] Wi-Fi packet transmitted successfully to Security Endpoint.");
    // In production, insert HTTP Client post requests or MQTT publish routines here.
  }
#endif
}

// ==========================================
// MAIN SETUP & PROGRAM ENTRY
// ==========================================
void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial && millis() < 3000); // Wait for Serial Monitor on boards with native USB
  Serial.println("\n===========================================");
  Serial.println("   ANTI-THEFT SMART LOCKER FIRMWARE V1.0");
  Serial.println("===========================================");

  // Initialize modular layers
  auth_init();
  display_init();
  alarm_init();
  lock_init();
  
  setup_wifi();

  // Check if system starts locked out from previous boot state
  if (auth_is_locked_out()) {
    current_state = STATE_LOCKOUT;
    alarm_start_siren();
    trigger_remote_alert("LOCKOUT_RESTORATION", "System restarted during lockout penalty window");
  } else {
    display_welcome();
    welcome_displayed = true;
  }
}

// ==========================================
// LOOP EXECUTION
// ==========================================
void loop() {
  // Run passive ticking handlers for lock timers and siren oscillation
  lock_handle();
  alarm_handle();

  // If the lock was unlocked and auto-locked in the background, update state
  if (current_state == STATE_UNLOCKED && !lock_is_open()) {
    current_state = STATE_LOCKED;
    welcome_displayed = false;
    trigger_remote_alert("AUTO_LOCK", "Locker automatically secured after timeout");
  }

  // Handle Lockout state transition when timer runs down
  if (current_state == STATE_LOCKOUT) {
    if (!auth_is_locked_out()) {
      alarm_stop_siren();
      current_state = STATE_LOCKED;
      welcome_displayed = false;
      trigger_remote_alert("LOCKOUT_EXPIRY", "Lockout penalty complete. System active.");
    } else {
      // Refresh LCD timer count once per second
      if (millis() - last_lockout_update >= 1000) {
        last_lockout_update = millis();
        display_lockout(auth_get_remaining_lockout_time());
      }
      return; // Do not check keypad input while locked out
    }
  }

  // Display initial welcome or return from an event
  if (current_state == STATE_LOCKED && !welcome_displayed) {
    display_welcome();
    welcome_displayed = true;
    buffer_index = 0;
    memset(input_buffer, 0, sizeof(input_buffer));
  }

  // Scan Keypad
  char key = keypad.getKey();
  if (key != NO_KEY) {
    Serial.print("[Keypad] Pressed: ");
    Serial.println(key);

    switch (current_state) {
      case STATE_LOCKED: {
        if (key >= '0' && key <= '9') {
          if (buffer_index < PIN_LENGTH) {
            input_buffer[buffer_index++] = key;
            input_buffer[buffer_index] = '\0';
            display_input(buffer_index);
            // Quick short beep for tactile feedback
            tone(PIN_BUZZER, TONE_FREQ_SUCCESS, 50); 
          } else {
            tone(PIN_BUZZER, TONE_FREQ_ERROR, 200);
          }
        }
        else if (key == '*') {
          // Clear current input
          buffer_index = 0;
          memset(input_buffer, 0, sizeof(input_buffer));
          display_input(0);
          tone(PIN_BUZZER, TONE_FREQ_ERROR, 50);
        }
        else if (key == '#') {
          if (buffer_index == PIN_LENGTH) {
            Serial.print("[Auth] Authenticating PIN: ");
            Serial.println(input_buffer);

            AuthResult res = auth_verify_password(input_buffer);
            if (res == AUTH_RESULT_SUCCESS) {
              current_state = STATE_UNLOCKED;
              display_access_granted();
              alarm_beep_success();
              lock_unlock();
              trigger_remote_alert("ACCESS_GRANTED", "Standard user passcode authenticated");
            } 
            else if (res == AUTH_RESULT_MASTER) {
              current_state = STATE_UNLOCKED;
              display_access_granted();
              alarm_beep_success();
              lock_unlock();
              trigger_remote_alert("MASTER_OVERRIDE", "Administrative master passcode authenticated");
            }
            else if (res == AUTH_RESULT_DURESS) {
              // Unlocks quietly to avoid alarming attacker, but sends silent panic alert
              current_state = STATE_UNLOCKED;
              display_access_granted();
              alarm_beep_success();
              lock_unlock();
              trigger_remote_alert("SILENT_DURESS_ALARM", "CRITICAL: Silent panic passcode used under duress!");
            }
            else {
              display_access_denied();
              alarm_beep_error();
              
              char alert_msg[64];
              sprintf(alert_msg, "Unauthorized code entry. Attempt count: %d", auth_get_failed_attempts());
              trigger_remote_alert("ACCESS_DENIED", alert_msg);

              if (auth_is_locked_out()) {
                current_state = STATE_LOCKOUT;
                alarm_start_siren();
                last_lockout_update = millis();
                display_lockout(auth_get_remaining_lockout_time());
                trigger_remote_alert("SYSTEM_LOCKOUT", "Emergency lockout engaged due to 3 failed attempts");
              } else {
                delay(2000); // Hold failure screen for visual feedback
                welcome_displayed = false; // Trigger screen reset
              }
            }
          } else {
            // Buffer is not complete PIN length
            tone(PIN_BUZZER, TONE_FREQ_ERROR, 200);
            display_clear();
            display_input(0);
            display_welcome();
          }
        }
        break;
      }

      case STATE_UNLOCKED: {
        // Unlocked state allows pressing 'A' to change current security password
        if (key == 'A') {
          current_state = STATE_PIN_CHANGE;
          display_new_pin_prompt();
          buffer_index = 0;
          memset(input_buffer, 0, sizeof(input_buffer));
        }
        break;
      }

      case STATE_PIN_CHANGE: {
        if (key >= '0' && key <= '9') {
          if (buffer_index < PIN_LENGTH) {
            input_buffer[buffer_index++] = key;
            input_buffer[buffer_index] = '\0';
            display_input(buffer_index);
            tone(PIN_BUZZER, TONE_FREQ_SUCCESS, 50);
          }
        }
        else if (key == '*') {
          buffer_index = 0;
          memset(input_buffer, 0, sizeof(input_buffer));
          display_input(0);
          tone(PIN_BUZZER, TONE_FREQ_ERROR, 50);
        }
        else if (key == '#') {
          if (buffer_index == PIN_LENGTH) {
            auth_change_password(input_buffer);
            display_pin_changed_success();
            alarm_beep_success();
            trigger_remote_alert("PIN_CHANGED", "Access code changed successfully");
            delay(2000);
            
            // Force lock re-activation after pin change
            lock_lock();
            current_state = STATE_LOCKED;
            welcome_displayed = false;
          } else {
            tone(PIN_BUZZER, TONE_FREQ_ERROR, 200);
            buffer_index = 0;
            memset(input_buffer, 0, sizeof(input_buffer));
            display_new_pin_prompt();
          }
        }
        break;
      }
      
      default:
        break;
    }
  }
}
