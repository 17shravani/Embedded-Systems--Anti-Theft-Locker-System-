# Technical Interview Preparation Guide

This guide is designed to prepare you to defend this project in lab vivas, technical reviews, and engineering interviews. It details the underlying embedded systems concepts, trade-offs, and security architectural details.

---

## 👨‍💻 Question 1: "Explain your project." (Must-Know Question)

### **Answer:**
"I designed and implemented an **Anti-Theft Locker System**, which is an electronic access control solution designed to secure valuables using authenticated entry, tamper monitoring, and remote alerting.

The system uses an **ESP32 microcontroller** as the core processor (with backward compatibility for Arduino UNO). It integrates a **4x4 matrix keypad** for PIN entry, a **16x2 I2C LCD** for visual status updates, a **Servo Motor** representing the locking bolt, a **Piezo Buzzer** for acoustic feedback, and status **LEDs**. 

Key professional features of my implementation include:
1. **State Machine Design**: Controls distinct states (Locked, Unlocked, Lockout, and PIN Change) reliably without using blocking `delay()` functions, ensuring system responsiveness.
2. **Persistent Credential Management**: Passwords are saved in non-volatile **EEPROM**. When users change the PIN via the keypad (pressing 'A' while unlocked), it persists across power failures.
3. **Anti-Tamper Lockout Strategy**: If an unauthorized user inputs 3 wrong codes consecutively, a loud oscillating siren starts, the LEDs flash, and the keypad is locked for 30 seconds. To prevent bypass, the failed attempt counter is saved to EEPROM; power-cycling the device resumes the lockout penalty instead of resetting it.
4. **Internet of Things Integration**: On ESP32, the system boots a virtual Wi-Fi interface and sends simulated packet notifications during access grants, denials, or lockouts."

---

## ⚙️ Question 2: Why did you use a Matrix Keypad instead of connecting individual push buttons? How does matrix scanning work?

### **Answer:**
"A 4x4 matrix keypad uses **8 GPIO pins** to read **16 buttons**, saving valuable microcontroller pins compared to individual wiring (which would require 16 pins).

The scanning works by utilizing **I/O multiplexing**:
- The columns (C1-C4) are configured as digital outputs, and rows (R1-R4) are configured as digital inputs with internal pull-up resistors.
- The microcontroller continually pulls one column LOW at a time, keeping others HIGH.
- If a button in that column is pressed, it pulls the corresponding row line LOW.
- By matching which column is driven LOW with which row reads LOW, the microcontroller identifies the pressed button coordinate."

---

## ⚡ Question 3: Why should we avoid powering the Servo motor directly from the ESP32 or Arduino 5V pin? How is this resolved?

### **Answer:**
"Servo motors contain DC motors that draw high current spikes (often exceeding 1 Amp) when initiating movement or fighting mechanical resistance.
- Microcontrollers are sensitive digital devices; their onboard linear regulators can typically handle only 100mA to 500mA.
- Drawing motor current through the MCU pin causes voltage sags (brownouts), which trigger system resets, corrupt the LCD display, or damage the MCU's silicon.
- **Resolution**: Use a split-rail power scheme. Connect the MCU to USB or a low-noise supply, and connect the Servo motor's power pins directly to an external regulated 5V/6V power source, linking only the grounds together to maintain signal reference."

---

## 💾 Question 4: How does the system handle password updates persistently? What happens during a power failure?

### **Answer:**
"The system uses non-volatile memory (**EEPROM** or ESP32 Flash emulation). Unlike SRAM, EEPROM retains data even when power is disconnected.
- On startup, the MCU reads address `0` (Magic Byte). If it matches `0xA5`, it loads the stored password from addresses `1` to `4`. If it doesn't match (first-time boot), it writes `0xA5` and the default PIN `"1234"` to the EEPROM.
- When the user changes the password, the new character array is written to address `1-4` and saved. If the locker is unplugged mid-operation, it boots back up using the updated password, preventing lockout."

