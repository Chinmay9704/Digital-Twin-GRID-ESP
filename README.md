# ESP8266 Sensor Data Publisher

This project is designed to run on an ESP8266 microcontroller and communicate with an STM32 microcontroller to collect sensor data. The ESP8266 connects to a WiFi network and publishes the sensor data to an MQTT broker. The project supports multiple sensor readings, including temperature, humidity, RPM, gas levels, and object count.

## Features

- **WiFi Connectivity**: Connects to a specified WiFi network.
- **MQTT Communication**: Publishes sensor data to an MQTT broker.
- **Data Parsing**: Processes data received from the STM32 microcontroller.
- **Watchdog Timer**: Monitors data reception and handles timeouts.
- **Error Handling**: Reconnects to WiFi and MQTT broker if disconnected.

## Project Structure

## Dependencies

The project uses the following libraries:

- [PubSubClient](https://github.com/knolleary/pubsubclient): A lightweight MQTT client for Arduino.

## Configuration

The project configuration is managed using the `platformio.ini` file. Key settings include:

- **Platform**: `espressif8266`
- **Board**: `esp12e`
- **Framework**: `arduino`
- **Library Dependencies**: `knolleary/PubSubClient@^2.8`
- **Monitor Speed**: `9600`

## How It Works

1. **WiFi Setup**: The ESP8266 connects to the specified WiFi network using the credentials provided in the source code.
2. **MQTT Setup**: The ESP8266 connects to an MQTT broker using the provided server address, port, username, and password.
3. **Data Reception**: The ESP8266 receives sensor data from the STM32 microcontroller over UART.
4. **Data Parsing**: The received data is parsed to extract temperature, humidity, RPM, gas levels, and object count.
5. **Data Publishing**: The parsed data is published to the MQTT broker under specific topics.

## Topics Used

- `sensors/temperature`: Publishes temperature readings.
- `sensors/humidity`: Publishes humidity readings.
- `sensors/rpm`: Publishes RPM readings.
- `sensors/gas`: Publishes gas level readings.
- `sensors/object_count`: Publishes object count readings.

## Setup Instructions

1. Clone this repository to your local machine.
2. Install [PlatformIO](https://platformio.org/) in your IDE (e.g., VS Code).
3. Open the project in your IDE.
4. Update the WiFi and MQTT credentials in `src/main.cpp`:
   ```cpp
   const char* ssid = "YourWiFiSSID";
   const char* password = "YourWiFiPassword";
   const char* mqtt_server = "YourMQTTBrokerIP";
   const int mqtt_port = 1883;
   const char* mqtt_user = "YourMQTTUsername";
   const char* mqtt_password = "YourMQTTPassword";
