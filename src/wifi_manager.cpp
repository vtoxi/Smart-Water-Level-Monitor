#include "wifi_manager.h"
#include "config.h"

WiFiManager::WiFiManager(ConfigManager& configManager)
    : configManager(configManager),
      state(WIFI_IDLE),
      apMode(false),
      autoReconnect(true),
      lastReconnectAttempt(0),
      reconnectInterval(WIFI_RECONNECT_INTERVAL),
      reconnectAttempts(0),
      dnsServer(nullptr) {
}

bool WiFiManager::begin() {
    WiFi.mode(WIFI_MODE_NULL);
    WiFi.persistent(false); // Don't save WiFi credentials to flash
    
    // Set hostname
    const SystemConfig& config = configManager.getConfig();
    WiFi.setHostname(config.deviceId);
    
    DEBUG_PRINTLN("WiFi Manager initialized");
    return true;
}

bool WiFiManager::connect(const char* ssid, const char* password, uint32_t timeout) {
    if (!ssid || strlen(ssid) == 0) {
        DEBUG_PRINTLN("WiFi: Invalid SSID");
        return false;
    }
    
    DEBUG_PRINTF("WiFi: Connecting to %s...\n", ssid);
    updateState(WIFI_CONNECTING);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    return waitForConnection(timeout);
}

bool WiFiManager::connect() {
    const SystemConfig& config = configManager.getConfig();
    
    if (!configManager.isWiFiConfigured()) {
        DEBUG_PRINTLN("WiFi: No credentials configured, starting AP mode");
        return startAP();
    }
    
    return connect(config.wifiSSID, config.wifiPassword);
}

bool WiFiManager::waitForConnection(uint32_t timeout) {
    uint32_t startTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
        delay(100);
        DEBUG_PRINT(".");
    }
    DEBUG_PRINTLN();
    
    if (WiFi.status() == WL_CONNECTED) {
        updateState(WIFI_CONNECTED);
        DEBUG_PRINTF("WiFi: Connected! IP: %s, RSSI: %d dBm\n", 
                     WiFi.localIP().toString().c_str(), WiFi.RSSI());
        reconnectAttempts = 0;
        return true;
    } else {
        updateState(WIFI_DISCONNECTED);
        DEBUG_PRINTLN("WiFi: Connection failed");
        return false;
    }
}

void WiFiManager::disconnect() {
    DEBUG_PRINTLN("WiFi: Disconnecting...");
    WiFi.disconnect(true);
    updateState(WIFI_DISCONNECTED);
}

bool WiFiManager::reconnect() {
    if (apMode) {
        DEBUG_PRINTLN("WiFi: Cannot reconnect in AP mode");
        return false;
    }
    
    DEBUG_PRINTF("WiFi: Reconnecting (attempt %d)...\n", reconnectAttempts + 1);
    reconnectAttempts++;
    lastReconnectAttempt = millis();
    
    // Exponential backoff for reconnect interval
    if (reconnectAttempts > 3) {
        reconnectInterval = min((uint32_t)(reconnectInterval * 2), (uint32_t)300000UL); // Max 5 minutes
    }
    
    return connect();
}

bool WiFiManager::startAP(const char* ssid, const char* password) {
    DEBUG_PRINTF("WiFi: Starting AP mode (SSID: %s)...\n", ssid);
    
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
    
    bool success = WiFi.softAP(ssid, password);
    
    if (success) {
        apMode = true;
        updateState(WIFI_AP_MODE);
        DEBUG_PRINTF("WiFi: AP started. IP: %s\n", WiFi.softAPIP().toString().c_str());
        
        // Start captive portal DNS
        startCaptivePortal();
        
        return true;
    } else {
        DEBUG_PRINTLN("WiFi: Failed to start AP");
        updateState(WIFI_ERROR);
        return false;
    }
}

void WiFiManager::stopAP() {
    if (apMode) {
        DEBUG_PRINTLN("WiFi: Stopping AP mode...");
        stopCaptivePortal();
        WiFi.softAPdisconnect(true);
        apMode = false;
        updateState(WIFI_IDLE);
    }
}

bool WiFiManager::startCaptivePortal() {
    if (!dnsServer) {
        dnsServer = new DNSServer();
    }
    
    // Redirect all DNS queries to our IP
    dnsServer->start(53, "*", WiFi.softAPIP());
    DEBUG_PRINTLN("Captive portal DNS started");
    
    return true;
}

void WiFiManager::stopCaptivePortal() {
    if (dnsServer) {
        dnsServer->stop();
        delete dnsServer;
        dnsServer = nullptr;
        DEBUG_PRINTLN("Captive portal DNS stopped");
    }
}

void WiFiManager::handleDNS() {
    if (dnsServer) {
        dnsServer->processNextRequest();
    }
}

void WiFiManager::checkConnection() {
    // Handle auto-reconnect
    if (autoReconnect && !apMode && !isConnected()) {
        if (millis() - lastReconnectAttempt >= reconnectInterval) {
            reconnect();
        }
    }
    
    // Update state if connection dropped
    if (state == WIFI_CONNECTED && WiFi.status() != WL_CONNECTED) {
        DEBUG_PRINTLN("WiFi: Connection lost");
        updateState(WIFI_DISCONNECTED);
    }
}

void WiFiManager::setHostname(const char* hostname) {
    WiFi.setHostname(hostname);
}

void WiFiManager::updateState(WiFiState newState) {
    if (state != newState) {
        state = newState;
        #if DEBUG_WIFI
        const char* stateNames[] = {"IDLE", "CONNECTING", "CONNECTED", "DISCONNECTED", "AP_MODE", "ERROR"};
        DEBUG_PRINTF("WiFi: State changed to %s\n", stateNames[newState]);
        #endif
    }
}

