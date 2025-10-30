#include "config_manager.h"
#include "config.h"
#include <esp_system.h>

ConfigManager::ConfigManager() {
    // Constructor
}

bool ConfigManager::begin() {
    if (!preferences.begin("waterlevel", false)) {
        DEBUG_PRINTLN("Failed to initialize NVS");
        return false;
    }
    
    // Check if this is first run
    bool isConfigured = preferences.getBool("configured", false);
    
    if (!isConfigured) {
        DEBUG_PRINTLN("First run detected, setting defaults");
        resetToDefaults();
        saveConfig();
    } else {
        loadConfig();
    }
    
    return true;
}

bool ConfigManager::loadConfig() {
    DEBUG_PRINTLN("Loading configuration from NVS...");
    
    // Tank configuration
    config.tankMode = (TankMode)preferences.getUChar("tankMode", SINGLE_TANK);
    config.unitSystem = (UnitSystem)preferences.getUChar("unitSystem", METRIC_CM);
    config.tank1EmptyCm = preferences.getFloat("t1Empty", DEFAULT_TANK1_EMPTY_CM);
    config.tank1FullCm = preferences.getFloat("t1Full", DEFAULT_TANK1_FULL_CM);
    config.tank2EmptyCm = preferences.getFloat("t2Empty", DEFAULT_TANK2_EMPTY_CM);
    config.tank2FullCm = preferences.getFloat("t2Full", DEFAULT_TANK2_FULL_CM);
    preferences.getString("t1Name", config.tank1Name, sizeof(config.tank1Name));
    preferences.getString("t2Name", config.tank2Name, sizeof(config.tank2Name));
    
    // WiFi configuration
    preferences.getString("wifiSSID", config.wifiSSID, sizeof(config.wifiSSID));
    preferences.getString("wifiPass", config.wifiPassword, sizeof(config.wifiPassword));
    
    // MQTT configuration
    preferences.getString("mqttBroker", config.mqttBroker, sizeof(config.mqttBroker));
    config.mqttPort = preferences.getUShort("mqttPort", DEFAULT_MQTT_PORT);
    preferences.getString("mqttUser", config.mqttUser, sizeof(config.mqttUser));
    preferences.getString("mqttPass", config.mqttPassword, sizeof(config.mqttPassword));
    preferences.getString("mqttTopic", config.mqttTopic, sizeof(config.mqttTopic));
    preferences.getString("mqttCmd", config.mqttCmdTopic, sizeof(config.mqttCmdTopic));
    config.mqttPublishInterval = preferences.getUInt("mqttInterval", MQTT_PUBLISH_INTERVAL);
    
    // Sensor configuration
    config.trigPin1 = preferences.getUChar("trigPin1", DEFAULT_TRIG_PIN_1);
    config.echoPin1 = preferences.getUChar("echoPin1", DEFAULT_ECHO_PIN_1);
    config.trigPin2 = preferences.getUChar("trigPin2", DEFAULT_TRIG_PIN_2);
    config.echoPin2 = preferences.getUChar("echoPin2", DEFAULT_ECHO_PIN_2);
    config.sensorReadInterval = preferences.getUInt("sensorInt", SENSOR_READ_INTERVAL);
    
    // Pump configuration
    config.pumpRelayPin = preferences.getUChar("pumpPin", DEFAULT_PUMP_RELAY_PIN);
    config.pumpMode = (PumpMode)preferences.getUChar("pumpMode", PUMP_MANUAL);
    config.pumpAutoOnThreshold = preferences.getFloat("pumpOnThr", PUMP_AUTO_ON_THRESHOLD);
    config.pumpAutoOffThreshold = preferences.getFloat("pumpOffThr", PUMP_AUTO_OFF_THRESHOLD);
    config.pumpMaxRunTime = preferences.getUInt("pumpMaxTime", PUMP_MAX_RUN_TIME);
    config.pumpCooldownTime = preferences.getUInt("pumpCool", PUMP_COOLDOWN_TIME);
    
    // Display configuration
    config.displayEnabled = preferences.getBool("dispEnabled", true);
    config.displayTimeout = preferences.getUInt("dispTimeout", DISPLAY_TIMEOUT);
    
    // Device ID
    preferences.getString("deviceId", config.deviceId, sizeof(config.deviceId));
    if (strlen(config.deviceId) == 0) {
        generateDeviceId();
    }
    
    config.isConfigured = preferences.getBool("configured", false);
    
    DEBUG_PRINTLN("Configuration loaded successfully");
    return true;
}

