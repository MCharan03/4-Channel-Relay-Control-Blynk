# 4-Channel Relay Control via Blynk

This project allows you to control a 4-channel relay module using an ESP8266 and the Blynk mobile app. You can control up to four different appliances or devices from anywhere with an internet connection.

## Features

*   **Remote Control:** Control four relays from your smartphone using the Blynk app.
*   **State Persistence:** The state of the relays is saved in EEPROM, so they will return to their last state after a power outage or reboot.
*   **WiFi Failover:** The device can be configured with multiple WiFi networks and will automatically try to connect to the next available one if the primary network fails.
*   **Over-the-Air (OTA) Updates:** Update the firmware of the ESP8266 wirelessly without needing a physical connection to your computer.

## Hardware Requirements

*   ESP8266 Development Board (e.g., NodeMCU, Wemos D1 Mini)
*   4-Channel Relay Module
*   Jumper Wires
*   A 5V power supply for the relay module (if required)

## Software Requirements

*   [Arduino IDE](https://www.arduino.cc/en/software)
*   [ESP8266 Core for Arduino](https://github.com/esp8266/Arduino)
*   [Blynk Library](https://github.com/blynkkk/blynk-library)
*   A Blynk account and a project set up in the Blynk app.

## Pinout

| ESP8266 Pin | Relay Module |
| :---: | :---: |
| D1 (GPIO5)  | IN1          |
| D2 (GPIO4)  | IN2          |
| D3 (GPIO0)  | IN3          |
| D4 (GPIO2)  | IN4          |

**Note:** You may need to provide a separate 5V power supply to the relay module, depending on its specifications. Connect the `VCC` and `GND` of the relay module to your power supply, and the `IN` pins to the ESP8266 as shown above.

## Setup and Configuration

1.  **Install the required libraries:**
    *   Open the Arduino IDE.
    *   Go to **Sketch > Include Library > Manage Libraries...**
    *   Search for and install the "Blynk" library.

2.  **Configure the Blynk App:**
    *   **Create a Blynk Template:** In the Blynk web dashboard, create a new Template. Note down the `Template ID` and `Template Name`.
    *   **Create a Datastream:** For each relay, create a Virtual Pin Datastream (e.g., V1, V2, V3, V4) of type `Integer` with a range of 0-1.
    *   **Create a Device:** Create a new device using your newly created Template. This will generate an `Auth Token`.
    *   **Configure Mobile App:** In the Blynk mobile app, add a new device using the generated `Auth Token`. Add four "Button" widgets to your dashboard and assign them to Virtual Pins `V1`, `V2`, `V3`, and `V4` respectively. Set their modes to "SWITCH" (or equivalent for toggle behavior).

3.  **Configure the Firmware:**
    *   Open the `4_Ch_relay_control_via_BLYNK.ino` file in the Arduino IDE.
    *   Update the following lines with your Blynk Template ID, Template Name, Auth Token, and WiFi credentials:

    ```cpp
    // Replace with your Blynk Template ID, Name, and Auth Token
    #define BLYNK_TEMPLATE_ID "YOUR_BLYNK_TEMPLATE_ID"
    #define BLYNK_TEMPLATE_NAME "YOUR_BLYNK_TEMPLATE_NAME"
    #define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

    // Replace with your WiFi credentials (supports multiple networks for failover)
    const char* wifiNetworks[][2] = {
      {"YOUR_WIFI_SSID_1", "YOUR_WIFI_PASSWORD_1"},
      {"YOUR_WIFI_SSID_2", "YOUR_WIFI_PASSWORD_2"},
      {"YOUR_WIFI_SSID_3", "YOUR_WIFI_PASSWORD_3"}
      // Add more networks if needed
    };
    ```

4.  **Upload the Firmware:**
    *   Select your ESP8266 board from the **Tools > Board** menu.
    *   Select the correct COM port from the **Tools > Port** menu.
    *   Click the "Upload" button.

## How it Works

The ESP8266 connects to your WiFi network and the Blynk server. When you press a button in the Blynk app, the corresponding virtual pin's state changes. The `BLYNK_WRITE` functions in the code detect this change and toggle the corresponding relay on or off. The state of each relay is saved to the ESP8266's EEPROM, ensuring that the relays return to their last state after a power cycle.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
