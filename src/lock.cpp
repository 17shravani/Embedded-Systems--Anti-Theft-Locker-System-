#include "lock.h"

#if defined(ESP32)
  #include <ESP32Servo.h>
#else
  #include <Servo.h>
#endif

static Servo lockerServo;
static bool open_state = false;
static uint32_t unlock_time = 0;

void lock_init() {
#if defined(ESP32)
  // ESP32 requires setting period/timer configurations occasionally, 
  // but ESP32Servo handles this automatically during attach.
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  lockerServo.setPeriodHertz(50); // Standard 50Hz servo
#endif

  lockerServo.attach(PIN_SERVO);
  lock_lock(); // Force locked position on startup
}

void lock_unlock() {
  lockerServo.write(90); // Turn to unlocked position
  open_state = true;
  unlock_time = millis();
  Serial.println("[Lock] Locker Unlocked (Servo: 90 degrees).");
}

void lock_lock() {
  lockerServo.write(0); // Turn to locked position
  open_state = false;
  Serial.println("[Lock] Locker Secured (Servo: 0 degrees).");
}

bool lock_is_open() {
  return open_state;
}

void lock_handle() {
  // If locker is currently open, verify if auto-lock timer has expired
  if (open_state) {
    if (millis() - unlock_time >= AUTO_LOCK_DELAY_MS) {
      lock_lock();
    }
  }
}
