#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "config.h"
#include "config_manager.h"

// WiFi connection state
enum WiFiState {
    WIFI_IDLE = 0,
    WIFI_CONNECTING = 1,
    WIFI_CONNECTED = 2,
    WIFI_DISCONNECTED = 3,
    WIFI_AP_MODE = 4,
    WIFI_ERROR = 5
};

class WiFiManager {
public:
    WiFiManager(ConfigManager& configManager);
    
    // Initialize WiFi manager
    bool begin();
    
    // WiFi connection management
    bool connect(const char* ssid, const char* password, uint32_t timeout = WIFI_CONNECT_TIMEOUT);
    bool connect(); // Use stored credentials
    void disconnect();
    bool reconnect();
    
    // Access Point mode
    bool startAP(const char* ssid = AP_SSID, const char* password = AP_PASSWORD);
    void stopAP();
    bool isAPMode() const { return apMode; }
    
    // Captive portal
    bool startCaptivePortal();
    void stopCaptivePortal();
    void handleDNS();
    
    // Status
    WiFiState getState() const { return state; }
    bool isConnected() const { return (state == WIFI_CONNECTED && WiFi.status() == WL_CONNECTED); }
    int8_t getRSSI() const { return WiFi.RSSI(); }
    String getIP() const { return WiFi.localIP().toString(); }
    String getSSID() const { return WiFi.SSID(); }
    String getAPIP() const { return WiFi.softAPIP().toString(); }
    
    // Auto-reconnect management
    void enableAutoReconnect(bool enable) { autoReconnect = enable; }
    void checkConnection(); // Call in loop to maintain connection
    
    // Configuration
    void setHostname(const char* hostname);
    
private:
    ConfigManager& configManager;
    WiFiState state;
    bool apMode;
    bool autoReconnect;
    uint32_t lastReconnectAttempt;
    uint32_t reconnectInterval;
    uint8_t reconnectAttempts;
    
    DNSServer* dnsServer;
    
    // Internal helpers
    void updateState(WiFiState newState);
    bool waitForConnection(uint32_t timeout);
};

#endif // WIFI_MANAGER_H

