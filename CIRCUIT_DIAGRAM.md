# 🔌 ESP8266 IoT Baby Monitoring System - Circuit & Wiring Specification

This document provides complete electrical connection details, voltage requirements, and pin assignments for building the **ESP8266 Infant Safety Monitoring System**.

---

## ⚡ Electrical Power Requirements

* **ESP8266 NodeMCU**: Powered via Micro-USB (5V DC from adapter) or `VIN` pin (5V DC input). Operating logic is **3.3V**.
* **DHT11 Sensor Module**: Requires **3.3V or 5V VCC** and **GND**.
* **Analog Sound Sensor**: Requires **3.3V or 5V VCC** and **GND**. Output `AO` connects to ESP8266 `A0` pin (0 - 1.0V internal ADC range; NodeMCU onboard resistor divider handles 0 - 3.3V).
* **PIR Motion Sensor (HC-SR501)**: Requires **5V VCC** (connect to `VIN` pin) and **GND**. Digital output is 3.3V logic tolerant.
* **5V Active Buzzer**: Driven directly by GPIO 12 (`D6`).
* **Warning LED**: Connected to GPIO 4 (`D2`) with a **220Ω current-limiting resistor** in series.

---

## 📌 Complete Pin Wiring Matrix

| Component | Pin Label | NodeMCU Board Pin | ESP8266 GPIO | Wire Color (Recommended) | Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **DHT11** | VCC | `3V3` / `VIN` | - | Red | 3.3V/5V Power |
| | GND | `GND` | - | Black | Common Ground |
| | DATA | `D5` | GPIO 14 | Yellow | Temperature & Humidity Data Line |
| **Sound Sensor** | VCC | `3V3` / `VIN` | - | Red | 3.3V/5V Power |
| | GND | `GND` | - | Black | Common Ground |
| | AO | `A0` | ADC0 | Blue | Analog Noise Signal |
| **PIR Sensor** | VCC | `VIN` | - | Red | 5V Power Supply |
| | GND | `GND` | - | Black | Common Ground |
| | OUT | `D1` | GPIO 5 | Green | Digital Motion Signal (HIGH = Motion) |
| **Status LED** | Anode (+) | `D2` (via 220Ω) | GPIO 4 | Orange | Warning LED Positive |
| | Cathode (-) | `GND` | - | Black | Warning LED Ground |
| **Active Buzzer**| Positive (+) | `D6` | GPIO 12 | Purple | Buzzer Signal |
| | Negative (-) | `GND` | - | Black | Buzzer Ground |

---

## 🎨 Breadboard Layout Diagram

```text
                        +---------------------------+
                        |      ESP8266 NodeMCU      |
                        +---------------------------+
                        | 3V3                    D5 |----> DHT11 DATA Pin
                        | GND                    D1 |----> PIR Sensor OUT Pin
                        | VIN                    A0 |----> Sound Sensor AO Pin
                        | GND                    D2 |----> [220Ω Resistor] ----> (+) Red LED (-) ----> GND
                        | GND                    D6 |----> Active Buzzer (+) ----> (-) GND
                        +---------------------------+
```

---

## ⚠️ Important Safety & Assembly Notes

1. **Avoid GPIO Pull Restrictions**:
   - Do NOT use GPIO 0 (`D3`), GPIO 2 (`D4`), or GPIO 15 (`D8`) for sensors that output `HIGH` during boot, as this may prevent the ESP8266 from booting correctly.
   - The selected pins (`D1`, `D2`, `D5`, `D6`) leave boot configuration pins unaffected.
2. **DHT11 Pull-up Resistor**:
   - If using a bare 4-pin DHT11 chip (instead of a 3-pin module), add a **10kΩ pull-up resistor** between VCC and DATA lines.
3. **Sound Sensor Potentiometer Calibration**:
   - Turn the blue potentiometer screw on the sound sensor module to calibrate the ambient noise baseline.
