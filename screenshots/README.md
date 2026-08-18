# Screenshots Checklist & Guide

To make your GitHub repository stand out as a professional proof of work, you must include visual evidence of your system operating under different conditions. Capture the following screenshots and save them in this folder.

### Recommended File Naming Convention
Use descriptive names for your files, such as `01_standby_lcd.png`, `02_unlock_servo.png`, etc., and embed them in your root `README.md` or a project report.

---

## 📸 Screenshots Checklist

### 1. Hardware/Simulation Wiring Layout
- [ ] **Wokwi Board Layout**: A high-resolution capture of the complete Wokwi circuit workspace showing all wires, LEDs, keypad, LCD, and servo connected.
- [ ] **Circuit Diagram Schematic**: A schematic drawing or Fritzing layout of the system wiring.

### 2. Locker System Operation States
- [ ] **Standby Screen**: The I2C LCD displaying `Locker Secure / Enter PIN:` with the Red LED active.
- [ ] **PIN Typing Screen**: The LCD displaying stars (`* * *`) as keys are clicked.
- [ ] **Access Granted Screen**: The LCD displaying `ACCESS GRANTED / Locker Opened!`, the Green LED active, and the Red LED inactive.
- [ ] **Unlocked Actuator State**: The Servo motor rotated to exactly `90 degrees` during the 5-second unlock window.
- [ ] **Access Denied Screen**: The LCD displaying `ACCESS DENIED! / Try Again...` after an incorrect PIN entry.
- [ ] **System Lockout Screen**: The LCD showing `SYSTEM LOCKED! / Wait: 30 seconds` immediately after the 3rd wrong attempt.
- [ ] **Active Alarm Indicator**: Alternating red/green LED flashing states captured during the lockout penalty.

### 3. Log Captures
- [ ] **Arduino IDE / Wokwi Serial Monitor**: A screenshot showing the system boot diagnostic messages, loaded credentials, keypad press characters, and the `[REMOTE ALERT]` prints.

### 4. Git Repository Completeness
- [ ] **PlatformIO/Arduino Folder Hierarchy**: A capture of your IDE file explorer demonstrating the organized, modular layout (`src/`, `docs/`, `arduino_code/`, etc.).
- [ ] **GitHub Main Page Preview**: A screenshot of your repository landing page, showing clean commit histories and the fully rendered README.