---

## 🔒 Question 5: How does your lockout mechanism protect against brute-force attacks? How did you secure it against power-cycle bypasses?

### **Answer:**
"The lockout mechanism blocks automated password guessing by counting consecutive failures. Once the counter reaches 3, the state transitions to `STATE_LOCKOUT`, which ignores keypad inputs for 30 seconds and sounds an alarm.
- To prevent a malicious user from resetting the wrong-attempt counter simply by unplugging the locker, the failed attempt count is written to the **EEPROM** immediately on each incorrect try.
- During bootup, the system reads the counter. If it is >= 3, the system immediately launches the lockout penalty, neutralizing power-cycling bypass attempts."

---

## 🔄 Question 6: What is a Finite State Machine (FSM), and why is it important in embedded software?

### **Answer:**
"A Finite State Machine is a computational model consisting of a finite number of states, transitions between those states, and actions.
- In this project, the FSM coordinates four states: `STATE_LOCKED`, `STATE_UNLOCKED`, `STATE_LOCKOUT`, and `STATE_PIN_CHANGE`.
- Using an FSM makes the code highly modular, predictable, and easy to debug. Instead of writing complex, tangled nested `if-else` blocks, the system behavior is isolated to the active state, and transition paths are explicitly defined."

---

## 🕒 Question 7: Explain the difference between blocking and non-blocking delays in microcontroller code. How does your project handle this?

### **Answer:**
"A blocking delay, like `delay(1000)`, freezes the CPU in a tight loop for the duration. During this time, the microcontroller cannot scan the keypad, update timers, or execute warning siren frequencies.
- Non-blocking execution uses the `millis()` function, which returns the elapsed milliseconds since the MCU booted.
- In my project, functions like `lock_handle()` and `alarm_handle()` check if `millis() - last_execution_time >= target_delay`. If not, they exit immediately, allowing the microcontroller to execute other tasks in the main loop, keeping the keypad highly responsive."

---

## 🐜 Question 8: What is switch contact bounce (debouncing), and how is it managed on the keypad?

### **Answer:**
"When a mechanical button is pressed, the metal contacts do not close cleanly; they bounce or vibrate, creating a series of rapid open/close transitions (noise) for 5 to 20 milliseconds. The MCU is fast enough to interpret this noise as multiple rapid button presses.
- In my project, debouncing is handled in software by the **`Keypad` library**. The library scans the pins at regular intervals and uses an internal timer. It only registers a keypress when the state remains stable for a set debouncing window (typically 30–50ms)."

---

## 🔌 Question 9: What is the purpose of the common ground link in an embedded system containing multiple power sources?

### **Answer:**
"Voltage is a relative measurement of electrical potential difference between two points. 
- The control signal from the MCU to the Servo motor is a digital PWM pulse. The servo needs to measure the voltage of this pulse.
- If the grounds of the MCU and the external power supply are not connected, there is no shared reference point, meaning the control signal's voltage is floating and undefined to the servo, resulting in erratic movements, jitter, or control failure."

---

## 📈 Question 10: How can this project be scaled up or optimized for commercial security standards?

### **Answer:**
"To upgrade this system to commercial grade, I would:
1. **Upgrade Authentication**: Integrate an SPI-based RFID scanner (MFRC522) or a fingerprint sensor (R305) to implement dual-factor authentication (PIN + Biometrics).
2. **Encryption**: Encrypt the password inside the EEPROM using a hashing algorithm (like SHA-256) with salting, so that even if the MCU is de-capped or the flash memory is dumped, raw PINs cannot be extracted.
3. **Physical Tamper Switch**: Place a micro-limit switch on the locker chassis. If the outer casing is pried open, it cuts power to the lock or immediately triggers the tamper alarm.
4. **Active Network Alerting**: Link the ESP32 Wi-Fi interface to a service like Twilio or AWS IoT to send instant push notifications, SMS alerts, or emails to the owner when an unauthorized access is detected."
