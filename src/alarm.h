#ifndef ALARM_H
#define ALARM_H

#include "config.h"

// Initialize the buzzer and status LEDs pins
void alarm_init();

// Output a success beep and flash the green LED
void alarm_beep_success();

// Output an error beep and flash the red LED
void alarm_beep_error();

// Activate continuous emergency siren and strobe red LED
void alarm_start_siren();

// Deactivate continuous emergency siren
void alarm_stop_siren();

// Periodically run in the loop to handle non-blocking siren oscillation and flashing
void alarm_handle();

#endif // ALARM_H
