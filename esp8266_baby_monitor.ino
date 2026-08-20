/*
  ================================================================================
  ESP8266 IoT Baby Monitoring System with Mobile Wi-Fi Notifications
  ================================================================================
  Author: DiVYA1136
  Target Board: ESP8266 NodeMCU v1.0 (ESP-12E Module)
  
  Features:
  - Continuously monitors Temperature & Humidity (DHT11), Sound level (Analog Sound Sensor), and Motion (PIR Sensor).
  - Configurable safety thresholds and alert conditions.
  - WiFi auto-reconnection handling (non-blocking).
  - Mobile Push Notifications via Blynk IoT platform.
  - Real-time Cloud/Mobile Dashboard synchronization.
  - Cooldown mechanism to prevent notification spam.
  - Local visual (LED) and audible (Buzzer) alerts.
  - Clean non-blocking architecture using millis() / BlynkTimer.
  ================================================================================
*/

// Define Blynk Template parameters (Must match your Blynk Console)
#define BLYNK_TEMPLATE_ID   "TMPLxxxxxx"
#define BLYNK_TEMPLATE_NAME "Baby Monitor"
#define BLYNK_AUTH_TOKEN    "YOUR_BLYNK_AUTH_TOKEN"

// Included Libraries
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <EEPROM.h>

// ================================================================================
// CONFIGURATION & THRESHOLD VARIABLES
// ================================================================================

// Wi-Fi Credentials
const char* WIFI_SSID     = "YOUR_WIFI_NAME";      // Replace with your Wi-Fi SSID
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";  // Replace with your Wi-Fi Password

// Hardware Pin Definitions (ESP8266 NodeMCU)
#define DHTPIN        14  // NodeMCU D5 (GPIO 14) - DHT11 Data Pin
#define DHTTYPE    DHT11  // Sensor type DHT11
#define PIR_PIN        5  // NodeMCU D1 (GPIO 5)  - PIR Motion Sensor Out
#define SOUND_PIN     A0  // NodeMCU A0 (ADC0)    - Analog Sound Sensor AO
#define LED_PIN        4  // NodeMCU D2 (GPIO 4)  - Status / Warning LED
#define BUZZER_PIN    12  // NodeMCU D6 (GPIO 12) - Local Alert Buzzer

// Safety Thresholds (Example settings - adjust after physical testing)
float MIN_TEMPERATURE  = 18.0;  // Celsius threshold for low temperature alert
float MAX_TEMPERATURE  = 30.0;  // Celsius threshold for high temperature alert
int SOUND_THRESHOLD    = 700;   // Analog reading (0 - 1023) sound threshold

// Timing & Cooldown Settings
const unsigned long SENSOR_READ_INTERVAL = 2000;   // Read sensors every 2 seconds
const unsigned long NOTIFICATION_COOLDOWN = 60000; // 60 seconds cooldown between mobile alerts

// System State Variables
float currentTemp       = 0.0;
float currentHumidity   = 0.0;
int currentSound        = 0;
bool currentMotion      = false;
bool isSystemNormal     = true;
bool isAlarmMuted       = false; // Remote alarm mute toggle state

unsigned long lastSensorReadTime  = 0;
unsigned long lastNotificationTime = 0;

// EEPROM Storage Configuration
const int EEPROM_SIZE = 512;
const uint16_t EEPROM_MAGIC = 0xB4B0; // Magic key to verify initialized memory

// Initialize DHT Sensor and Blynk Timer
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// Function Declarations
void connectToWiFi();
void reconnectWiFi();
void readSensors();
void checkTemperature();
void checkSound();
void checkMotion();
void evaluateSystemStatus();
void sendNotification(String eventCode, String alertMessage);
void updateDashboard();
void loadThresholdsFromEEPROM();
void saveThresholdsToEEPROM();

// ================================================================================
// EEPROM NON-VOLATILE MEMORY HELPERS
// ================================================================================

void loadThresholdsFromEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  uint16_t magic = 0;
  EEPROM.get(0, magic);
  if (magic == EEPROM_MAGIC) {
    EEPROM.get(2, MIN_TEMPERATURE);
    EEPROM.get(6, MAX_TEMPERATURE);
    EEPROM.get(10, SOUND_THRESHOLD);
    Serial.println(F("[EEPROM] Loaded persistent thresholds from memory:"));
    Serial.print(F("        MIN_TEMP: ")); Serial.print(MIN_TEMPERATURE, 1); Serial.println(F(" °C"));
    Serial.print(F("        MAX_TEMP: ")); Serial.print(MAX_TEMPERATURE, 1); Serial.println(F(" °C"));
    Serial.print(F("        SOUND_TH: ")); Serial.println(SOUND_THRESHOLD);
  } else {
    Serial.println(F("[EEPROM] First boot or default memory. Writing defaults..."));
    saveThresholdsToEEPROM();
  }
}

void saveThresholdsToEEPROM() {
  EEPROM.put(0, EEPROM_MAGIC);
  EEPROM.put(2, MIN_TEMPERATURE);
  EEPROM.put(6, MAX_TEMPERATURE);
  EEPROM.put(10, SOUND_THRESHOLD);
  EEPROM.commit();
  Serial.println(F("[EEPROM] Thresholds saved to persistent flash memory."));
}

// ================================================================================
// INITIALIZATION SETUP
// ================================================================================
void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println(F("=========================================="));
  Serial.println(F(" ESP8266 Infant Safety Monitoring System  "));
  Serial.println(F("=========================================="));

  // Load saved threshold settings from non-volatile EEPROM
  loadThresholdsFromEEPROM();

  // Configure Hardware Pins
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Default Pin States
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Initialize DHT Sensor
  dht.begin();
  Serial.println(F("[DHT11] Sensor initialized successfully."));

  // Connect to Wi-Fi Network & Blynk Cloud
  connectToWiFi();

  // Set up periodic sensor reading timer via BlynkTimer
  timer.setInterval(SENSOR_READ_INTERVAL, readSensors);
}

// ================================================================================
// MAIN LOOP (NON-BLOCKING)
// ================================================================================
void loop() {
  // Maintain Wi-Fi Connection
  reconnectWiFi();

  // Run Blynk process if connected
  if (Blynk.connected()) {
    Blynk.run();
  }

  // Execute scheduled timers (Sensor reading and cloud updates)
  timer.run();
}

// ================================================================================
// NETWORK MANAGEMENT FUNCTIONS
// ================================================================================

// Connect to Wi-Fi and Blynk on Startup
void connectToWiFi() {
  Serial.print(F("Connecting to WiFi: "));
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(F("."));
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println(F("WiFi connected successfully!"));
    Serial.print(F("IP Address: "));
    Serial.println(WiFi.localIP());

    // Connect to Blynk IoT Server
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect(5000); // 5 sec timeout
  } else {
    Serial.println();
    Serial.println(F("WiFi connection failed! Will retry in main loop..."));
  }
}

// Automatic Non-blocking Wi-Fi Reconnection
void reconnectWiFi() {
  static unsigned long lastReconnectAttempt = 0;
  unsigned long now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastReconnectAttempt > 10000) { // Retry every 10 seconds
      lastReconnectAttempt = now;
      Serial.println(F("[NETWORK] Wi-Fi lost! Attempting background reconnection..."));
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    }
  }
}

// ================================================================================
// SENSOR READING & LOGIC PROCESSING
// ================================================================================

// Master Function to Read All Connected Sensors
void readSensors() {
  // 1. Read Temperature and Humidity from DHT11
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Validate DHT11 Reading
  if (isnan(t) || isnan(h)) {
    Serial.println(F("[WARNING] Failed to read from DHT11 sensor! Using previous values."));
  } else {
    currentTemp = t;
    currentHumidity = h;
  }

  // 2. Read Sound Level with Multi-Sample Noise Averaging Filter
  long soundSum = 0;
  for (int i = 0; i < 10; i++) {
    soundSum += analogRead(SOUND_PIN);
    delayMicroseconds(150);
  }
  currentSound = (int)(soundSum / 10);

  // 3. Read Motion State from PIR Sensor
  currentMotion = (digitalRead(PIR_PIN) == HIGH);

  // Print Serial Telemetry
  Serial.println();
  Serial.println(F("--- Sensor Telemetry ---"));
  Serial.print(F("Temperature: ")); Serial.print(currentTemp, 1); Serial.println(F(" °C"));
  Serial.print(F("Humidity:    ")); Serial.print(currentHumidity, 1); Serial.println(F(" %"));
  Serial.print(F("Sound Level: ")); Serial.println(currentSound);
  Serial.print(F("Motion:      ")); Serial.println(currentMotion ? F("DETECTED") : F("NOT DETECTED"));
  Serial.print(F("Wi-Fi Status: ")); Serial.println(WiFi.status() == WL_CONNECTED ? F("CONNECTED") : F("DISCONNECTED"));

  // Check Thresholds & Fire Alerts
  checkTemperature();
  checkSound();
  checkMotion();

  // Evaluate Overall System Safety Status
  evaluateSystemStatus();

  // Sync Data to Blynk Cloud Dashboard
  updateDashboard();
}

