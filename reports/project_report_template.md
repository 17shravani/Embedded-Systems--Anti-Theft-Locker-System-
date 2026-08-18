# Project Report: Smart Anti-Theft Locker System
**Course Name:** Embedded Systems Lab (Project 1)  
**Academic Session:** 2026-2027  
**Student Name:** [Your Name]  
**Roll/Enrollment Number:** [Your Roll Number]  
**Institution:** [Your University/College Name]

---

## 1. Executive Summary
Provide a high-level overview of your project (2-3 paragraphs). Describe the purpose of building the locker, the selected microcontrollers, sensors, and actuators, and summarize the key functional achievements (persistent password updates, active siren alarms, and network notifications).

---

## 2. Problem Statement and Objectives
*   **Problem Statement**: Detail why mechanical lockers are insecure, and describe the common bypass methods.
*   **Project Objectives**:
    1. Develop dual-board (ESP32/Arduino) firmware for electronic locks.
    2. Implement secure, non-volatile password storage that survives power loss.
    3. Mitigate brute-force attacks via temporary lockdowns.
    4. Provide virtual and real-world system verifications.

---

## 3. Hardware Architecture and Connections
Discuss the hardware design. Explain the roles of the:
- Controller (ESP32 / Arduino UNO)
- Input devices (4x4 Matrix Keypad)
- Output displays (LiquidCrystal I2C 16x2)
- Actuators (SG90 Servo Motor representing the deadbolt)
- Warnings indicators (Piezo buzzer, red/green LEDs)

### Block Diagram
*(Insert schematic capture or block diagram here)*

---

## 4. Software Design and Flow
Describe your code structure.
- Explain the **Finite State Machine (FSM)**.
- Discuss how **non-blocking delays** (`millis()`) are used to ensure system stability and keypress responsiveness.
- Explain the role of the **EEPROM** in saving the credentials and failed attempt counter.

### FSM State Transition Table
*(Discuss Locked, Unlocked, Lockout, and PIN Change states)*

---

## 5. Experimental Results and Verifications
Provide step-by-step documentation of your testing.
- Paste simulation links (e.g. Wokwi ESP32 Project).
- Embed captures showing success prompts, incorrect entries, and locked out sirens.
- Discuss the log outputs from the Serial Monitor.

---

## 6. Learning Outcomes and Engineering Insights
Detail what you learned through this project:
- Power distribution and ground connections (split rail logic).
- Debouncing push button arrays.
- Memory map allocation in EEPROM blocks.
- Virtual IoT development protocols.

---

## 7. References and Bibliography
List any books, datasheets, or tutorial links used during the construction of this project:
1. ESP32 DevKit V1 Pinout Datasheet.
2. Arduino EEPROM Library Documentation.
3. Wokwi Simulation Platform User Manual.
