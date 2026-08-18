# Circuit Wiring Guide - Anti-Theft Locker System

This document outlines the detailed wiring specifications for both the **ESP32 Microcontroller** and the **Arduino UNO** setups. It includes pin mappings, power routing, pull-up/pull-down resistor details, and notes on using a virtual simulator vs. real hardware.

---

## 1. Power Distribution and Grounding (Crucial for Real Hardware)

> [!WARNING]
> Do NOT power the Servo Motor or Solenoid Lock directly from the ESP32/Arduino 5V pin. Actuators draw high current when moving, which can cause voltage spikes and dropouts, leading to MCU resets, LCD corruption, or damage to the microcontroller.

- **Microcontroller**: Powered via USB (5V) or an external 5V regulated source on the `5V`/`VIN` pin.
- **Servo Motor / Solenoid Lock**: Powered by an external 5V to 6V DC power supply (capable of supplying at least 1.5A to 2A).
- **Common Ground**: You **MUST** connect the ground (`GND`) of the external power supply to the ground (`GND`) of the ESP32/Arduino. Without this, the PWM control signal will have no reference and the servo will jitter or fail to work.

---

## 2. ESP32 Pin Connection Table

Below is the pin connection guide when using an ESP32 as the primary controller.

### Power Connections
| Component | Pin Label | Connected To | Notes |
| :--- | :--- | :--- | :--- |
| ESP32 | GND | External Power Supply GND & Breadboard GND Rail | Common ground reference |
| ESP32 | VIN / 5V | USB Power or Regulated 5V Supply Positive | MCU power |
| LCD 16x2 I2C | VCC | Breadboard 5V Rail | Power for display and backlight |
| LCD 16x2 I2C | GND | Breadboard GND Rail | Ground |
| Servo Motor | VCC (Red) | External Supply Positive (5V-6V) | Separate power for motor |
| Servo Motor | GND (Brown/Blk) | Breadboard GND Rail | Common ground |

### Control Signal Connections
| Component | Component Pin | ESP32 Pin | Wiring Notes |
| :--- | :--- | :--- | :--- |
| **I2C LCD 16x2** | SDA | GPIO 21 | Hardware I2C Data line |
| | SCL | GPIO 22 | Hardware I2C Clock line |
| **4x4 Keypad** | Row 1 (R1) | GPIO 13 | Digital I/O (Input with Pullup) |
| | Row 2 (R2) | GPIO 12 | Digital I/O (Input with Pullup) |
| | Row 3 (R3) | GPIO 14 | Digital I/O (Input with Pullup) |
| | Row 4 (R4) | GPIO 27 | Digital I/O (Input with Pullup) |
| | Col 1 (C1) | GPIO 26 | Digital I/O (Output) |
| | Col 2 (C2) | GPIO 25 | Digital I/O (Output) |
| | Col 3 (C3) | GPIO 33 | Digital I/O (Output) |
| | Col 4 (C4) | GPIO 32 | Digital I/O (Output) |
| **Servo Motor** | Signal (Orange/Yel) | GPIO 18 | PWM Control output |
| **Buzzer** | Positive (+) | GPIO 19 | Piezo buzzer output (PWM or Digital) |
| | Negative (-) | GND | Ground |
| **Green LED** | Anode (+) | GPIO 2 | Connect through 220Ω resistor to pin |
| | Cathode (-) | GND | Connect to Ground |
| **Red LED** | Anode (+) | GPIO 4 | Connect through 220Ω resistor to pin |
| | Cathode (-) | GND | Connect to Ground |

---

## 3. Arduino UNO Pin Connection Table (Alternative Option)

For students using the Arduino UNO, connect according to this mapping:

| Component | Pin Label | Arduino UNO Pin | Wiring Notes |
| :--- | :--- | :--- | :--- |
| **I2C LCD 16x2** | SDA | A4 | Direct hardware I2C Data |
| | SCL | A5 | Direct hardware I2C Clock |
| **4x4 Keypad** | Row 1 (R1) | Pin 9 | Row inputs |
| | Row 2 (R2) | Pin 8 | Row inputs |
| | Row 3 (R3) | Pin 7 | Row inputs |
| | Row 4 (R4) | Pin 6 | Row inputs |
| | Col 1 (C1) | Pin 5 | Column outputs |
| | Col 2 (C2) | Pin 4 | Column outputs |
| | Col 3 (C3) | Pin 3 | Column outputs |
| | Col 4 (C4) | Pin 2 | Column outputs |
| **Servo Motor** | Signal | Pin 10 | PWM output |
| **Buzzer** | Positive (+) | Pin 11 | Digital/Tone output |
| **Green LED** | Anode (+) | Pin 12 | Through 220Ω resistor |
| **Red LED** | Anode (+) | Pin 13 | Through 220Ω resistor |

---

## 4. Key Electronic Notes

1. **LED Current Limiting**: Always place a **220Ω to 330Ω resistor** in series with both the Green and Red LEDs to prevent drawing excessive current from the microcontrollers' GPIO pins.
2. **I2C Pull-Up Resistors**: Most I2C LCD modules have built-in 4.7kΩ pull-up resistors on the SDA and SCL lines. If using raw LCDs or custom boards, ensure SDA and SCL are pulled up to VCC (3.3V for ESP32, 5V for Arduino) with 4.7kΩ resistors.
3. **Piezo Buzzer Drive**: A standard active buzzer can be driven directly by a GPIO pin. However, if using a high-draw magnetic buzzer, use a small NPN transistor (like BC547) to switch power to it.
