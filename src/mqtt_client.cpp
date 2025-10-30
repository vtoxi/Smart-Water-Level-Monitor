#include "mqtt_client.h"
#include "config.h"

MQTTClient::MQTTClient(ConfigManager& configManager)
    : configManager(configManager),
      client(wifiClient),
      autoReconnect(true),
      lastReconnectAttempt(0),
      reconnectInterval(MQTT_RECONNECT_INTERVAL),
      lastPublish(0),
      reconnectAttempts(0),
      hasLastMessage(false) {
}

bool MQTTClient::begin() {
    const SystemConfig& config = configManager.getConfig();
    
    if (!configManager.isMQTTConfigured()) {
        DEBUG_PRINTLN("MQTT: Not configured");
        return false;
    }
    
    client.setServer(config.mqttBroker, config.mqttPort);
    client.setBufferSize(512); // Increase buffer for JSON payloads
    
    DEBUG_PRINTF("MQTT: Configured for %s:%d\n", config.mqttBroker, config.mqttPort);
    return true;
}

bool MQTTClient::connect() {
    if (!configManager.isMQTTConfigured()) {
        return false;
    }
    
    if (WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("MQTT: WiFi not connected");
        return false;
    }
    
    const SystemConfig& config = configManager.getConfig();
    
    DEBUG_PRINTF("MQTT: Connecting to %s:%d...\n", config.mqttBroker, config.mqttPort);
    
    bool connected = false;
    if (strlen(config.mqttUser) > 0) {
        connected = client.connect(config.deviceId, config.mqttUser, config.mqttPassword);
    } else {
        connected = client.connect(config.deviceId);
    }
    
    if (connected) {
        DEBUG_PRINTLN("MQTT: Connected");
        reconnectAttempts = 0;
        
        // Subscribe to command topic
        if (strlen(config.mqttCmdTopic) > 0) {
            subscribe(config.mqttCmdTopic);
        }
        
        // Publish buffered message if available
        if (hasLastMessage) {
            publishBuffered();
        }
        
        return true;
    } else {
        DEBUG_PRINTF("MQTT: Connection failed, state=%d\n", client.state());
        return false;
    }
}

bool MQTTClient::reconnect() {
    DEBUG_PRINTF("MQTT: Reconnecting (attempt %d)...\n", reconnectAttempts + 1);
    reconnectAttempts++;
    lastReconnectAttempt = millis();
    
    // Exponential backoff
    if (reconnectAttempts > 3) {
        reconnectInterval = min((uint32_t)(reconnectInterval * 2), (uint32_t)60000UL); // Max 1 minute
    }
    
    return connect();
}

void MQTTClient::disconnect() {
    if (client.connected()) {
        client.disconnect();
        DEBUG_PRINTLN("MQTT: Disconnected");
    }
}

void MQTTClient::loop() {
    if (client.connected()) {
        client.loop();
    }
}

bool MQTTClient::publish(const char* topic, const char* payload, bool retained) {
    if (!client.connected()) {
        DEBUG_PRINTLN("MQTT: Not connected, buffering message");
        
        // Buffer the message
        strncpy(lastMessageTopic, topic, sizeof(lastMessageTopic) - 1);
        strncpy(lastMessagePayload, payload, sizeof(lastMessagePayload) - 1);
        hasLastMessage = true;
        
        return false;
    }
    
    bool success = client.publish(topic, payload, retained);
    
    if (success) {
        lastPublish = millis();
        #if DEBUG_MQTT
        DEBUG_PRINTF("MQTT: Published to %s: %s\n", topic, payload);
        #endif
    } else {
        DEBUG_PRINTF("MQTT: Failed to publish to %s\n", topic);
    }
    
    return success;
}