bool ConfigManager::saveConfig() {
    DEBUG_PRINTLN("Saving configuration to NVS...");
    
    // Tank configuration
    preferences.putUChar("tankMode", config.tankMode);
    preferences.putUChar("unitSystem", config.unitSystem);
    preferences.putFloat("t1Empty", config.tank1EmptyCm);
    preferences.putFloat("t1Full", config.tank1FullCm);
    preferences.putFloat("t2Empty", config.tank2EmptyCm);
    preferences.putFloat("t2Full", config.tank2FullCm);
    preferences.putString("t1Name", config.tank1Name);
    preferences.putString("t2Name", config.tank2Name);
    
    // WiFi configuration
    preferences.putString("wifiSSID", config.wifiSSID);
    preferences.putString("wifiPass", config.wifiPassword);
    
    // MQTT configuration
    preferences.putString("mqttBroker", config.mqttBroker);
    preferences.putUShort("mqttPort", config.mqttPort);
    preferences.putString("mqttUser", config.mqttUser);
    preferences.putString("mqttPass", config.mqttPassword);
    preferences.putString("mqttTopic", config.mqttTopic);
    preferences.putString("mqttCmd", config.mqttCmdTopic);
    preferences.putUInt("mqttInterval", config.mqttPublishInterval);
    
    // Sensor configuration
    preferences.putUChar("trigPin1", config.trigPin1);
    preferences.putUChar("echoPin1", config.echoPin1);
    preferences.putUChar("trigPin2", config.trigPin2);
    preferences.putUChar("echoPin2", config.echoPin2);
    preferences.putUInt("sensorInt", config.sensorReadInterval);
    
    // Pump configuration
    preferences.putUChar("pumpPin", config.pumpRelayPin);
    preferences.putUChar("pumpMode", config.pumpMode);
    preferences.putFloat("pumpOnThr", config.pumpAutoOnThreshold);
    preferences.putFloat("pumpOffThr", config.pumpAutoOffThreshold);
    preferences.putUInt("pumpMaxTime", config.pumpMaxRunTime);
    preferences.putUInt("pumpCool", config.pumpCooldownTime);
    
    // Display configuration
    preferences.putBool("dispEnabled", config.displayEnabled);
    preferences.putUInt("dispTimeout", config.displayTimeout);
    
    // Device ID
    preferences.putString("deviceId", config.deviceId);
    
    // Mark as configured
    preferences.putBool("configured", true);
    config.isConfigured = true;
    
    DEBUG_PRINTLN("Configuration saved successfully");
    return true;
}

