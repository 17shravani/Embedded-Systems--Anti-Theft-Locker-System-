# Anti-Theft Locker System
<img width="1916" height="916" alt="Screenshot 2026-08-18 155021" src="https://github.com/user-attachments/assets/df7a03fb-deb7-4352-afc8-a171d1c4c0b8" />

[![ESP32](https://img.shields.io/badge/Platform-ESP32-red.svg)](https://www.espressif.com/)
[![Arduino](https://img.shields.io/badge/Platform-Arduino_UNO-blue.svg)](https://www.arduino.cc/)
[![Wokwi Simulator](https://img.shields.io/badge/Simulation-Wokwi-green.svg)](https://wokwi.com/)
[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

An industry-oriented, secure, automated electronic locker system designed as a course project and portfolio-grade proof of work. This system integrates physical authentication, non-volatile credential management, brute-force mitigation, and remote security alerts using the **ESP32** (or **Arduino UNO**) microcontroller.

---

## 📖 1. Project Explanation

### What is an Anti-Theft Locker System?
Traditional mechanical locks are vulnerable to picking, duplicate keys, and lack logging mechanisms. An **Anti-Theft Locker System** replaces physical keys with a microcontroller-managed locking mechanism. Access is governed by software authentication (keypad PIN code) backed by persistent storage, real-time feedback screens, and active alarms.

### Simple Explanation
Think of this as a smart hotel safe. You enter a 4-digit code. If correct, the locker clicks open and lets you access your valuables, then locks itself automatically after you're done. If someone tries to guess your code, the locker beeps a warning. If they fail three times in a row, a loud siren goes off, the screen freezes, and it locks them out.

### Technical Explanation
The firmware utilizes a **Finite State Machine (FSM)** architecture running on a microcontroller to monitor a **4x4 multiplexed matrix keypad** and manage a **servo motor actuator** representing the lock. Credential validation is handled by comparing entered arrays with salts stored in the **EEPROM (Non-Volatile Memory)**. 

To mitigate brute force, consecutive failures increment a persistent counter. At a threshold of 3, the FSM transitions to a lockout state where a piezo buzzer is oscillated at dual frequencies (siren) using hardware timers, and the keypad input is ignored for a penalty window. Additionally, on ESP32, the system boots a Wi-Fi stack to send TCP/IP alerts to an administrative endpoint during security events.

---

## 💼 2. Industry Relevance & Business Value

Electronic locker access control systems are critical across multiple commercial domains:
*   **Banking & Finance**: Secures safe deposit boxes with dual-factor PIN and biometric systems.
*   **Retail & Logistics**: Powers automated delivery lockers (e.g., Amazon Lockers) for secure parcel pickups.
*   **Healthcare**: Restricts access to sensitive pharmaceuticals, narcotics, and patient logs in hospitals.
*   **Hotel Safes**: Provides local, user-set storage solutions with master bypass codes.

### Key Business Value:
*   **Controlled Access**: Only authorized users can unlock specific physical nodes.
*   **Electronic Auditability**: Access attempts, failures, and updates are logged over serial/network streams.
*   **Low-Cost Deployment**: Uses common off-the-shelf microcontrollers and sensor matrices to provide high-level security.

---

## 🛠️ 3. Tech Stack & Components Used

This project supports two hardware options to accommodate component availability. It can be run on real hardware or inside the **Wokwi Simulator**.

### Selection: Option C (Advanced) / Recommended Student Choice
The project uses the **ESP32** microcontroller as its primary target because of its high clock speeds, integrated Wi-Fi stack, and ample memory, but includes an automatic AVR registers configuration to run on an **Arduino UNO** without code changes.

### Components:
*   **ESP32 DevKit V1** (or Arduino UNO) - Central processing unit.
*   **4x4 Matrix Keypad** - User entry interface.
*   **16x2 I2C LCD Display** - Status and prompt feedback screen.
*   **Servo Motor (SG90)** - Lock bolt actuator.
*   **Piezo Buzzer** - Audible alarms and click tones.
*   **Red & Green LEDs** - Status indicator lights.
*   **220Ω Resistors** - Current limiters for the LEDs.
*   **Breadboard & Jumper Wires** - Wiring connections.
*   **External 5V DC Power Supply** - Dedicated high-draw actuator power.

---

## 📐 4. Project Architecture

### Data Flow Diagram
```
User Enters PIN
      ↓
Keypad Matrix Scanning (4 Rows x 4 Cols Multiplexed)
      ↓
MCU Reading (Debounced Inputs)
      ↓
Password Verification Logic (EEPROM Check)
      ↓
   +-- Correct?
   |
   +---> YES: Unlock Locker (Servo 90°) -> Green LED ON -> Auto-Lock after 5s
   |
   +---> NO: Increment Wrong-Attempt Counter (Saved to EEPROM)
               ↓
            Attempts >= 3?
               ↓
            YES: Trigger Lockout State (30s Keypad Freeze, Sirens & Red LED Flashing)
```

### State-Machine Table
| Current State | Event | Next State | Outputs |
| :--- | :--- | :--- | :--- |
| `LOCKED` | Key Press | `LOCKED` | LCD prints `*`, Buzzer clicks |
| `LOCKED` | PIN Verified Correct | `UNLOCKED` | LCD "Access Granted", Green LED ON, Servo 90° |
| `LOCKED` | PIN Incorrect (Att < 3) | `LOCKED` | LCD "Access Denied", Red LED Flashes, Beep |
| `LOCKED` | PIN Incorrect (Att = 3) | `LOCKOUT` | LCD "SYSTEM LOCKED", Siren starts, Red/Grn Strobe |
| `UNLOCKED` | Timer Exceeds 5s | `LOCKED` | LCD "Locker Secure", Red LED ON, Servo 0° |
| `UNLOCKED` | Press key 'A' | `PIN_CHANGE` | LCD "New PIN: " |
| `PIN_CHANGE` | New PIN entered + `#` | `LOCKED` | Writes new PIN to EEPROM, LCD "PASSWORD SET", Servo 0° |
| `LOCKOUT` | Lockout Timer Exceeds 30s | `LOCKED` | Siren stops, Red LED ON, Reset Attempts |

---

## 🔌 5. Circuit Connections

Please refer to the [Wiring Guide](circuit_diagram/wiring_guide.md) for full schematic tables for both ESP32 and Arduino UNO.

### Schematic Diagram Summary (ESP32)
*   **I2C LCD**: SDA ➔ GPIO 21, SCL ➔ GPIO 22, VCC ➔ 5V, GND ➔ GND.
*   **Keypad Rows (1-4)**: GPIO 13, 12, 14, 27.
*   **Keypad Columns (1-4)**: GPIO 26, 25, 33, 32.
*   **Servo Motor**: Signal ➔ GPIO 18, VCC ➔ External 5V, GND ➔ Shared GND.
*   **Buzzer**: Positive (+) ➔ GPIO 19, Negative (-) ➔ GND.
*   **Green LED**: Anode (+) ➔ GPIO 2 (via 220Ω), Cathode (-) ➔ GND.
*   **Red LED**: Anode (+) ➔ GPIO 4 (via 220Ω), Cathode (-) ➔ GND.

---

## 📂 6. Repository Folder Structure

```
Anti-Theft-Locker-Embedded-System/
├── src/                          # Modular production C++ source files
│   ├── main.cpp                  # Coordination loop and FSM
│   ├── config.h                  # Hardware pin mappings and thresholds
│   ├── auth.cpp / auth.h         # Persistent memory and lockout logic
│   ├── display.cpp / display.h   # LiquidCrystal I2C print handlers
│   ├── lock.cpp / lock.h         # Servo pwm rotation control
│   └── alarm.cpp / alarm.h       # Buzzer siren tones and strobe LEDs
├── arduino_code/                 # Monolithic Arduino IDE / Wokwi file
│   └── arduino_code.ino          # Consolidates all modules for easy uploads
├── simulation/                   # Virtual simulation configs
│   ├── diagram.json              # Wokwi wiring layout file
│   └── README.md                 # Simulation launch instructions
├── circuit_diagram/              # Connection diagrams
│   └── wiring_guide.md           # Master wiring reference tables
├── docs/                         # Technical documentation
│   ├── testing_strategy.md       # Quality Assurance checklist and test matrix
│   └── interview_prep.md         # Top 10 predicted viva/interview Q&As
└── README.md                     # Home page instruction guide
```

---

## 🚀 7. How to Install and Run

### Virtual Simulation (Wokwi)
No hardware needed!
1. Go to [Wokwi ESP32 Web Simulator](https://wokwi.com/projects/new/esp32).
2. Click the `diagram.json` tab, delete the default contents, and paste the code from [simulation/diagram.json](simulation/diagram.json).
3. Click the `sketch.ino` (or `main.cpp`) tab and replace its code with the contents of [arduino_code/arduino_code.ino](arduino_code/arduino_code.ino).
4. Run the simulation and enter PIN `1234#` to test!
*(Read more details in the [Simulation Guide](simulation/README.md).)*

### Real Hardware Deployment (Arduino IDE)
1. Install the latest [Arduino IDE](https://www.arduino.cc/en/software).
2. If using ESP32, install the Espressif Board Core via Board Manager (`https://dl.espressif.com/dl/package_esp32_index.json`).
3. Connect your board via USB.
4. Install these libraries from **Tools ➔ Manage Libraries**:
    *   `Keypad` by Mark Stanley, Alexander Brevig
    *   `LiquidCrystal I2C` by Frank de Brabander
    *   `ESP32Servo` by Kevin Harrington (only if using ESP32)
5. Open `arduino_code/arduino_code.ino` in the Arduino IDE.
6. Select your board (e.g. `ESP32 Dev Module` or `Arduino Uno`) and COM Port.
7. Click **Upload**.
8. Open the Serial Monitor at **115200 Baud** to view real-time logs.

---

## 📊 8. Testing Results Checklist

Before pushing modifications or presenting your project, ensure all test cases in the [Testing Strategy Matrix](docs/testing_strategy.md) pass. Key highlights:
- [x] Correct PIN entry (`1234#`) successfully retracts the servo, lights the green LED, chimes, and auto-locks after 5 seconds.
- [x] Incorrect PIN entry flashes the red LED and chimes an error beep.
- [x] 3 consecutive incorrect PIN entries activate the emergency siren and lock the system out.
- [x] Unplugging during lockout does not bypass the penalty (lockout persists on boot).
- [x] Changing PIN by pressing `A` when unlocked writes the new credentials to the EEPROM.

---

## 🔮 9. Future Improvements

If you want to expand this project to a higher level, consider:
1.  **Dual-Factor Authentication**: Combine the keypad entry with an MFRC522 RFID reader or an R305 fingerprint sensor.
2.  **IoT Integration**: Connect the ESP32 to a cloud interface (e.g., Blynk or Adafruit IO) to send push notifications directly to your phone when an unauthorized entry triggers the siren.
3.  **Encrypted EEPROM Storage**: Encrypt stored passwords using SHA-256 before writing to EEPROM to protect against physical data dumping.

---

## 🎓 10. Learning Outcomes

By building this project, you will demonstrate experience in:
1.  **Finite State Machine (FSM)** programming in Embedded C.
2.  **Hardware-Software Interfacing** (Multiplexed Keypads, I2C, and PWM signals).
3.  **Non-Volatile Memory management** (EEPROM) for security-sensitive data.
4.  **Split-Rail Power Design** to avoid brownouts in motor-driven circuits.
5.  **Multi-threaded Simulation** concepts using non-blocking delays (`millis()`).

---

## 👤 Author
*   **Your Name** - [GitHub Profile](https://github.com/your-username)
*   Course: Embedded Systems Lab Project
*   Institution: Your College/University Name