bool MQTTClient::publishSensorData(const SensorReading& tank1, const SensorReading* tank2) {
    const SystemConfig& config = configManager.getConfig();
    
    if (!client.connected() || !tank1.isValid) {
        return false;
    }
    
    // Check if it's time to publish
    if (millis() - lastPublish < config.mqttPublishInterval) {
        return true; // Not an error, just too soon
    }
    
    String payload = createDevicePayload(tank1, tank2);
    return publish(config.mqttTopic, payload.c_str());
}

bool MQTTClient::publishStatus(bool wifi, bool mqtt, bool ble, bool pump) {
    const SystemConfig& config = configManager.getConfig();
    
    DynamicJsonDocument doc(256);
    doc["device_id"] = config.deviceId;
    doc["wifi"] = wifi;
    doc["mqtt"] = mqtt;
    doc["ble"] = ble;
    doc["pump"] = pump;
    doc["timestamp"] = millis() / 1000;
    
    String payload;
    serializeJson(doc, payload);
    
    char topic[150];
    snprintf(topic, sizeof(topic), "%s/status", config.mqttTopic);
    
    return publish(topic, payload.c_str());
}

bool MQTTClient::subscribe(const char* topic) {
    if (!client.connected()) {
        return false;
    }
    
    bool success = client.subscribe(topic);
    if (success) {
        DEBUG_PRINTF("MQTT: Subscribed to %s\n", topic);
    } else {
        DEBUG_PRINTF("MQTT: Failed to subscribe to %s\n", topic);
    }
    
    return success;
}

bool MQTTClient::unsubscribe(const char* topic) {
    if (!client.connected()) {
        return false;
    }
    
    return client.unsubscribe(topic);
}

void MQTTClient::setCallback(MQTTCallback callback) {
    client.setCallback(callback);
}

void MQTTClient::updateConfig() {
    const SystemConfig& config = configManager.getConfig();
    client.setServer(config.mqttBroker, config.mqttPort);
    DEBUG_PRINTLN("MQTT: Configuration updated");
}

void MQTTClient::checkConnection() {
    if (autoReconnect && !client.connected()) {
        if (millis() - lastReconnectAttempt >= reconnectInterval) {
            reconnect();
        }
    }
}

bool MQTTClient::publishBuffered() {
    if (!hasLastMessage || !client.connected()) {
        return false;
    }
    
    DEBUG_PRINTLN("MQTT: Publishing buffered message");
    bool success = client.publish(lastMessageTopic, lastMessagePayload);
    
    if (success) {
        hasLastMessage = false;
    }
    
    return success;
}

String MQTTClient::createDevicePayload(const SensorReading& tank1, const SensorReading* tank2) {
    const SystemConfig& config = configManager.getConfig();
    
    DynamicJsonDocument doc(512);
    
    doc["device_id"] = config.deviceId;
    doc["timestamp"] = millis() / 1000;
    doc["tank_mode"] = config.tankMode == SINGLE_TANK ? "single" : "dual";
    
    // Tank 1 data
    JsonObject t1 = doc.createNestedObject("tank1");
    t1["name"] = config.tank1Name;
    t1["level_percent"] = round(tank1.levelPercent * 10) / 10.0;
    t1["distance_cm"] = round(tank1.distanceCm * 10) / 10.0;
    t1["valid"] = tank1.isValid;
    
    // Tank 2 data (if dual mode)
    if (config.tankMode == DUAL_TANK && tank2 && tank2->isValid) {
        JsonObject t2 = doc.createNestedObject("tank2");
        t2["name"] = config.tank2Name;
        t2["level_percent"] = round(tank2->levelPercent * 10) / 10.0;
        t2["distance_cm"] = round(tank2->distanceCm * 10) / 10.0;
        t2["valid"] = tank2->isValid;
    }
    
    String output;
    serializeJson(doc, output);
    
    return output;
}

bool MQTTClient::isConnected() {
    return client.connected();
}

bool MQTTClient::validateConnection() {
    return (WiFi.status() == WL_CONNECTED && client.connected());
}

