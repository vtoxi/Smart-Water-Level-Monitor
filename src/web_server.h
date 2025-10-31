#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

// Board-specific async TCP library
#ifdef ESP8266
    #include <ESPAsyncTCP.h>
#else
    #include <AsyncTCP.h>
#endif

// Board-specific OTA update header
#ifdef ESP8266
    #include <Updater.h>
#else
    #include <Update.h>
#endif

#include <ArduinoJson.h>
#include "config_manager.h"
#include "sensor_ultrasonic.h"

// Forward declarations
class PumpController;

class WebServer {
public:
    WebServer(ConfigManager& configManager, uint16_t port = 80);
    
    // Initialize web server
    bool begin();
    void stop();
    
    // Register sensor and pump references
    void setSensor1(UltrasonicSensor* sensor) { sensor1 = sensor; }
    void setSensor2(UltrasonicSensor* sensor) { sensor2 = sensor; }
    void setPumpController(PumpController* pump) { pumpController = pump; }
    
    // Server status
    bool isRunning() const { return running; }
    
private:
    ConfigManager& configManager;
    AsyncWebServer server;
    UltrasonicSensor* sensor1;
    UltrasonicSensor* sensor2;
    PumpController* pumpController;
    bool running;
    
    // Route handlers
    void setupRoutes();
    void handleRoot(AsyncWebServerRequest* request);
    void handleStatus(AsyncWebServerRequest* request);
    void handleConfig(AsyncWebServerRequest* request);
    void handleConfigSave(AsyncWebServerRequest* request);
    void handleWiFiScan(AsyncWebServerRequest* request);
    void handleRestart(AsyncWebServerRequest* request);
    void handleReset(AsyncWebServerRequest* request);
    void handleOTAUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final);
    void handlePumpControl(AsyncWebServerRequest* request);
    
    // Helper functions
    String getStatusJSON();
    String getConfigJSON();
    bool validateConfig(JsonObject& config);
    void sendCORS(AsyncWebServerRequest* request);
};

#endif // WEB_SERVER_H