void ConfigManager::resetToDefaults() {
    DEBUG_PRINTLN("Resetting to factory defaults...");
    
    // Tank defaults
    config.tankMode = SINGLE_TANK;
    config.unitSystem = METRIC_CM;
    config.tank1EmptyCm = DEFAULT_TANK1_EMPTY_CM;
    config.tank1FullCm = DEFAULT_TANK1_FULL_CM;
    config.tank2EmptyCm = DEFAULT_TANK2_EMPTY_CM;
    config.tank2FullCm = DEFAULT_TANK2_FULL_CM;
    strcpy(config.tank1Name, "Tank 1");
    strcpy(config.tank2Name, "Tank 2");
    
    // WiFi defaults (empty - will trigger AP mode)
    memset(config.wifiSSID, 0, sizeof(config.wifiSSID));
    memset(config.wifiPassword, 0, sizeof(config.wifiPassword));
    
    // MQTT defaults
    strcpy(config.mqttBroker, DEFAULT_MQTT_BROKER);
    config.mqttPort = DEFAULT_MQTT_PORT;
    memset(config.mqttUser, 0, sizeof(config.mqttUser));
    memset(config.mqttPassword, 0, sizeof(config.mqttPassword));
    strcpy(config.mqttTopic, DEFAULT_MQTT_TOPIC);
    strcpy(config.mqttCmdTopic, DEFAULT_MQTT_CMD_TOPIC);
    config.mqttPublishInterval = MQTT_PUBLISH_INTERVAL;
    
    // Sensor defaults
    config.trigPin1 = DEFAULT_TRIG_PIN_1;
    config.echoPin1 = DEFAULT_ECHO_PIN_1;
    config.trigPin2 = DEFAULT_TRIG_PIN_2;
    config.echoPin2 = DEFAULT_ECHO_PIN_2;
    config.sensorReadInterval = SENSOR_READ_INTERVAL;
    
    // Pump defaults
    config.pumpRelayPin = DEFAULT_PUMP_RELAY_PIN;
    config.pumpMode = PUMP_MANUAL;
    config.pumpAutoOnThreshold = PUMP_AUTO_ON_THRESHOLD;
    config.pumpAutoOffThreshold = PUMP_AUTO_OFF_THRESHOLD;
    config.pumpMaxRunTime = PUMP_MAX_RUN_TIME;
    config.pumpCooldownTime = PUMP_COOLDOWN_TIME;
    
    // Display defaults
    config.displayEnabled = true;
    config.displayTimeout = DISPLAY_TIMEOUT;
    
    // Generate device ID
    generateDeviceId();
    
    config.isConfigured = false;
}

void ConfigManager::generateDeviceId() {
    uint64_t chipid = ESP.getEfuseMac();
    snprintf(config.deviceId, sizeof(config.deviceId), "%s_%04X%08X", 
             DEVICE_NAME, (uint16_t)(chipid >> 32), (uint32_t)chipid);
    DEBUG_PRINTF("Generated Device ID: %s\n", config.deviceId);
}

bool ConfigManager::setTankMode(TankMode mode) {
    if (mode != SINGLE_TANK && mode != DUAL_TANK) return false;
    config.tankMode = mode;
    return true;
}

bool ConfigManager::setUnitSystem(UnitSystem units) {
    if (units != METRIC_CM && units != IMPERIAL_INCH) return false;
    config.unitSystem = units;
    return true;
}

bool ConfigManager::setTank1Calibration(float emptyCm, float fullCm) {
    if (!validateCalibration(emptyCm, fullCm)) return false;
    config.tank1EmptyCm = emptyCm;
    config.tank1FullCm = fullCm;
    return true;
}

bool ConfigManager::setTank2Calibration(float emptyCm, float fullCm) {
    if (!validateCalibration(emptyCm, fullCm)) return false;
    config.tank2EmptyCm = emptyCm;
    config.tank2FullCm = fullCm;
    return true;
}

bool ConfigManager::setTankName(uint8_t tankNum, const char* name) {
    if (tankNum == 1) {
        strncpy(config.tank1Name, name, sizeof(config.tank1Name) - 1);
        config.tank1Name[sizeof(config.tank1Name) - 1] = '\0';
        return true;
    } else if (tankNum == 2) {
        strncpy(config.tank2Name, name, sizeof(config.tank2Name) - 1);
        config.tank2Name[sizeof(config.tank2Name) - 1] = '\0';
        return true;
    }
    return false;
}

bool ConfigManager::setWiFiCredentials(const char* ssid, const char* password) {
    if (strlen(ssid) == 0 || strlen(ssid) >= sizeof(config.wifiSSID)) return false;
    if (strlen(password) < 8 || strlen(password) >= sizeof(config.wifiPassword)) return false;
    
    strcpy(config.wifiSSID, ssid);
    strcpy(config.wifiPassword, password);
    return true;
}

bool ConfigManager::setMQTTConfig(const char* broker, uint16_t port, const char* user, const char* password) {
    if (strlen(broker) == 0 || strlen(broker) >= sizeof(config.mqttBroker)) return false;
    if (port == 0) return false;
    
    strcpy(config.mqttBroker, broker);
    config.mqttPort = port;
    strncpy(config.mqttUser, user, sizeof(config.mqttUser) - 1);
    strncpy(config.mqttPassword, password, sizeof(config.mqttPassword) - 1);
    return true;
}

