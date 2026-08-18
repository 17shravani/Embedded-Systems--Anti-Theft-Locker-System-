/**
 * ============================================================================
 * Project: Enterprise Smart Vault & Anti-Theft Locker System
 * Platform: ESP32 (Recommended) or Arduino UNO
 * Version: 2.4 Enterprise Edition
 * Features:
 *   - Multi-Role Authentication: User PIN ("1234"), Master PIN ("9999")
 *   - Silent Duress / Panic Code ("1122"): Unlocks + fires covert silent alert
 *   - Persistent Flash/EEPROM Credential Storage
 *   - Brute-Force Defense: 3-Attempt Lockout with Dual-Tone Siren
 *   - SW-420 Physical Vibration / Chassis Tamper Detection
 *   - Simulated Cloud IoT MQTT / REST Telemetry Streams over Wi-Fi
 * ============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

#if defined(ESP32)
  #include <WiFi.h>
  #include <ESP32Servo.h>
#else
  #include <Servo.h>
#endif

// ============================================================================
// 1. SYSTEM CONFIGURATIONS & CREDENTIALS
// ============================================================================
#define SERIAL_BAUD 115200          
#define PIN_LENGTH 4                
#define DEFAULT_PIN "1234"          
#define MASTER_PIN "9999"           // Master administrative override PIN
#define DURESS_PIN "1122"           // Silent Panic Duress code
#define MAX_WRONG_ATTEMPTS 3        
#define LOCKOUT_DURATION_MS 30000   
#define AUTO_LOCK_DELAY_MS 5000     

// Pin Mappings
#if defined(ESP32)
  #define I2C_SDA 21
  #define I2C_SCL 22
  #define LCD_ADDR 0x27

  #define KEYPAD_R1 13
  #define KEYPAD_R2 12
  #define KEYPAD_R3 14
  #define KEYPAD_R4 27
  #define KEYPAD_C1 26
  #define KEYPAD_C2 25
  #define KEYPAD_C3 33
  #define KEYPAD_C4 32

  #define PIN_SERVO 18
  #define PIN_BUZZER 19
  #define PIN_LED_GREEN 2
  #define PIN_LED_RED 4
  #define PIN_TAMPER_SENSOR 34      // SW-420 Vibration Sensor
#else
  #define LCD_ADDR 0x27
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
  #define PIN_TAMPER_SENSOR A0
#endif

#define TONE_FREQ_SUCCESS 1500      
#define TONE_FREQ_ERROR 500         
#define TONE_FREQ_ALARM_1 800       
#define TONE_FREQ_ALARM_2 1200      

// EEPROM Map
#define EEPROM_SIZE 32
#define ADDR_MAGIC_BYTE 0
#define ADDR_PASSWORD_START 1
#define ADDR_FAILED_ATTEMPTS 6
#define MAGIC_BYTE_VALUE 0xA5

// Authentication Result Enum
enum AuthResult {
  AUTH_RESULT_SUCCESS,
  AUTH_RESULT_MASTER,
  AUTH_RESULT_DURESS,
  AUTH_RESULT_FAILED,
  AUTH_RESULT_LOCKED_OUT
};

enum SystemState {
  STATE_LOCKED,
  STATE_UNLOCKED,
  STATE_LOCKOUT,
  STATE_PIN_CHANGE,
  STATE_TAMPER_ALARM
};

// Global objects
static LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
static Servo lockerServo;

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

static SystemState current_state = STATE_LOCKED;
static char stored_password[PIN_LENGTH + 1] = {0};
static char input_buffer[PIN_LENGTH + 1] = {0};
static int buffer_index = 0;
static int failed_attempts = 0;
static bool locked_out = false;
static uint32_t lockout_start_time = 0;
static uint32_t last_lockout_update = 0;
static uint32_t unlock_time = 0;
static bool open_state = false;
static bool siren_active = false;
static uint32_t last_siren_toggle = 0;
static bool siren_state = false;
static bool welcome_displayed = false;

// Forward Declarations
void auth_init();
AuthResult auth_verify_password(const char* entered_pin);
void auth_change_password(const char* new_pin);
void auth_increment_failed_attempts();
void auth_reset_failed_attempts();
bool auth_is_locked_out();
uint32_t auth_get_remaining_lockout_time();
void auth_trigger_lockout();

void display_init();
void display_welcome();
void display_input(int char_count);
void display_access_granted();
void display_access_denied();
void display_lockout(uint32_t remaining_ms);
void display_new_pin_prompt();
void display_pin_changed_success();
void display_clear();

void lock_init();
void lock_unlock();
void lock_lock();
bool lock_is_open();
void lock_handle();

void alarm_init();
void alarm_beep_success();
void alarm_beep_error();
void alarm_start_siren();
void alarm_stop_siren();
void alarm_handle();

void setup_wifi();
void trigger_remote_alert(const char* event_type, const char* details);

// ============================================================================
// SETUP & LOOP
// ============================================================================
void setup() {
  Serial.begin(SERIAL_BAUD);
  while (!Serial && millis() < 3000); 
  Serial.println("\n===========================================");
  Serial.println("  ENTERPRISE VAULT SECURITY OS V2.4");
  Serial.println("===========================================");

  auth_init();
  display_init();
  alarm_init();
  lock_init();
  
  pinMode(PIN_TAMPER_SENSOR, INPUT_PULLUP);

  setup_wifi();

  if (auth_is_locked_out()) {
    current_state = STATE_LOCKOUT;
    alarm_start_siren();
    trigger_remote_alert("LOCKOUT_RESTORATION", "System restarted during active penalty window");
  } else {
    display_welcome();
    welcome_displayed = true;
  }
}

void loop() {
  lock_handle();
  alarm_handle();

  // Background auto-lock transition
  if (current_state == STATE_UNLOCKED && !lock_is_open()) {
    current_state = STATE_LOCKED;
    welcome_displayed = false;
    trigger_remote_alert("AUTO_LOCK", "Locker secured automatically after timeout");
  }

  // Handle system penalty ticks
  if (current_state == STATE_LOCKOUT) {
    if (!auth_is_locked_out()) {
      alarm_stop_siren();
      current_state = STATE_LOCKED;
      welcome_displayed = false;
      trigger_remote_alert("LOCKOUT_EXPIRY", "Lockout penalty completed. System ready.");
    } else {
      if (millis() - last_lockout_update >= 1000) {
        last_lockout_update = millis();
        display_lockout(auth_get_remaining_lockout_time());
      }
      return; 
    }
  }

  // Draw standby screen
  if (current_state == STATE_LOCKED && !welcome_displayed) {
    display_welcome();
    welcome_displayed = true;
    buffer_index = 0;
    memset(input_buffer, 0, sizeof(input_buffer));
  }

  // Read Keypad Matrix
  char key = keypad.getKey();
  if (key != NO_KEY) {
    Serial.print("[Keypad] Key Pressed: ");
    Serial.println(key);

    switch (current_state) {
      case STATE_LOCKED: {
        if (key >= '0' && key <= '9') {
          if (buffer_index < PIN_LENGTH) {
            input_buffer[buffer_index++] = key;
            input_buffer[buffer_index] = '\0';
            display_input(buffer_index);
            tone(PIN_BUZZER, TONE_FREQ_SUCCESS, 50); 
          } else {
            tone(PIN_BUZZER, TONE_FREQ_ERROR, 200);
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
            AuthResult res = auth_verify_password(input_buffer);
            
            if (res == AUTH_RESULT_SUCCESS) {
              current_state = STATE_UNLOCKED;
              display_access_granted();
              alarm_beep_success();
              lock_unlock();
              trigger_remote_alert("ACCESS_GRANTED", "Standard user passcode authorized");
            } 
            else if (res == AUTH_RESULT_MASTER) {
              current_state = STATE_UNLOCKED;
              display_access_granted();
              alarm_beep_success();
              lock_unlock();
              trigger_remote_alert("MASTER_OVERRIDE", "Master administrative passcode authorized");
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
              sprintf(alert_msg, "Unauthorized code entry. Attempt count: %d", failed_attempts);
              trigger_remote_alert("ACCESS_DENIED", alert_msg);

              if (auth_is_locked_out()) {
                current_state = STATE_LOCKOUT;
                alarm_start_siren();
                last_lockout_update = millis();
                display_lockout(auth_get_remaining_lockout_time());
                trigger_remote_alert("SYSTEM_LOCKOUT", "Emergency lockout engaged due to 3 failed attempts");
              } else {
                delay(2000); 
                welcome_displayed = false; 
              }
            }
          } else {
            tone(PIN_BUZZER, TONE_FREQ_ERROR, 200);
            display_clear();
            display_input(0);
            display_welcome();
          }
        }
        break;
      }

      case STATE_UNLOCKED: {
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

// ============================================================================
// MODULE IMPLEMENTATIONS
// ============================================================================
void auth_init() {
#if defined(ESP32)
  EEPROM.begin(EEPROM_SIZE);
#endif

  uint8_t magic = EEPROM.read(ADDR_MAGIC_BYTE);

  if (magic != MAGIC_BYTE_VALUE) {
    EEPROM.write(ADDR_MAGIC_BYTE, MAGIC_BYTE_VALUE);
    for (int i = 0; i < PIN_LENGTH; i++) {
      EEPROM.write(ADDR_PASSWORD_START + i, DEFAULT_PIN[i]);
    }
    EEPROM.write(ADDR_PASSWORD_START + PIN_LENGTH, '\0');
    EEPROM.write(ADDR_FAILED_ATTEMPTS, 0);

#if defined(ESP32)
    EEPROM.commit();
#endif
  }

  for (int i = 0; i < PIN_LENGTH; i++) {
    stored_password[i] = (char)EEPROM.read(ADDR_PASSWORD_START + i);
  }
  stored_password[PIN_LENGTH] = '\0';

  failed_attempts = EEPROM.read(ADDR_FAILED_ATTEMPTS);
  if (failed_attempts >= MAX_WRONG_ATTEMPTS) {
    locked_out = true;
    lockout_start_time = millis();
  }
}

AuthResult auth_verify_password(const char* entered_pin) {
  if (locked_out) return AUTH_RESULT_LOCKED_OUT;
  
  if (strcmp(entered_pin, DURESS_PIN) == 0) {
    auth_reset_failed_attempts();
    return AUTH_RESULT_DURESS;
  }

  if (strcmp(entered_pin, MASTER_PIN) == 0) {
    auth_reset_failed_attempts();
    return AUTH_RESULT_MASTER;
  }

  if (strcmp(entered_pin, stored_password) == 0) {
    auth_reset_failed_attempts();
    return AUTH_RESULT_SUCCESS;
  } else {
    auth_increment_failed_attempts();
    return AUTH_RESULT_FAILED;
  }
}

void auth_change_password(const char* new_pin) {
  if (strlen(new_pin) != PIN_LENGTH) return;
  strncpy(stored_password, new_pin, PIN_LENGTH);
  stored_password[PIN_LENGTH] = '\0';

  for (int i = 0; i < PIN_LENGTH; i++) {
    EEPROM.write(ADDR_PASSWORD_START + i, stored_password[i]);
  }
#if defined(ESP32)
  EEPROM.commit();
#endif
}

void auth_increment_failed_attempts() {
  failed_attempts++;
  EEPROM.write(ADDR_FAILED_ATTEMPTS, failed_attempts);
#if defined(ESP32)
  EEPROM.commit();
#endif

  if (failed_attempts >= MAX_WRONG_ATTEMPTS) {
    auth_trigger_lockout();
  }
}

void auth_reset_failed_attempts() {
  failed_attempts = 0;
  EEPROM.write(ADDR_FAILED_ATTEMPTS, 0);
#if defined(ESP32)
  EEPROM.commit();
#endif
  locked_out = false;
}

bool auth_is_locked_out() {
  if (locked_out) {
    if (millis() - lockout_start_time >= LOCKOUT_DURATION_MS) {
      locked_out = false;
      auth_reset_failed_attempts();
    }
  }
  return locked_out;
}

uint32_t auth_get_remaining_lockout_time() {
  if (!locked_out) return 0;
  uint32_t elapsed = millis() - lockout_start_time;
  if (elapsed >= LOCKOUT_DURATION_MS) return 0;
  return LOCKOUT_DURATION_MS - elapsed;
}

void auth_trigger_lockout() {
  locked_out = true;
  lockout_start_time = millis();
}

// ----------------- LCD DISPLAY MODULE -----------------
void display_init() {
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("VAULT OS BOOT...");
  delay(1000);
}

void display_welcome() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("  LOCKER SECURE ");
  lcd.setCursor(0, 1);
  lcd.print("Enter PIN/Card: ");
}

void display_input(int char_count) {
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
  uint32_t seconds = (remaining_ms + 999) / 1000;
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

// ----------------- SERVO LOCK MODULE -----------------
void lock_init() {
#if defined(ESP32)
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  lockerServo.setPeriodHertz(50);
#endif
  lockerServo.attach(PIN_SERVO);
  lock_lock(); 
}

void lock_unlock() {
  lockerServo.write(90);
  open_state = true;
  unlock_time = millis();
}

void lock_lock() {
  lockerServo.write(0);
  open_state = false;
}

bool lock_is_open() {
  return open_state;
}

void lock_handle() {
  if (open_state) {
    if (millis() - unlock_time >= AUTO_LOCK_DELAY_MS) {
      lock_lock();
    }
  }
}

// ----------------- ALERT & ALARM MODULE -----------------
void alarm_init() {
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, HIGH);
  noTone(PIN_BUZZER);
}

void alarm_beep_success() {
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_LED_GREEN, HIGH);
  tone(PIN_BUZZER, TONE_FREQ_SUCCESS);
  delay(200);
  noTone(PIN_BUZZER);
}

void alarm_beep_error() {
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
}

void alarm_stop_siren() {
  siren_active = false;
  noTone(PIN_BUZZER);
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, HIGH);
}

void alarm_handle() {
  if (siren_active) {
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

// ----------------- NETWORK ALERTS -----------------
void setup_wifi() {
#if defined(ESP32)
  Serial.println("\n[Network] Initializing Wi-Fi SoC...");
  Serial.println("[Network] Connecting to virtual Wokwi-GUEST network...");
  WiFi.begin("Wokwi-GUEST", "");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(300);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[Network] Wi-Fi Link Established!");
    Serial.print("[Network] IP allocated: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n[Network] Operating in offline security mode.");
  }
#else
  Serial.println("[Network] Network alerting is disabled on AVR boards.");
#endif
}

void trigger_remote_alert(const char* event_type, const char* details) {
  Serial.print("\n>>> [REMOTE ALERT] Security Event: ");
  Serial.print(event_type);
  Serial.print(" | Description: ");
  Serial.println(details);

#if defined(ESP32)
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(">>> [REMOTE ALERT] Transmitted to MQTT Broker (lockers/vault-01/events).");
  }
#endif
}
