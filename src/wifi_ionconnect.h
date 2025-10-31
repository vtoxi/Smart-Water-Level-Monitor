#ifndef WIFI_IONCONNECT_H
#define WIFI_IONCONNECT_H
#define ION_MINIMAL_MODE 1
#define ION_ENABLE_BLE 0
#define ION_ENABLE_OTA 0
#define ION_ENABLE_MDNS 0
#define ION_ENABLE_DIAGNOSTICS 0

#include <Arduino.h>
#include "config.h"
#include "config_manager.h"
#include <IonConnect.h>

/**
 * Wrapper class for IonConnect WiFi management
 * Integrates IonConnect with our ConfigManager
 */
class WiFiIonConnect {
public:
    WiFiIonConnect(ConfigManager& configManager);
    ~WiFiIonConnect();
    
    // Initialize WiFi with IonConnect
    bool begin();
    void loop();
    
    // WiFi status
    bool isConnected();
    String getIP() const;
    String getSSID() const;
    int8_t getRSSI() const;
    
    // AP mode status
    bool isAPMode();
    String getAPIP() const;
    
    // Configuration
    void setHostname(const char* hostname);
    void disconnect();
    void reconnect();
    
private:
    ConfigManager& configManager;
    IonConnectDevice* ion;  // Platform-specific IonConnect instance (pointer for ESP8266 memory)
    
    // Callbacks for IonConnect events
    void setupCallbacks();
};

#endif // WIFI_IONCONNECT_H

