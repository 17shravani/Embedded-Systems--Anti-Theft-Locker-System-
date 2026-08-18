#include "auth.h"
#include <EEPROM.h>

#define EEPROM_SIZE 32
#define ADDR_MAGIC_BYTE 0
#define ADDR_PASSWORD_START 1
#define ADDR_FAILED_ATTEMPTS 6
#define MAGIC_BYTE_VALUE 0xA5

static char stored_password[PIN_LENGTH + 1] = {0};
static int failed_attempts = 0;
static bool locked_out = false;
static uint32_t lockout_start_time = 0;

void auth_init() {
  // Initialize EEPROM emulation for ESP32
#if defined(ESP32)
  EEPROM.begin(EEPROM_SIZE);
#endif

  // Read Magic Byte
  uint8_t magic = EEPROM.read(ADDR_MAGIC_BYTE);

  if (magic != MAGIC_BYTE_VALUE) {
    Serial.println("[Auth] EEPROM uninitialized. Writing defaults...");
    // Write magic byte
    EEPROM.write(ADDR_MAGIC_BYTE, MAGIC_BYTE_VALUE);
    
    // Write default password
    for (int i = 0; i < PIN_LENGTH; i++) {
      EEPROM.write(ADDR_PASSWORD_START + i, DEFAULT_PIN[i]);
    }
    EEPROM.write(ADDR_PASSWORD_START + PIN_LENGTH, '\0');

    // Write initial failed attempts
    EEPROM.write(ADDR_FAILED_ATTEMPTS, 0);

#if defined(ESP32)
    EEPROM.commit();
#endif
  }

  // Load password from EEPROM
  for (int i = 0; i < PIN_LENGTH; i++) {
    stored_password[i] = (char)EEPROM.read(ADDR_PASSWORD_START + i);
  }
  stored_password[PIN_LENGTH] = '\0';

  // Load persistent failed attempts to prevent bypass by power cycling
  failed_attempts = EEPROM.read(ADDR_FAILED_ATTEMPTS);
  if (failed_attempts >= MAX_WRONG_ATTEMPTS) {
    // If it was already locked out before power loss, re-enable lockout on boot
    locked_out = true;
    lockout_start_time = millis(); // Start lockout timer from boot
    Serial.println("[Auth] System restored in LOCKED OUT state!");
  }

  Serial.print("[Auth] Loaded PIN: ");
  Serial.println(stored_password);
  Serial.print("[Auth] Current Persistent Failed Attempts: ");
  Serial.println(failed_attempts);
}

AuthResult auth_verify_password(const char* entered_pin) {
  if (locked_out) {
    return AUTH_RESULT_LOCKED_OUT;
  }
  
  if (strcmp(entered_pin, DURESS_PIN) == 0) {
    auth_reset_failed_attempts();
    Serial.println("[Auth] >>> CRITICAL: SILENT DURESS / PANIC CODE DETECTED! <<<");
    return AUTH_RESULT_DURESS;
  }

  if (strcmp(entered_pin, MASTER_PIN) == 0) {
    auth_reset_failed_attempts();
    Serial.println("[Auth] Master Admin PIN Authenticated.");
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
  Serial.print("[Auth] PIN changed successfully to: ");
  Serial.println(stored_password);
}

void auth_increment_failed_attempts() {
  failed_attempts++;
  EEPROM.write(ADDR_FAILED_ATTEMPTS, failed_attempts);
#if defined(ESP32)
  EEPROM.commit();
#endif
  Serial.print("[Auth] Failed attempt recorded. Total: ");
  Serial.println(failed_attempts);

  if (failed_attempts >= MAX_WRONG_ATTEMPTS) {
    auth_trigger_lockout();
  }
}

int auth_get_failed_attempts() {
  return failed_attempts;
}

void auth_reset_failed_attempts() {
  failed_attempts = 0;
  EEPROM.write(ADDR_FAILED_ATTEMPTS, 0);
#if defined(ESP32)
  EEPROM.commit();
#endif
  locked_out = false;
  Serial.println("[Auth] Failed attempts reset.");
}

bool auth_is_locked_out() {
  if (locked_out) {
    // Check if the lockout period has completed
    if (millis() - lockout_start_time >= LOCKOUT_DURATION_MS) {
      locked_out = false;
      auth_reset_failed_attempts(); // Reset failed attempts upon successful completion of lockout
      Serial.println("[Auth] Lockout period expired. System ready.");
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
  Serial.println("[Auth] MAXIMUM ATTEMPTS EXCEEDED. SYSTEM LOCKED.");
}
