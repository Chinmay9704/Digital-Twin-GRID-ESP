#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// WiFi and MQTT settings
const char* ssid = "iQOO Neo 7";
const char* password = "12345678";
const char* mqtt_server = "144.126.254.154";
const int mqtt_port = 1883;
const char* mqtt_user = "solstxce";
const char* mqtt_password = "1234";
const char* temp_topic = "sensors/temperature";
const char* humid_topic = "sensors/humidity";
const char* rpm_topic = "sensors/rpm";           // New topic for RPM
const char* gas_topic = "sensors/gas";           // New topic for Gas readings
const char* object_topic = "sensors/object_count";  // New topic for object count
const char* client_id = "ESP8266_TempSensor";

// Increase buffer size for more reliable serial communication
const int BUFFER_SIZE = 128;
char serialBuffer[BUFFER_SIZE];
int bufferIndex = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// Add watchdog timer
unsigned long lastDataReceived = 0;
const unsigned long DATA_TIMEOUT = 10000; // 10 seconds

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.println("IP address: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi connection failed! Restarting...");
    ESP.restart();
  }
}

void reconnect() {
  int retries = 0;
  while (!client.connected() && retries < 3) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(client_id, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.publish("esp8266/status", "connected");
    } else {
      Serial.printf("failed, rc=%d ", client.state());
      delay(2000);
      retries++;
    }
  }
}

void setup() {
  // Initialize UART
  Serial.begin(9600);  // Use hardware serial for STM32 communication
  
  // Wait for serial to initialize
  delay(2000);
  Serial.println("\nESP8266 starting up...");
  
  setup_wifi();
  client.setKeepAlive(60);
  client.setSocketTimeout(10);
  client.setServer(mqtt_server, mqtt_port);
  
  Serial.println("ESP8266 initialization complete");
  Serial.println("Waiting for data from STM32...");
}

void processData(const String& data) {
  Serial.print("Processing data: ");
  Serial.println(data);
  
  if (data.length() < 5) {
    Serial.println("Data too short, skipping");
    return;
  }
  
  // Parse the data string for all parameters
  int tIndex = data.indexOf("T:");
  int hIndex = data.indexOf(",H:");
  int rIndex = data.indexOf(",R:");
  int gIndex = data.indexOf(",G:");
  int oIndex = data.indexOf(",O:");  // New index for object count
  
  // Check if temperature and humidity are present
  if (tIndex != -1 && hIndex != -1) {
    // Extract temperature
    String tempStr = data.substring(tIndex + 2, hIndex);
    float temperature = tempStr.toFloat();
    
    // Extract humidity
    String humStr;
    if (rIndex != -1) {
      humStr = data.substring(hIndex + 3, rIndex);
    } else if (gIndex != -1) {
      humStr = data.substring(hIndex + 3, gIndex);
    } else if (oIndex != -1) {
      humStr = data.substring(hIndex + 3, oIndex);
    } else {
      humStr = data.substring(hIndex + 3);
    }
    float humidity = humStr.toFloat();
    
    // Extract RPM if present
    float rpm = 0.0;
    if (rIndex != -1) {
      String rpmStr;
      if (gIndex != -1) {
        rpmStr = data.substring(rIndex + 3, gIndex);
      } else if (oIndex != -1) {
        rpmStr = data.substring(rIndex + 3, oIndex);
      } else {
        rpmStr = data.substring(rIndex + 3);
      }
      rpm = rpmStr.toFloat();
    }
    
    // Extract Gas reading if present
    float gas = 0.0;
    if (gIndex != -1) {
      String gasStr;
      if (oIndex != -1) {
        gasStr = data.substring(gIndex + 3, oIndex);
      } else {
        gasStr = data.substring(gIndex + 3);
      }
      gas = gasStr.toFloat();
    }
    
    // Extract Object count if present
    int objectCount = 0;
    if (oIndex != -1) {
      String objectStr = data.substring(oIndex + 3);
      objectCount = objectStr.toInt();
    }
    
    // Debug output
    Serial.printf("Parsed values - Temp: %.2f, Humidity: %.2f, RPM: %.2f, Gas: %.2f, Objects: %d\n", 
                  temperature, humidity, rpm, gas, objectCount);
    
    // Validate readings
    if (temperature >= -40 && temperature <= 80 && 
        humidity >= 0 && humidity <= 100) {
      
      // Prepare data for MQTT
      char tempBuffer[10], humidBuffer[10], rpmBuffer[10], gasBuffer[10], objectBuffer[10];
      dtostrf(temperature, 4, 2, tempBuffer);
      dtostrf(humidity, 4, 2, humidBuffer);
      dtostrf(rpm, 4, 2, rpmBuffer);
      dtostrf(gas, 4, 2, gasBuffer);
      sprintf(objectBuffer, "%d", objectCount);
      
      // Publish data if connected to MQTT broker
      if (client.connected()) {
        // Publish temperature and humidity
        client.publish(temp_topic, tempBuffer);
        client.publish(humid_topic, humidBuffer);
        
        // Publish RPM and Gas if present
        if (rIndex != -1) {
          client.publish(rpm_topic, rpmBuffer);
        }
        
        if (gIndex != -1) {
          client.publish(gas_topic, gasBuffer);
        }
        
        // Publish object count if present
        if (oIndex != -1) {
          client.publish(object_topic, objectBuffer);
        }
        
        Serial.printf("Published - Temp: %s, Humidity: %s, RPM: %s, Gas: %s, Objects: %s\n", 
                    tempBuffer, humidBuffer, rpmBuffer, gasBuffer, objectBuffer);
      } else {
        Serial.println("MQTT not connected, couldn't publish");
      }
    } else {
      Serial.println("Invalid temperature or humidity values");
    }
  } else {
    Serial.println("Failed to parse data format");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    setup_wifi();
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read from STM32
  while (Serial.available()) {
    char c = Serial.read();
    Serial.printf("Received char: %c (ASCII: %d)\n", c, c);  // Debug received characters
    
    lastDataReceived = millis();
    
    if (c == '\n') {
      serialBuffer[bufferIndex] = '\0';
      String dataString = String(serialBuffer);
      Serial.print("Complete line received: ");
      Serial.println(dataString);
      processData(dataString);
      bufferIndex = 0;
    } else if (bufferIndex < BUFFER_SIZE - 1) {
      serialBuffer[bufferIndex++] = c;
    }
  }

  if (millis() - lastDataReceived > DATA_TIMEOUT) {
    Serial.println("No data received for 10 seconds!");
    client.publish("esp8266/status", "no_data");
    lastDataReceived = millis();
  }

  delay(10);
}