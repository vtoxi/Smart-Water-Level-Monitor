#include "wifi_ionconnect.h"
#include "config.h"

// Board-specific WiFi includes
#ifdef ESP8266
    #include <ESP8266WiFi.h>
#else
    #include <WiFi.h>
#endif

WiFiIonConnect::WiFiIonConnect(ConfigManager& configManager)
    : configManager(configManager) {
}

void WiFiIonConnect::setupCallbacks() {
    // Register connection callback
    ion.onConnect([this]() {
        DEBUG_PRINTLN("\n✓ IonConnect: WiFi Connected!");
        DEBUG_PRINTF("  IP Address: %s\n", WiFi.localIP().toString().c_str());
        DEBUG_PRINTF("  Signal: %d dBm\n", WiFi.RSSI());
        
        // Save credentials if they changed
        String currentSSID = WiFi.SSID();
        const SystemConfig& config = configManager.getConfig();
        
        if (currentSSID.length() > 0 && currentSSID != config.wifiSSID) {
            DEBUG_PRINTLN("  Saving new WiFi credentials");
            configManager.setWiFiCredentials(currentSSID.c_str(), WiFi.psk().c_str());
            configManager.saveConfig();
        }
    });
    
    // Register disconnection callback
    ion.onDisconnect([this]() {
        DEBUG_PRINTLN("⚠ IonConnect: WiFi Disconnected");
    });
    
    // Register portal start callback
    ion.onPortalStart([this]() {
        DEBUG_PRINTLN("\n→ IonConnect: Captive Portal Started");
        DEBUG_PRINTLN("  Connect to: IonConnect-XXXX WiFi");
        DEBUG_PRINTLN("  Go to: http://192.168.4.1");
    });
    
    // Register config saved callback
    ion.onConfigSaved([this]() {
        DEBUG_PRINTLN("✓ IonConnect: Configuration saved");
    });
}

bool WiFiIonConnect::begin() {
    const SystemConfig& config = configManager.getConfig();
    
    DEBUG_PRINTLN("IonConnect: Initializing...");
    
    // Initialize IonConnect with device name
    if (!ion.init(config.deviceId)) {
        DEBUG_PRINTLN("✗ IonConnect: Initialization failed!");
        return false;
    }
    
    // Setup event callbacks
    setupCallbacks();
    
    // If we have stored credentials, add them
    if (strlen(config.wifiSSID) > 0) {
        DEBUG_PRINTF("IonConnect: Adding saved network: %s\n", config.wifiSSID);
        ion.addNetwork(config.wifiSSID, config.wifiPassword);
    }
    
    // Start IonConnect - will either connect or start portal
    if (!ion.begin()) {
        DEBUG_PRINTLN("✗ IonConnect: begin() failed!");
        return false;
    }
    
    DEBUG_PRINTLN("✓ IonConnect initialized successfully");
    return true;
}

void WiFiIonConnect::loop() {
    // IonConnect handles everything internally
    ion.handle();
}

bool WiFiIonConnect::isConnected() {
    return ion.isConnected();
}

String WiFiIonConnect::getIP() const {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "";
}

String WiFiIonConnect::getSSID() const {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.SSID();
    }
    return "";
}

int8_t WiFiIonConnect::getRSSI() const {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}

bool WiFiIonConnect::isAPMode() {
    return ion.isPortalActive();
}

String WiFiIonConnect::getAPIP() const {
    if (WiFi.getMode() & WIFI_AP) {
        return WiFi.softAPIP().toString();
    }
    return "";
}

void WiFiIonConnect::setHostname(const char* hostname) {
    // Hostname is set during init(), but we can call init() again to change it
    DEBUG_PRINTF("IonConnect: Hostname set to: %s\n", hostname);
}

void WiFiIonConnect::disconnect() {
    ion.disconnect();
}

void WiFiIonConnect::reconnect() {
    ion.connect();
}

