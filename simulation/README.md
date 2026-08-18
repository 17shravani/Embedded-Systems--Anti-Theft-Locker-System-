# Virtual Simulation Guide (Wokwi)

To allow verification and grading without physical hardware, a complete virtual simulation configuration has been provided. 

---

## 1. Quick Start with Wokwi

1. Open a web browser and go to [Wokwi ESP32 Project Creator](https://wokwi.com/projects/new/esp32).
2. Look at the tab structure in the project editor window. You will see a `sketch.ino` (or `main.cpp`) tab and a `diagram.json` tab.
3. Open `diagram.json` in this folder, copy its entire contents, and replace everything in the Wokwi `diagram.json` tab.
4. Copy the complete code from [arduino_code.ino](../arduino_code/arduino_code.ino) and paste it into the Wokwi code editor tab.
5. In Wokwi's Library Manager (search on the left-hand panel or edit the `libraries.txt` tab if present), add the following libraries:
   - `Keypad` by Mark Stanley, Alexander Brevig
   - `LiquidCrystal I2C` by Frank de Brabander
   - `ESP32Servo` by Kevin Harrington (for ESP32 servo control)
6. Click the **Play** button (green arrow) to compile and run the simulation.

---

## 2. Interactive Testing Flow

### Scenario A: Authorized Access (Unlock)
1. The LCD will display `Locker Secure` and `Enter PIN:`.
2. The **Red LED** is active (ON), showing the locker is locked.
3. Click the buttons on the keypad: `1` `2` `3` `4`. The screen prints a masked character (`*`) for each digit.
4. Press the `#` key to submit the entry.
5. Watch the output:
   - The LCD clears and displays `Access Granted`.
   - The **Green LED** lights up; the **Red LED** goes off.
   - The **Servo Motor** rotates 90 degrees to represent the bolt retracting.
   - Wait 5 seconds: the servo automatically returns to 0 degrees, the Red LED lights back up, and the LCD prints `Locker Secure / Enter PIN:`.

### Scenario B: Wrong PIN Entry
1. Press `5` `5` `5` `5` and then `#`.
2. Notice:
   - LCD prints `Access Denied!`.
   - The **Buzzer** emits a short warning tone.
   - The **Red LED** blinks.
   - The attempt count increases.

### Scenario C: Tamper Alarm Trigger (3 Wrong Attempts)
1. Repeat the incorrect PIN process three consecutive times.
2. Upon the 3rd wrong entry, the system enters the alarm and lockout state:
   - LCD prints `SYSTEM LOCKED! / Alarm Sounding...`
   - The **Buzzer** alerts continuously with a high-pitched siren.
   - The **Red LED** flashes rapidly.
   - Try typing on the keypad: it is entirely locked out.
   - The system resets to normal after a 30-second lockout timer expires.