// Check Temperature Thresholds (High and Low)
void checkTemperature() {
  if (currentTemp > MAX_TEMPERATURE) {
    Serial.println(F("WARNING: High temperature detected!"));
    
    String alertMsg = "⚠️ Baby Monitoring Alert:\nTemperature is above maximum safety limit.\nCurrent temperature: " 
                      + String(currentTemp, 1) + " °C";
                      
    sendNotification("temp_alert", alertMsg);
  } else if (currentTemp < MIN_TEMPERATURE && currentTemp > 0.0) {
    Serial.println(F("WARNING: Low temperature detected!"));

    String alertMsg = "❄️ Baby Monitoring Alert:\nTemperature is below minimum comfortable limit.\nCurrent temperature: " 
                      + String(currentTemp, 1) + " °C";

    sendNotification("temp_low_alert", alertMsg);
  }
}

// Check High Sound Level Threshold
void checkSound() {
  if (currentSound > SOUND_THRESHOLD) {
    Serial.println(F("WARNING: High sound level detected!"));

    String alertMsg = "🔊 Baby Monitoring Alert:\nHigh sound detected.\nSound level: " 
                      + String(currentSound);

    sendNotification("sound_alert", alertMsg);
  }
}

// Check Motion Detection
void checkMotion() {
  if (currentMotion) {
    Serial.println(F("WARNING: Motion detected in crib area!"));

    String alertMsg = "🚶 Baby Monitoring Alert:\nMotion detected in the monitoring area.";

    sendNotification("motion_alert", alertMsg);
  }
}

// Evaluate Overall System Status and Control Local Hardware Indicators
void evaluateSystemStatus() {
  if (currentTemp > MAX_TEMPERATURE || currentTemp < MIN_TEMPERATURE || currentSound > SOUND_THRESHOLD || currentMotion) {
    isSystemNormal = false;
    digitalWrite(LED_PIN, HIGH);     // Turn ON warning LED continuously
    if (!isAlarmMuted) {
      digitalWrite(BUZZER_PIN, HIGH);  // Turn ON local buzzer if not muted
    } else {
      digitalWrite(BUZZER_PIN, LOW);   // Muted remotely by caregiver
    }
    Serial.println(F("System Status: ALERT / ABNORMAL"));
  } else {
    isSystemNormal = true;
    digitalWrite(BUZZER_PIN, LOW);   // Turn OFF local buzzer
    // Heartbeat LED pulse (brief 50ms pulse to indicate active monitoring)
    digitalWrite(LED_PIN, HIGH);
    delay(50);
    digitalWrite(LED_PIN, LOW);
    Serial.println(F("System Status: NORMAL (Heartbeat pulse active)"));
  }
}

// ================================================================================
// MOBILE NOTIFICATION & DASHBOARD SYNC
// ================================================================================

