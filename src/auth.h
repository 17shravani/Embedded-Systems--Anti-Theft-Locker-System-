#ifndef AUTH_H
#define AUTH_H

#include "config.h"

enum AuthResult {
  AUTH_RESULT_SUCCESS,
  AUTH_RESULT_MASTER,
  AUTH_RESULT_DURESS,
  AUTH_RESULT_FAILED,
  AUTH_RESULT_LOCKED_OUT
};

// Initialize authentication states and load password from EEPROM
void auth_init();

// Verify if the input password matches the stored password, master PIN, or duress code
AuthResult auth_verify_password(const char* entered_pin);

// Update the stored password in EEPROM/flash
void auth_change_password(const char* new_pin);

// Increment wrong attempts counter, persisting to EEPROM to prevent bypass on reset
void auth_increment_failed_attempts();

// Get the current number of failed attempts
int auth_get_failed_attempts();

// Reset failed attempt counter to 0
void auth_reset_failed_attempts();

// Check if the system is currently locked out
bool auth_is_locked_out();

// Get the remaining lockout duration in milliseconds
uint32_t auth_get_remaining_lockout_time();

// Set the lockout active status
void auth_trigger_lockout();

#endif // AUTH_H
