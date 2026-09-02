<div align="center">

# Arduino Alarm Clock

**Multifunctional alarm clock built on Arduino Uno Kit 3.0**

<p>
  <img src="https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white" />
  <img src="https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white" />
  <img src="https://img.shields.io/badge/PlatformIO-FF7F00?style=for-the-badge&logo=platformio&logoColor=white" />
</p>

</div>

---

## About

A full-featured digital alarm clock built on the **Arduino Uno Kit 3.0**. Displays live time and date on an I2C LCD, tracks room temperature and humidity, and lets you set the time, date, and alarm using physical buttons and a joystick. Alarm settings are saved to EEPROM, so they survive a power loss. Built as a coursework project — **scored 100/100**.

##  Features

-  **Live clock screen** — hours, minutes, seconds, and full date, powered by a DS1302 RTC module
-  **Environment monitoring** — temperature and humidity readings from a DHT11 sensor, refreshed every 2 seconds
-  **Configurable alarm** — set the alarm time, toggle it on/off, and it persists across reboots via EEPROM
-  **Alarm trigger screen** — buzzer beeps in on/off pulses and RGB + status LEDs flash until any button is pressed
-  **Dual input control** — 4 push buttons *and* a joystick, both usable to navigate menus and adjust values
-  **Status indication via RGB + LEDs** — color coding shows the current mode at a glance (clock, sensors, editing time, editing date, editing alarm, alarming)
-  **Debounced button handling** — custom debounce logic prevents false triggers on all 4 buttons
-  **EEPROM persistence** — alarm hour, minute, and enabled state are stored with a magic-byte check to detect first boot

##  Screens & Navigation

The clock is built as a state machine with 6 screens:

| State | What it shows | Entered from |
|---|---|---|
| `STATE_CLOCK` | Current time + date, alarm indicator | Startup / back from any screen |
| `STATE_SENSORS` | Temperature & humidity | `MODE` from Clock |
| `STATE_SET_TIME` | Edit hour → minute → second | `NEXT` from Clock |
| `STATE_SET_DATE` | Edit day → month → year | Automatically after Set Time |
| `STATE_SET_ALARM` | Edit alarm hour → minute, toggle on/off | Automatically after Set Date |
| `STATE_ALARMING` | "ALARM!!!" screen, buzzer + LEDs active | Automatically when current time matches alarm time |

**Controls:**
- `BTN_UP` / `BTN_DOWN` or **joystick up/down** — increase / decrease the selected value
- `BTN_NEXT` — confirm the current field and move to the next one
- `BTN_MODE` — switch screens / go back / toggle alarm on-off from the clock screen
- Any button press silences an active alarm

##  Hardware & Pinout

| Component | Pin(s) |
|---|---|
| Button 1 (Up) | D5 |
| Button 2 (Down) | D4 |
| Button 3 (Next/Confirm) | D3 |
| Button 4 (Mode/Back) | D2 |
| Buzzer | D8 |
| RGB LED (R/G/B) | D9 / D10 / D11 |
| Status LED — Red | D6 |
| Status LED — Yellow | D13 |
| Status LED — Blue | D12 |
| DHT11 (temperature & humidity) | D7 |
| DS1302 RTC — CLK / DAT / RST | A1 / A2 / A3 |
| Joystick (Y-axis) | A0 |
| LCD (I2C, 16×2) | I2C (SDA/SCL) |

## Tech Stack

- **Language:** C++ (Arduino framework)
- **Board:** Arduino Uno
- **Build system:** PlatformIO
- **Libraries:**
  - `LiquidCrystal_I2C` — I2C 16×2 LCD driver
  - `DHT sensor library` + `Adafruit Unified Sensor` — temperature/humidity readings
  - `ClearDS1302` — real-time clock driver
  - `EEPROM` — persistent alarm storage

## 📁 Project Structure

```
alarm/
├── include/
│   ├── config.h         # Pin definitions, EEPROM addresses, constants
│   ├── context.h        # Global app state (Context struct, AppState enum)
│   ├── lcd_wrapper.h     # LCD interface
│   ├── rtc_wrapper.h     # RTC interface
│   ├── screens.h         # Screen enter/update function declarations
│   └── sensors.h         # DHT11 sensor interface
├── src/
│   ├── main.cpp                # setup() / loop() — pin init, EEPROM load, state dispatch
│   ├── lcd_wrapper.cpp          # LCD implementation
│   ├── rtc_wrapper.cpp          # RTC implementation
│   ├── sensors.cpp              # DHT11 implementation
│   └── screens/
│       ├── init.cpp             # Boot splash screen
│       └── clock.cpp            # All screen logic: clock, sensors, set time/date/alarm, alarming
└── platformio.ini        # Board & library configuration
```

## Building & Uploading

This project uses [PlatformIO](https://platformio.org/).

1. Install [PlatformIO Core](https://platformio.org/install) or the [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode) for VS Code.
2. Clone this repository and open the `alarm/` folder in PlatformIO.
3. Connect the Arduino Uno via USB.
4. Build and upload:
   ```bash
   pio run --target upload
   ```
5. Open the serial monitor if needed:
   ```bash
   pio device monitor
   ```

## 📝 Notes

- Alarm settings are validated with a magic byte in EEPROM (`0xAB`) — on first boot (empty EEPROM), the clock defaults the alarm to `07:00` (disabled) instead of reading garbage data.
- RTC readings are sanity-checked on every read (e.g. hour clamped to 0–23) to guard against corrupted RTC data.
- This was a coursework project for the *Intelligent Systems* program at **TUKE** — submitted and scored **100/100**.
