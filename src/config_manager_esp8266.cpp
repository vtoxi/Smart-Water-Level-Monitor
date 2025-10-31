// ESP8266-specific configuration storage using LittleFS + JSON
#ifdef ESP8266

#include "config_manager.h"
#include "config.h"

bool ConfigManager::loadFromLittleFS() {
    DEBUG_PRINTLN("Loading configuration from LittleFS...");
    
    File file = LittleFS.open("/config.json", "r");
    if (!file) {
        DEBUG_PRINTLN("Failed to open config file");
        return false;
    }
    
    StaticJsonDocument<1536> doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        DEBUG_PRINTLN("Failed to parse config JSON");
        return false;
    }
    
    // Load all settings from JSON - using correct field names from SystemConfig
    config.tankMode = (TankMode)doc["tankMode"].as<int>();
    config.unitSystem = (UnitSystem)doc["unitSystem"].as<int>();
    config.tank1EmptyCm = doc["t1Empty"].as<float>();
    config.tank1FullCm = doc["t1Full"].as<float>();
    config.tank2EmptyCm = doc["t2Empty"].as<float>();
    config.tank2FullCm = doc["t2Full"].as<float>();
    strlcpy(config.tank1Name, doc["t1Name"] | "Tank 1", sizeof(config.tank1Name));
    strlcpy(config.tank2Name, doc["t2Name"] | "Tank 2", sizeof(config.tank2Name));
    
    strlcpy(config.wifiSSID, doc["wifiSSID"] | "", sizeof(config.wifiSSID));
    strlcpy(config.wifiPassword, doc["wifiPass"] | "", sizeof(config.wifiPassword));
    
    strlcpy(config.mqttBroker, doc["mqttBroker"] | "", sizeof(config.mqttBroker));
    config.mqttPort = doc["mqttPort"].as<uint16_t>();
    strlcpy(config.mqttUser, doc["mqttUser"] | "", sizeof(config.mqttUser));
    strlcpy(config.mqttPassword, doc["mqttPass"] | "", sizeof(config.mqttPassword));
    strlcpy(config.mqttTopic, doc["mqttTopic"] | "", sizeof(config.mqttTopic));
    strlcpy(config.mqttCmdTopic, doc["mqttCmdTopic"] | "", sizeof(config.mqttCmdTopic));
    config.mqttPublishInterval = doc["mqttInterval"].as<uint32_t>();
    
    config.trigPin1 = doc["trigPin1"].as<uint8_t>();
    config.echoPin1 = doc["echoPin1"].as<uint8_t>();
    config.trigPin2 = doc["trigPin2"].as<uint8_t>();
    config.echoPin2 = doc["echoPin2"].as<uint8_t>();
    
    config.pumpMode = (PumpMode)doc["pumpMode"].as<int>();
    config.pumpAutoOnThreshold = doc["pumpOnThresh"].as<float>();
    config.pumpAutoOffThreshold = doc["pumpOffThresh"].as<float>();
    config.pumpMaxRunTime = doc["pumpMaxRun"].as<uint32_t>();
    config.pumpCooldownTime = doc["pumpCool"].as<uint32_t>();
    config.pumpRelayPin = doc["pumpPin"].as<uint8_t>();
    
    config.displayEnabled = doc["dispEnabled"].as<bool>();
    config.displayTimeout = doc["dispTimeout"].as<uint32_t>();
    
    strlcpy(config.deviceId, doc["deviceId"] | "", sizeof(config.deviceId));
    config.isConfigured = true;
    
    DEBUG_PRINTLN("Configuration loaded from LittleFS");
    return true;
}

bool ConfigManager::saveToLittleFS() {
    DEBUG_PRINTLN("Saving configuration to LittleFS...");
    
    StaticJsonDocument<1536> doc;
    
    // Save all settings to JSON - using correct field names from SystemConfig
    doc["tankMode"] = config.tankMode;
    doc["unitSystem"] = config.unitSystem;
    doc["t1Empty"] = config.tank1EmptyCm;
    doc["t1Full"] = config.tank1FullCm;
    doc["t2Empty"] = config.tank2EmptyCm;
    doc["t2Full"] = config.tank2FullCm;
    doc["t1Name"] = config.tank1Name;
    doc["t2Name"] = config.tank2Name;
    
    doc["wifiSSID"] = config.wifiSSID;
    doc["wifiPass"] = config.wifiPassword;
    
    doc["mqttBroker"] = config.mqttBroker;
    doc["mqttPort"] = config.mqttPort;
    doc["mqttUser"] = config.mqttUser;
    doc["mqttPass"] = config.mqttPassword;
    doc["mqttTopic"] = config.mqttTopic;
    doc["mqttCmdTopic"] = config.mqttCmdTopic;
    doc["mqttInterval"] = config.mqttPublishInterval;
    
    doc["trigPin1"] = config.trigPin1;
    doc["echoPin1"] = config.echoPin1;
    doc["trigPin2"] = config.trigPin2;
    doc["echoPin2"] = config.echoPin2;
    
    doc["pumpMode"] = config.pumpMode;
    doc["pumpOnThresh"] = config.pumpAutoOnThreshold;
    doc["pumpOffThresh"] = config.pumpAutoOffThreshold;
    doc["pumpMaxRun"] = config.pumpMaxRunTime;
    doc["pumpCool"] = config.pumpCooldownTime;
    doc["pumpPin"] = config.pumpRelayPin;
    
    doc["dispEnabled"] = config.displayEnabled;
    doc["dispTimeout"] = config.displayTimeout;
    
    doc["deviceId"] = config.deviceId;
    
    File file = LittleFS.open("/config.json", "w");
    if (!file) {
        DEBUG_PRINTLN("Failed to open config file for writing");
        return false;
    }
    
    if (serializeJson(doc, file) == 0) {
        DEBUG_PRINTLN("Failed to write config JSON");
        file.close();
        return false;
    }
    
    file.close();
    config.isConfigured = true;
    DEBUG_PRINTLN("Configuration saved to LittleFS");
    return true;
}

#endif // ESP8266

