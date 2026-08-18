#ifndef LOCK_H
#define LOCK_H

#include "config.h"

// Initialize the locking actuator
void lock_init();

// Rotate servo/activate solenoid to unlock the locker
void lock_unlock();

// Rotate servo/deactivate solenoid to lock the locker
void lock_lock();

// Check if the locker is currently in an unlocked state
bool lock_is_open();

// Periodically run in the loop to check and manage the auto-lock timeout
void lock_handle();

#endif // LOCK_H