// Send Mobile Push Notification with Cooldown Spam Prevention
void sendNotification(String eventCode, String alertMessage) {
  unsigned long currentTime = millis();

  // Check if cooldown period has passed
  if (currentTime - lastNotificationTime >= NOTIFICATION_COOLDOWN || lastNotificationTime == 0) {
    lastNotificationTime = currentTime;

    Serial.println(F("Sending mobile notification..."));

    if (Blynk.connected()) {
      // Send mobile push notification event via Blynk IoT
      Blynk.logEvent(eventCode.c_str(), alertMessage);
      Serial.println(F("Notification sent successfully via Blynk!"));
    } else {
      Serial.println(F("[ERROR] Cannot send notification: Blynk disconnected!"));
    }
  } else {
    Serial.print(F("[COOLDOWN] Notification skipped to prevent spam. Time remaining: "));
    Serial.print((NOTIFICATION_COOLDOWN - (currentTime - lastNotificationTime)) / 1000);
    Serial.println(F(" s"));
  }
}

// Send Real-time Data to Blynk Cloud Dashboard Virtual Pins
void updateDashboard() {
  if (!Blynk.connected()) return;

  // Virtual Pin Mapping:
  // V0: Temperature (°C)
  // V1: Humidity (%)
  // V2: Sound Level (0-1023)
  // V3: Motion State (0=Clear, 1=Detected)
  // V4: System Alert Status (String)
  // V5: Wi-Fi Signal Strength / Status (dBm)
  // V6: Dynamic High Temp Threshold Input (20.0 - 45.0 °C)
  // V7: Dynamic Sound Threshold Input (100 - 1023)
  // V8: Remote Alarm Mute Switch (0=Unmuted, 1=Muted)
  // V9: System Diagnostics & Uptime String
  // V10: Dynamic Low Temp Threshold Input (15.0 - 25.0 °C)

  Blynk.virtualWrite(V0, currentTemp);
  Blynk.virtualWrite(V1, currentHumidity);
  Blynk.virtualWrite(V2, currentSound);
  Blynk.virtualWrite(V3, currentMotion ? 1 : 0);
  Blynk.virtualWrite(V4, isSystemNormal ? "NORMAL" : "ALERT!");
  Blynk.virtualWrite(V5, WiFi.RSSI());
  Blynk.virtualWrite(V8, isAlarmMuted ? 1 : 0);

  // Transmit Diagnostic Heartbeat Telemetry (Uptime & Free Memory)
  String diagInfo = "Uptime: " + String(millis() / 60000) + "m | FreeHeap: " + String(ESP.getFreeHeap()) + "B";
  Blynk.virtualWrite(V9, diagInfo);
}

// Dynamic High Temperature Threshold update from Blynk App Slider/Numeric Input (V6)
BLYNK_WRITE(V6) {
  float val = param.asFloat();
  if (val >= 20.0 && val <= 45.0) {
    MAX_TEMPERATURE = val;
    saveThresholdsToEEPROM();
    Serial.print(F("[BLYNK] Dynamic update: MAX_TEMPERATURE set to "));
    Serial.print(MAX_TEMPERATURE, 1);
    Serial.println(F(" °C"));
  }
}

// Dynamic Sound Threshold update from Blynk App Slider/Numeric Input (V7)
BLYNK_WRITE(V7) {
  int val = param.asInt();
  if (val >= 100 && val <= 1023) {
    SOUND_THRESHOLD = val;
    saveThresholdsToEEPROM();
    Serial.print(F("[BLYNK] Dynamic update: SOUND_THRESHOLD set to "));
    Serial.println(SOUND_THRESHOLD);
  }
}

// Remote Alarm Mute Switch from Blynk App Button Widget (V8)
BLYNK_WRITE(V8) {
  int val = param.asInt();
  isAlarmMuted = (val == 1);
  if (isAlarmMuted) {
    digitalWrite(BUZZER_PIN, LOW); // Silence active buzzer immediately
    Serial.println(F("[BLYNK] Remote Alarm MUTE activated by caregiver."));
  } else {
    Serial.println(F("[BLYNK] Remote Alarm UNMUTED by caregiver."));
  }
}

// Dynamic Low Temperature Threshold update from Blynk App Slider/Numeric Input (V10)
BLYNK_WRITE(V10) {
  float val = param.asFloat();
  if (val >= 15.0 && val <= 25.0) {
    MIN_TEMPERATURE = val;
    saveThresholdsToEEPROM();
    Serial.print(F("[BLYNK] Dynamic update: MIN_TEMPERATURE set to "));
    Serial.print(MIN_TEMPERATURE, 1);
    Serial.println(F(" °C"));
  }
}