bool ConfigManager::setMQTTTopics(const char* topic, const char* cmdTopic) {
    if (strlen(topic) == 0 || strlen(topic) >= sizeof(config.mqttTopic)) return false;
    
    strcpy(config.mqttTopic, topic);
    if (cmdTopic && strlen(cmdTopic) > 0) {
        strcpy(config.mqttCmdTopic, cmdTopic);
    }
    return true;
}

bool ConfigManager::setSensorPins(uint8_t tank, uint8_t trigPin, uint8_t echoPin) {
    if (trigPin > 39 || echoPin > 39) return false; // Valid GPIO range for ESP32
    
    if (tank == 1) {
        config.trigPin1 = trigPin;
        config.echoPin1 = echoPin;
        return true;
    } else if (tank == 2) {
        config.trigPin2 = trigPin;
        config.echoPin2 = echoPin;
        return true;
    }
    return false;
}

bool ConfigManager::setPumpConfig(PumpMode mode, uint8_t relayPin, float onThreshold, float offThreshold) {
    if (relayPin > 39) return false;
    if (onThreshold < 0 || onThreshold > 100) return false;
    if (offThreshold < 0 || offThreshold > 100) return false;
    if (onThreshold >= offThreshold) return false;
    
    config.pumpMode = mode;
    config.pumpRelayPin = relayPin;
    config.pumpAutoOnThreshold = onThreshold;
    config.pumpAutoOffThreshold = offThreshold;
    return true;
}

bool ConfigManager::setDisplayConfig(bool enabled, uint32_t timeout) {
    config.displayEnabled = enabled;
    config.displayTimeout = timeout;
    return true;
}

bool ConfigManager::isWiFiConfigured() const {
    return (strlen(config.wifiSSID) > 0 && strlen(config.wifiPassword) >= 8);
}

bool ConfigManager::isMQTTConfigured() const {
    return (strlen(config.mqttBroker) > 0 && config.mqttPort > 0);
}

bool ConfigManager::validateCalibration(float emptyCm, float fullCm) const {
    if (emptyCm <= 0 || fullCm <= 0) return false;
    if (emptyCm <= fullCm) return false; // Empty distance should be greater than full
    if (emptyCm > 500 || fullCm < 5) return false; // Reasonable limits
    return true;
}

bool ConfigManager::validateConfig() const {
    if (!validateCalibration(config.tank1EmptyCm, config.tank1FullCm)) return false;
    if (config.tankMode == DUAL_TANK) {
        if (!validateCalibration(config.tank2EmptyCm, config.tank2FullCm)) return false;
    }
    return true;
}

void ConfigManager::printConfig() const {
    DEBUG_PRINTLN("\n========== Current Configuration ==========");
    DEBUG_PRINTF("Device ID: %s\n", config.deviceId);
    DEBUG_PRINTF("Tank Mode: %s\n", config.tankMode == SINGLE_TANK ? "Single" : "Dual");
    DEBUG_PRINTF("Unit System: %s\n", config.unitSystem == METRIC_CM ? "Metric (cm)" : "Imperial (in)");
    DEBUG_PRINTF("Tank 1: %s (Empty: %.1f cm, Full: %.1f cm)\n", 
                 config.tank1Name, config.tank1EmptyCm, config.tank1FullCm);
    if (config.tankMode == DUAL_TANK) {
        DEBUG_PRINTF("Tank 2: %s (Empty: %.1f cm, Full: %.1f cm)\n", 
                     config.tank2Name, config.tank2EmptyCm, config.tank2FullCm);
    }
    DEBUG_PRINTF("WiFi: %s%s\n", config.wifiSSID, 
                 strlen(config.wifiSSID) > 0 ? " (configured)" : "(not configured)");
    DEBUG_PRINTF("MQTT: %s:%d\n", config.mqttBroker, config.mqttPort);
    DEBUG_PRINTF("Pump Mode: %s\n", 
                 config.pumpMode == PUMP_MANUAL ? "Manual" : 
                 config.pumpMode == PUMP_AUTOMATIC ? "Automatic" : "Scheduled");
    DEBUG_PRINTLN("==========================================\n");
}

