// ✅ Include Required Libraries
#define BLYNK_TEMPLATE_ID "YOUR_BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "YOUR_BLYNK_TEMPLATE_NAME"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h> // For state persistence on ESP8266

// Define Blynk Virtual Pins for each relay
#define VIRTUAL_PIN_RELAY_1 V1
#define VIRTUAL_PIN_RELAY_2 V2
#define VIRTUAL_PIN_RELAY_3 V3
#define VIRTUAL_PIN_RELAY_4 V4

// ✅ Wi-Fi Credentials (Supports Multiple Networks)
const char* wifiNetworks[][2] = {
  {"YOUR_WIFI_SSID_1", "YOUR_WIFI_PASSWORD_1"},
  {"YOUR_WIFI_SSID_2", "YOUR_WIFI_PASSWORD_2"},
  {"YOUR_WIFI_SSID_3", "YOUR_WIFI_PASSWORD_3"}
  // Add more networks if needed
};
#define WIFI_COUNT 3

// Define EEPROM size for relay states (1 byte per relay)
#define EEPROM_SIZE 4

// ✅ Relay Pins (for 4-Channel Relay)
#define RELAY1  5  // D1 (GPIO5)
#define RELAY2  4  // D2 (GPIO4)
#define RELAY3  0  // D3 (GPIO0)
#define RELAY4  2  // D4 (GPIO2)

// ✅ Function to Connect to Wi-Fi (Failover)
bool connectWiFi() {
  Serial.println("Attempting to connect to WiFi...");
  for (int i = 0; i < WIFI_COUNT; i++) {
    Serial.printf("Connecting to %s...\n", wifiNetworks[i][0]);
    WiFi.begin(wifiNetworks[i][0], wifiNetworks[i][1]);
    int retries = 40; // Try for ~20 seconds (500ms * 40)

    while (WiFi.status() != WL_CONNECTED && retries > 0) {
      delay(500);
      Serial.print(".");
      retries--;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\n✅ WiFi Connected to %s, IP: %s\n", wifiNetworks[i][0], WiFi.localIP().toString().c_str());
      return true;  // Connection successful
    }
    Serial.println("\nFailed to connect to this network.");
  }
  Serial.println("❌ All WiFi networks failed.");
  return false;  // Connection failed
}

// ✅ Function to Connect Blynk
void connectBlynk() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connecting to Blynk...");
    Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str());
    if (Blynk.connected()) {
      Serial.println("✅ Blynk Connected");
    } else {
      Serial.println("❌ Blynk connection failed.");
    }
  } else {
    Serial.println("WiFi not connected, cannot connect to Blynk.");
  }
}

// This function is called every time the device connects to the Blynk Cloud
BLYNK_CONNECTED() {
  Serial.println("Blynk connected. Requesting current states from server...");
  // Request the current state of all virtual pins from the server
  Blynk.syncAll();
  // Also send local states to ensure consistency if syncAll doesn't cover all cases immediately
  // This is handled by the BLYNK_WRITE callbacks when syncAll triggers them.
  // However, if a relay state was changed locally (e.g., by a physical button, not implemented yet) and not synced, this would push it.
  // For now, relying on Blynk.syncAll() to trigger BLYNK_WRITE is sufficient.
}

// ✅ OTA Setup
void setupOTA() {
  ArduinoOTA.setHostname("ESP8266-Relay-Control");
  ArduinoOTA.onProgress([](unsigned int p, unsigned int t) {
    Serial.printf("OTA: %u%%\r", (p * 100) / t);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    if (error == OTA_AUTH_ERROR) Serial.println("\n❌ OTA Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("\n❌ OTA Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("\n❌ OTA Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("\n❌ OTA Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("\n❌ OTA End Failed");
  });
  ArduinoOTA.begin();
}

// --- Global Variables ---
int relayPins[] = {RELAY1, RELAY2, RELAY3, RELAY4};
int numRelays = sizeof(relayPins) / sizeof(relayPins[0]);

// --- Helper Functions ---

void setRelayState(int relayIndex, int state) {
  if (relayIndex >= 0 && relayIndex < numRelays) {
    digitalWrite(relayPins[relayIndex], state);
    EEPROM.write(relayIndex, state); // Save state to EEPROM
    EEPROM.commit(); // Commit changes to EEPROM
    Serial.printf("Relay %d (Pin %d) set to state: %s and saved.\n", relayIndex + 1, relayPins[relayIndex], state == HIGH ? "HIGH" : "LOW");
    // Update Blynk app to reflect the new state (important for sync and manual changes)
    Blynk.virtualWrite(VIRTUAL_PIN_RELAY_1 + relayIndex, state); // V1 + 0 = V1, V1 + 1 = V2 etc.
  } else {
    Serial.println("Invalid relay index.");
  }
}

int readRelayState(int relayIndex) {
  if (relayIndex >= 0 && relayIndex < numRelays) {
    // Default to HIGH if no state is saved (first boot) because relays are active LOW
    return EEPROM.read(relayIndex);
  }
  return HIGH; // Default for invalid index
}

void setup() {
  Serial.begin(115200);
  delay(100); // Give serial a moment to initialize

  Serial.println("Initializing EEPROM...");
  EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM with defined size

  // Initialize relay pins and restore last known states
  for (int i = 0; i < numRelays; i++) {
    pinMode(relayPins[i], OUTPUT);
    int savedState = readRelayState(i);
    // EEPROM.read() returns 255 if address has never been written. Treat as default HIGH.
    if (savedState == 255) savedState = HIGH; 
    digitalWrite(relayPins[i], savedState);
    Serial.printf("Relay %d (Pin %d) initialized to state: %s\n", i + 1, relayPins[i], savedState == HIGH ? "HIGH" : "LOW");
  }

  // ✅ Connect to Wi-Fi and then Blynk
  if (connectWiFi()) {
    connectBlynk();
  }

  // ✅ Setup OTA
  setupOTA();
}

// ✅ Auto Reconnect Logic in loop()
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    if (connectWiFi()) connectBlynk(); // Reconnect Blynk after Wi-Fi restores
  }
  if (Blynk.connected()) Blynk.run();  // Keep Blynk Running
  ArduinoOTA.handle();                 // Keep OTA Running
}

// ✅ Blynk Virtual Pin Handling for 4 Relays
BLYNK_WRITE(V1) {
  Serial.println("🔄 Syncing Relay 1...");
  setRelayState(0, param.asInt() ? LOW : HIGH); // relayIndex 0 for RELAY1
}

BLYNK_WRITE(V2) {
  Serial.println("🔄 Syncing Relay 2...");
  setRelayState(1, param.asInt() ? LOW : HIGH); // relayIndex 1 for RELAY2
}

BLYNK_WRITE(V3) {
  Serial.println("🔄 Syncing Relay 3...");
  setRelayState(2, param.asInt() ? LOW : HIGH); // relayIndex 2 for RELAY3
}

BLYNK_WRITE(V4) {
  Serial.println("🔄 Syncing Relay 4...");
  setRelayState(3, param.asInt() ? LOW : HIGH); // relayIndex 3 for RELAY4
}