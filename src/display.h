#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"

// Initialize the LCD display module
void display_init();

// Show the default idle locked screen
void display_welcome();

// Render input masking characters '*' based on password characters entered
void display_input(int char_count);

// Display the success screen when credentials are correct
void display_access_granted();

// Display incorrect password warning screen
void display_access_denied();

// Display lockout warning screen showing remaining timer seconds
void display_lockout(uint32_t remaining_ms);

// Show screen prompting for a new PIN
void display_new_pin_prompt();

// Show successful PIN change confirmation
void display_pin_changed_success();

// General display clear utility
void display_clear();

#endif // DISPLAY_H
