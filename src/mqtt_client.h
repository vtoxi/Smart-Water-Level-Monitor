#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config_manager.h"
#include "sensor_ultrasonic.h"

// MQTT callback function type
typedef void (*MQTTCallback)(char* topic, byte* payload, unsigned int length);

class MQTTClient {
public:
    MQTTClient(ConfigManager& configManager);
    
    // Initialize MQTT client
    bool begin();
    
    // Connection management
    bool connect();
    bool reconnect();
    void disconnect();
    bool isConnected();
    
    // Must be called regularly in loop
    void loop();
    
    // Publishing
    bool publish(const char* topic, const char* payload, bool retained = false);
    bool publishSensorData(const SensorReading& tank1, const SensorReading* tank2 = nullptr);
    bool publishStatus(bool wifi, bool mqtt, bool ble, bool pump);
    
    // Subscribing
    bool subscribe(const char* topic);
    bool unsubscribe(const char* topic);
    void setCallback(MQTTCallback callback);
    
    // Configuration
    void updateConfig();
    
    // Auto-reconnect
    void enableAutoReconnect(bool enable) { autoReconnect = enable; }
    void checkConnection();
    
    // Last message buffer (offline storage)
    bool hasBufferedMessage() const { return hasLastMessage; }
    bool publishBuffered();
    
private:
    ConfigManager& configManager;
    WiFiClient wifiClient;
    PubSubClient client;
    
    bool autoReconnect;
    uint32_t lastReconnectAttempt;
    uint32_t reconnectInterval;
    uint32_t lastPublish;
    uint8_t reconnectAttempts;
    
    // Message buffering
    bool hasLastMessage;
    char lastMessageTopic[128];
    char lastMessagePayload[512];
    
    // Internal helpers
    String createDevicePayload(const SensorReading& tank1, const SensorReading* tank2 = nullptr);
    bool validateConnection();
};

#endif // MQTT_CLIENT_H

