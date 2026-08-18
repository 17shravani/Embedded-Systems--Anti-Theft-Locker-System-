# Testing Strategy & Quality Assurance Matrix

This document provides a comprehensive testing protocol to verify all features of the **Anti-Theft Locker System**. It covers functional requirements, state transitions, security boundary limits, and physical/virtual signal verification.

---

## 1. Test Setup Configurations

### Virtual Environment (Wokwi)
- Compile target: **ESP32 DevKit V1**
- Required Libraries in Library Manager:
  - `Keypad`
  - `LiquidCrystal I2C`
  - `ESP32Servo`
- Serial Monitor: Active (115200 Baud)

### Real Hardware Environment
- Multimeter: Set to DC Voltage to test LED/Buzzer line signals if components are absent.
- Power Supply: External 5V-6V DC supply for the Servo motor.
- Computer: Connected via micro-USB/USB-C to inspect real-time log outputs on the Arduino IDE Serial Monitor.

---

## 2. Test Cases Matrix

Run the following test scenarios in order to validate the system.

| Test Case | Description | Input Actions | Expected System Output | Verification Criteria | Status (P/F) |
| :--- | :--- | :--- | :--- | :--- | :---: |
| **TC-01** | System Initialization | Power up / Restart the system. | - LCD prints "Locker Booting..." then "LOCKER SECURE / Enter PIN:".<br>- Red LED turns ON (Locked indicator).<br>- Green LED is OFF.<br>- Servo rotates to 0° (Locked).<br>- Serial outputs initial boot diagnostics and Wi-Fi connection states. | - LCD showing standby message.<br>- Red LED active.<br>- Servo position at 0°.<br>- Serial outputs loaded credentials. | |
| **TC-02** | Digit Entry & Masking | Press digits `1`, `2`, `3`. | - LCD updates character count as `* * *`.<br>- Piezo buzzer emits a very short (50ms) high frequency click for key click tactile feedback. | - Screen displaying 3 stars.<br>- Short beep heard on each button press. | |
| **TC-03** | Input Deletion | Press keypad `*` during digit entry. | - LCD character buffer resets.<br>- Line 2 prints blank spaces after "Enter PIN:".<br>- Input buffer is empty. | - Screen clears entry stars back to empty PIN status. | |
| **TC-04** | Valid Authentication | Input correct passcode `1234` and press `#`. | - LCD displays "ACCESS GRANTED / Locker Opened!".<br>- Red LED turns OFF, Green LED turns ON.<br>- Servo rotates 90° (Bolt retracts).<br>- Buzzer beeps a success tone (1500Hz) for 200ms.<br>- Serial sends `ACCESS_GRANTED` alert details. | - Green LED ON, Red LED OFF.<br>- Servo position at 90°.<br>- Success chime heard.<br>- Packet alert log printed. | |
| **TC-05** | Auto-Lock Timeout | Wait 5 seconds after TC-04 success. | - LCD clears and goes back to standby "LOCKER SECURE / Enter PIN:".<br>- Green LED turns OFF, Red LED turns ON.<br>- Servo rotates back to 0° (Lock engaged).<br>- Serial reports `AUTO_LOCK` event. | - Auto-relocking occurs precisely 5s after access was granted.<br>- Board returns to locked state. | |
| **TC-06** | Invalid Entry (Attempt 1) | Enter incorrect PIN `9999` and press `#`. | - LCD prints "ACCESS DENIED! / Try Again...".<br>- Buzzer sounds error tone (500Hz) and Red LED flashes 3 times.<br>- Red LED remains ON.<br>- Serial logs `ACCESS_DENIED` with attempt count 1. | - Denial message displayed.<br>- Error alarm tone sounded.<br>- Persistent counter incremented. | |
| **TC-07** | Lockout Trigger (3 Attempts) | Enter wrong PINs two more times consecutively. | - Upon the 3rd failed entry, LCD prints "SYSTEM LOCKED! / Wait: 30 seconds".<br>- System enters `STATE_LOCKOUT`.<br>- Emergency siren begins oscillating continuously.<br>- Red/Green LEDs alternate strobe flash. | - Lockdown begins immediately after 3rd bad attempt.<br>- Siren warbles (alternates 800Hz / 1200Hz).<br>- Keypad entry ignored. | |
| **TC-08** | Keyboard Lockout Safety | Attempt typing on keypad during 30s lockout. | - Keypad input is entirely ignored.<br>- LCD continues counting down lockout seconds. | - No stars appear on LCD.<br>- Key beep sounds are inactive. | |
| **TC-09** | Persistent Lockout on Reset | Power down MCU during lockout, wait 5s, power back ON. | - MCU boots up, detects persistent attempt counter >= 3 from EEPROM.<br>- System immediately resumes Lockout penalty countdown (siren starts and LCD locks). | - Power cycle does not bypass the penalty lockout duration. | |
| **TC-10** | Lockout Clearance | Allow the 30-second penalty timer to run down to 0. | - Siren stops.<br>- LCD updates back to "LOCKER SECURE / Enter PIN:".<br>- Attempt counter resets to 0 in memory and EEPROM. | - Siren silences.<br>- Standby message restored.<br>- Keypad becomes responsive. | |
| **TC-11** | Password Modification | Input correct code `1234#`, then immediately press `A`. | - LCD changes to "CHANGE PASSWORD / New PIN: ".<br>- System enters `STATE_PIN_CHANGE`. | - Input buffer is reset.<br>- User enters PIN change menu. | |
| **TC-12** | Save New Passcode | Input `5678` and press `#` in PIN change mode. | - LCD displays "PASSWORD SET! / Locker Locking".<br>- Success tone plays.<br>- Servo returns to locked state (0°).<br>- EEPROM is updated with PIN `5678`.<br>- System goes back to standby. | - Code successfully changed.<br>- Standby screen active. | |
| **TC-13** | Verification of New Passcode | Enter old passcode `1234#`, then enter new passcode `5678#`. | - `1234#` returns ACCESS DENIED.<br>- `5678#` returns ACCESS GRANTED and unlocks servo. | - Passcode updated and verified persistently. | |

---

## 3. Recommended Debugging Checks

1. **LCD showing solid black squares or no characters**:
   - Adjust the contrast potentiometer on the back of the I2C backpack.
   - Verify I2C address. The default is `0x27`. If it does not respond, run an I2C scanner sketch to locate address (`0x3F` is another common address).
2. **Servo motor vibrating but not moving**:
   - The USB power line from your PC cannot supply enough current. Ensure you are using an external 5V power supply for the motor with a common ground wire linked to the MCU.
3. **Buzzer sounding click noises instead of tones**:
   - Ensure the piezo buzzer is connected to a PWM-capable pin, and that your board is running the correct ESP32/AVR tone library functions.
