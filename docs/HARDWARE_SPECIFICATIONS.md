# 🔬 Hardware Specifications & Power Consumption Guide

This document details technical specifications, power requirements, electrical operating ranges, and power budget calculations for the **ESP8266 IoT Baby Monitoring System**.

---

## ⚡ 1. Microcontroller Unit (MCU)

### ESP8266 NodeMCU v1.0 (ESP-12E)
- **Core**: Tensilica L106 32-bit RISC microprocessor running at 80 MHz / 160 MHz
- **Flash Memory**: 4 MB SPI Flash
- **SRAM**: 80 KB Instruction RAM + 40 KB Data RAM
- **Operating Voltage**: 3.3V DC
- **Input Voltage (VIN Pin)**: 4.5V to 9V DC (Recommended 5V via Micro-USB)
- **Wi-Fi Protocol**: 802.11 b/g/n (2.4 GHz only)
- **Peak Current Draw (Wi-Fi TX)**: ~170 mA to 240 mA
- **Sleep / Idle Current Draw**: ~20 mA

---

## 🌡️ 2. Sensor Specifications

### A. DHT11 Temperature & Humidity Sensor
- **Operating Voltage**: 3.3V – 5.5V DC
- **Temperature Measurement Range**: 0°C to 50°C (Accuracy: ±2°C)
- **Humidity Measurement Range**: 20% to 90% RH (Accuracy: ±5% RH)
- **Sampling Interval**: 1 Hz (1 reading per second)
- **Active Current Draw**: 1.5 mA (Standby: 100 µA)

### B. Analog Sound Sensor Module
- **Operating Voltage**: 3.3V – 5V DC
- **Sensor Type**: Electret Condenser Microphone with LM393 Operational Amplifier
- **Output Type**: Analog Output (`AO`) connected to ESP8266 `A0` (0 – 1.0V internal scale / 0 – 3.3V with onboard voltage divider)
- **Active Current Draw**: 4 mA – 8 mA

### C. PIR Motion Sensor (HC-SR501)
- **Operating Voltage**: 4.5V – 20V DC (Connect to NodeMCU `VIN` pin)
- **Output Signal**: High/Low 3.3V TTL Digital (`HIGH` when motion detected)
- **Detection Distance**: 3 meters to 7 meters (Adjustable via onboard potentiometer)
- **Detection Angle**: < 120° cone
- **Active Current Draw**: 50 µA (Quiescent)

---

## 🚨 3. Actuators & Indicators

### A. Active Buzzer Module (5V)
- **Operating Voltage**: 3.3V – 5V DC
- **Resonant Frequency**: 2300 Hz ± 300 Hz
- **Sound Output Level**: ≥ 85 dB at 10 cm distance
- **Current Draw**: 30 mA

### B. Status Warning LED (Red 5mm)
- **Forward Voltage ($V_f$)**: 2.0V DC
- **Recommended Forward Current ($I_f$)**: 15 mA
- **Current-Limiting Resistor**: 220 Ω ($\frac{3.3V - 2.0V}{0.015A} \approx 86.6 \Omega \rightarrow$ Standard 220 Ω used for safety)

---

## 📊 4. Overall System Power Budget

| Mode / Component | Average Current | Peak Current | Power Consumption @ 5V |
| :--- | :--- | :--- | :--- |
| **ESP8266 MCU (Active Wi-Fi)** | 80 mA | 240 mA | 0.40 W – 1.20 W |
| **DHT11 Sensor** | 1.5 mA | 2.5 mA | 0.01 W |
| **Sound Sensor** | 5.0 mA | 8.0 mA | 0.03 W |
| **PIR Motion Sensor** | 0.05 mA | 0.1 mA | 0.001 W |
| **Warning LED (Red)** | 6.0 mA | 6.0 mA | 0.03 W |
| **Active Buzzer** | 0.0 mA (Normal) | 30.0 mA (Alert) | 0.15 W (Alert) |
| **TOTAL SYSTEM CONSUMPTION** | **~92.5 mA** | **~286.6 mA** | **~0.47 W (Normal) / 1.43 W (Peak Alert)** |

> [!TIP]
> A standard 5V / 1A USB wall adapter provides 1000 mA, leaving over 700 mA of current headroom to ensure rock-solid stability and zero brownout reboots.
