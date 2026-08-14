# 👶 ESP8266 IoT Baby Monitoring System with Mobile Wi-Fi Notifications

[![Board](https://img.shields.io/badge/Board-ESP8266_NodeMCU-blue.svg)](https://www.espressif.com/en/products/socs/esp8266)
[![IDE](https://img.shields.io/badge/IDE-Arduino_IDE-00979D.svg)](https://www.arduino.cc/en/software)
[![Cloud](https://img.shields.io/badge/Cloud-Blynk_IoT-23b47e.svg)](https://blynk.io/)
[![Status](https://img.shields.io/badge/Status-Completed-success.svg)](README.md)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

An IoT-based infant safety monitoring system built using the **ESP8266 NodeMCU**, **DHT11**, **Analog Sound Sensor**, and **PIR Motion Sensor**. It continuously monitors ambient temperature, humidity, noise/crying level, and motion around the crib. 

When safety thresholds are breached, the system activates local audio/visual alarms and dispatches instant **mobile push notifications** to the caregiver's smartphone via **Blynk IoT**.

---

## 🌟 Key Features

- **🌡️ Temperature & Humidity Monitoring**: Tracks room conditions via the DHT11 sensor to ensure a comfortable and safe environment.
- **🔊 Cry & Noise Detection**: Reads real-time sound levels via an analog microphone module to alert caregivers when high noise or crying occurs.
- **🚶 Crib Motion Detection**: Monitors movement near the baby crib using a PIR motion sensor.
- **📱 Real-time Mobile Push Notifications**: Sends instant alert notifications directly to the caregiver's mobile phone via Blynk IoT Cloud.
- **⏱️ Notification Spam Prevention**: Implements a non-blocking 60-second cooldown mechanism (`NOTIFICATION_COOLDOWN`) to avoid overwhelming the caregiver with duplicate push alerts.
- **🚨 Local Visual & Audible Alarms**: Activates a status LED and active buzzer immediately upon hazard detection for immediate local alert.
- **🔄 Non-blocking Wi-Fi Reconnection**: Built with asynchronous Wi-Fi recovery logic so sensor monitoring never freezes if internet connectivity drops temporarily.
- **🔕 Remote Alarm Mute Control**: Allows caregivers to silence active alarm buzzers remotely from the mobile app via virtual pin `V8`.
- **💾 Persistent EEPROM Storage**: Dynamic threshold settings configured via mobile sliders survive power cuts and reboots.
- **🩺 Diagnostic Heartbeat Telemetry**: Continuous system health reporting (Uptime, Free Heap Memory, RSSI) sent to virtual pin `V9`.
- **📊 Live Mobile & Web Dashboard**: Visual gauges, charts, and status indicators updated in real-time on the Blynk mobile application.

---

## 🏗️ System Architecture

```text
  +-----------------------+
  |  DHT11 Temp/Humidity  |---- (Pin D5 / GPIO 14) ----+
  +-----------------------+                            |
  |  Analog Sound Sensor  |---- (Pin A0 / ADC0) -------+----> [ ESP8266 NodeMCU ]
  +-----------------------+                            |              |
  |   PIR Motion Sensor   |---- (Pin D1 / GPIO 5) -----+              | Evaluate Thresholds, EEPROM
  +-----------------------+                                           | & Cooldown Logic
                                                                      v
  [ Local LED (D2) + Buzzer (D6) ] <----------------------------------+
                                                                      |
                                                               (2.4GHz Wi-Fi)
                                                                      v
                                                            [ Blynk IoT Cloud ]
                                                                      |
                                                  +-------------------+-------------------+
                                                  |                                       |
                                                  v                                       v
                                  [ Mobile Push Notifications ]               [ Live App Dashboard ]
                                   (Caregiver Smartphone)                       (Gauges & Remote Mute)
```

---

## 🛠️ Required Hardware

| Component | Quantity | Specification / Description |
| :--- | :--- | :--- |
| **ESP8266 NodeMCU v1.0** | 1 | ESP-12E Wi-Fi Microcontroller |
| **DHT11 Sensor** | 1 | Temperature & Humidity Sensor |
| **Sound Sensor Module** | 1 | Analog Output (AO) Microphone Sensor |
| **PIR Motion Sensor** | 1 | HC-SR501 or Mini PIR Module |
| **5V Active Buzzer** | 1 | Local audio alarm |
| **5mm Red LED** | 1 | Warning LED indicator |
| **220Ω Resistor** | 1 | Current limiting resistor for LED |
| **Breadboard & Jumpers** | 1 Set | Male-to-Male & Male-to-Female wires |
| **Micro-USB Cable** | 1 | Power & programming cable |

---

## 🔌 Circuit Pin Connections

| Component Pin | ESP8266 NodeMCU Pin | GPIO | Description |
| :--- | :--- | :--- | :--- |
| **DHT11 Data** | **D5** | GPIO 14 | Digital signal line |
| **DHT11 VCC / GND** | **3.3V / GND** | - | Power supply |
| **Sound Sensor AO** | **A0** | ADC0 | Analog noise output (0 - 1023) |
| **Sound Sensor VCC / GND** | **3.3V / GND** | - | Power supply |
| **PIR Sensor OUT** | **D1** | GPIO 5 | Digital motion output (HIGH on motion) |
| **PIR Sensor VCC / GND** | **VIN (5V) / GND** | - | Power supply |
| **Status LED (+)** | **D2** (via 220Ω) | GPIO 4 | Active HIGH warning LED |
| **Active Buzzer (+)** | **D6** | GPIO 12 | Active HIGH alarm buzzer |

---

## 💻 Software & Libraries

### Required Arduino IDE Libraries:
Install via **Tools -> Manage Libraries...**:

1. **`Blynk`** (by Volodymyr Shymanskyy)
2. **`DHT sensor library`** (by Adafruit)
3. **`Adafruit Unified Sensor`** (by Adafruit)
4. **`ESP8266WiFi`** & **`EEPROM`** (Built into ESP8266 Arduino Board Package)

---

## ⚙️ Configuration & Setup

### 1. Blynk IoT Cloud Setup
1. Create a free account at [blynk.cloud](https://blynk.cloud/).
2. Create a **Template** named `Baby Monitor` (Hardware: `ESP8266`, Connection: `WiFi`).
3. Add **Events** under template settings:
   - `temp_alert`: High Temperature Warning
   - `temp_low_alert`: Low Temperature / Hypothermia Risk Warning
   - `sound_alert`: High Sound/Crying Warning
   - `motion_alert`: Movement Detected Warning
   - *Ensure **Send Push Notification** is enabled for all events.*
4. Add Datastreams:
   - `V0`: Temperature (`Double`, °C)
   - `V1`: Humidity (`Double`, %)
   - `V2`: Sound Level (`Integer`, 0-1023)
   - `V3`: Motion State (`Integer`, 0/1)
   - `V4`: Alert Status (`String`)
   - `V5`: Wi-Fi Signal Strength / RSSI (`Integer`, dBm)
   - `V6`: Dynamic High Temp Threshold Setting (`Double`, 20-45 °C)
   - `V7`: Dynamic Sound Threshold Setting (`Integer`, 100-1023)
   - `V8`: Remote Alarm Mute Switch (`Integer`, 0/1)
   - `V9`: System Diagnostic Telemetry (`String`)
5. Create a new **Device** from template and copy your credentials:
   ```cpp
   #define BLYNK_TEMPLATE_ID   "TMPLxxxxxx"
   #define BLYNK_TEMPLATE_NAME "Baby Monitor"
   #define BLYNK_AUTH_TOKEN    "YOUR_BLYNK_AUTH_TOKEN"
   ```

### 2. Code Configuration
Open [`esp8266_baby_monitor.ino`](esp8266_baby_monitor.ino) and update your Wi-Fi and Blynk settings:

```cpp
const char* WIFI_SSID     = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Configurable Thresholds
float MAX_TEMPERATURE  = 30.0;  // °C
int SOUND_THRESHOLD    = 700;   // ADC value (0-1023)
```

---

## 🚀 How to Upload & Run

1. Open Arduino IDE and add ESP8266 URL in **Preferences**:
   `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
2. Install **ESP8266** board package from **Boards Manager**.
3. Select Board: **NodeMCU 1.0 (ESP-12E Module)**.
4. Connect NodeMCU via USB and select the correct COM Port.
5. Click **Upload** and open **Serial Monitor** at `115200 baud`.

---

## 📋 Pre-Deployment Checklist

Before deploying the monitoring system near the infant's crib, verify the following:

- [x] **2.4GHz Wi-Fi Network**: Ensure the ESP8266 is configured for a 2.4GHz network (5GHz Wi-Fi networks are not supported).
- [x] **Power Supply**: Use a stable 5V / 1A USB power adapter or 18650 Li-ion battery shield for continuous operation.
- [x] **Sensors Calibrated**: Adjust the PIR motion module sensitivity and analog sound module potentiometer for optimal noise thresholding.
- [x] **Blynk Push Notifications**: Confirm push notification permissions are allowed for the Blynk mobile app on your smartphone.
- [x] **Cooldown Test**: Verify that the 60-second notification cooldown operates correctly to prevent notification spam.

---

## 📑 Project Documentation

For complete detailed architecture, testing procedures, circuit diagrams, and troubleshooting steps, refer to:
- 📖 [Full Technical System Guide](baby_monitor_system_guide.md)
- 🔌 [Detailed Circuit & Wiring Specification](CIRCUIT_DIAGRAM.md)
- 🔬 [Hardware Specifications & Power Budget](docs/HARDWARE_SPECIFICATIONS.md)

---

## 📜 License

This project is licensed under the [MIT License](LICENSE) - open for educational and personal use.


