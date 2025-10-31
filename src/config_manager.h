#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>

// ESP32 uses Preferences, ESP8266 will use LittleFS with JSON
#ifndef ESP8266
    #include <Preferences.h>
#else
    // ESP8266 compatibility - will store in LittleFS
    #include <LittleFS.h>
    #include <ArduinoJson.h>
#endif

// Tank operation mode
enum TankMode {
    SINGLE_TANK = 0,
    DUAL_TANK = 1
};

// Unit system
enum UnitSystem {
    METRIC_CM = 0,      // Centimeters
    IMPERIAL_INCH = 1   // Inches
};

// Pump operation mode
enum PumpMode {
    PUMP_MANUAL = 0,
    PUMP_AUTOMATIC = 1,
    PUMP_SCHEDULED = 2
};

// Configuration structure
struct SystemConfig {
    // Tank configuration
    TankMode tankMode;
    UnitSystem unitSystem;
    float tank1EmptyCm;
    float tank1FullCm;
    float tank2EmptyCm;
    float tank2FullCm;
    char tank1Name[32];
    char tank2Name[32];
    
    // WiFi configuration
    char wifiSSID[64];
    char wifiPassword[64];
    
    // MQTT configuration
    char mqttBroker[128];
    uint16_t mqttPort;
    char mqttUser[64];
    char mqttPassword[64];
    char mqttTopic[128];
    char mqttCmdTopic[128];
    uint32_t mqttPublishInterval;
    
    // Sensor configuration
    uint8_t trigPin1;
    uint8_t echoPin1;
    uint8_t trigPin2;
    uint8_t echoPin2;
    uint32_t sensorReadInterval;
    
    // Pump configuration
    uint8_t pumpRelayPin;
    PumpMode pumpMode;
    float pumpAutoOnThreshold;
    float pumpAutoOffThreshold;
    uint32_t pumpMaxRunTime;
    uint32_t pumpCooldownTime;
    
    // Display configuration
    bool displayEnabled;
    uint32_t displayTimeout;
    
    // Device identification
    char deviceId[32];
    
    // First run flag
    bool isConfigured;
};

class ConfigManager {
public:
    ConfigManager();
    
    // Initialize configuration manager
    bool begin();
    
    // Load configuration from NVS
    bool loadConfig();
    
    // Save configuration to NVS
    bool saveConfig();
    
    // Reset to factory defaults
    void resetToDefaults();
    
    // Get current configuration
    const SystemConfig& getConfig() const { return config; }
    SystemConfig& getConfigRef() { return config; }
    
    // Configuration setters (with validation)
    bool setTankMode(TankMode mode);
    bool setUnitSystem(UnitSystem units);
    bool setTank1Calibration(float emptyCm, float fullCm);
    bool setTank2Calibration(float emptyCm, float fullCm);
    bool setTankName(uint8_t tankNum, const char* name);
    bool setWiFiCredentials(const char* ssid, const char* password);
    bool setMQTTConfig(const char* broker, uint16_t port, const char* user, const char* password);
    bool setMQTTTopics(const char* topic, const char* cmdTopic);
    bool setSensorPins(uint8_t tank, uint8_t trigPin, uint8_t echoPin);
    bool setPumpConfig(PumpMode mode, uint8_t relayPin, float onThreshold, float offThreshold);
    bool setDisplayConfig(bool enabled, uint32_t timeout);
    
    // Validation helpers
    bool isWiFiConfigured() const;
    bool isMQTTConfigured() const;
    bool validateCalibration(float emptyCm, float fullCm) const;
    
    // Debug helper
    void printConfig() const;
    
private:
    SystemConfig config;
    
    #ifndef ESP8266
        Preferences preferences;
    #else
        bool loadFromLittleFS();
        bool saveToLittleFS();
    #endif
    
    // Internal helpers
    void generateDeviceId();
    bool validateConfig() const;
};

#endif // CONFIG_MANAGER_H

