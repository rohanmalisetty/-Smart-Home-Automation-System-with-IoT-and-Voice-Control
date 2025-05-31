#ifndef CONFIG_H
#define CONFIG_H

// Wi-Fi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT Broker details (e.g., HiveMQ, Mosquitto, CloudMQTT)
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
const char* MQTT_USERNAME = ""; // Leave blank if no authentication
const char* MQTT_PASSWORD = ""; // Leave blank if no authentication

// MQTT Topics
const char* MQTT_PUBLISH_TOPIC_SENSORS = "home/sensor_data";
const char* MQTT_SUBSCRIBE_TOPIC_CONTROL = "home/control";

#endif
