#include <WiFi.h>
#include <PubSubClient.h> // MQTT client library
#include <DHT.h>          // DHT sensor library
#include <ArduinoJson.h>  // JSON parsing/creation library
#include "config.h"       // Contains sensitive info like Wi-Fi credentials, MQTT broker details

// Define sensor pins
#define DHTPIN 4          // DHT11/DHT22 sensor data pin
#define DHTTYPE DHT11     // DHT11 or DHT22
#define LDR_PIN 34        // Analog pin for LDR
#define PIR_PIN 2         // Digital pin for PIR sensor

// Define relay pins for appliances
#define RELAY1_PIN 16     // Light 1
#define RELAY2_PIN 17     // Fan 1
// Add more as needed

DHT dht(DHTPIN, DHTTYPE);

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    messageTemp += (char)payload[i];
  }
  Serial.println();

  // Handle incoming MQTT messages (commands from cloud/app)
  if (String(topic) == MQTT_SUBSCRIBE_TOPIC_CONTROL) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, messageTemp);

    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    if (doc.containsKey("light1")) {
      int light1_state = doc["light1"];
      digitalWrite(RELAY1_PIN, light1_state == 1 ? HIGH : LOW); // Assuming active HIGH relay
      Serial.print("Light 1 set to: ");
      Serial.println(light1_state);
    }
    if (doc.containsKey("fan1")) {
      int fan1_state = doc["fan1"];
      digitalWrite(RELAY2_PIN, fan1_state == 1 ? HIGH : LOW);
      Serial.print("Fan 1 set to: ");
      Serial.println(fan1_state);
    }
    // Add logic for other appliances
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      Serial.println("connected");
      // Subscribe to control topic
      client.subscribe(MQTT_SUBSCRIBE_TOPIC_CONTROL);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  dht.begin();
  pinMode(LDR_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, LOW); // Ensure off initially
  digitalWrite(RELAY2_PIN, LOW);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) { // Publish sensor data every 5 seconds
    lastMsg = now;

    // Read sensors
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int light_val = analogRead(LDR_PIN);
    int motion_val = digitalRead(PIR_PIN);

    // Check if any reads failed and exit early (to avoid NaN)
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }

    // Create JSON payload
    StaticJsonDocument<200> doc;
    doc["temperature"] = t;
    doc["humidity"] = h;
    doc["light"] = map(light_val, 0, 4095, 0, 100); // Map LDR to percentage
    doc["motion"] = motion_val;

    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);

    Serial.print("Publishing sensor data: ");
    Serial.println(jsonBuffer);
    client.publish(MQTT_PUBLISH_TOPIC_SENSORS, jsonBuffer);
  }
}
