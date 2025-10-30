#ifndef DISPLAY_OLED_H
#define DISPLAY_OLED_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "sensor_ultrasonic.h"

// Display screen modes
enum DisplayScreen {
    SCREEN_MAIN = 0,
    SCREEN_NETWORK_STATUS = 1,
    SCREEN_SENSOR_DETAILS = 2,
    SCREEN_PUMP_STATUS = 3
};

// Connection status
struct ConnectionStatus {
    bool wifiConnected;
    bool mqttConnected;
    bool bleConnected;
    int8_t wifiRSSI;
    String wifiSSID;
};

class DisplayOLED {
public:
    DisplayOLED();
    
    // Initialize display
    bool begin();
    
    // Display control
    void clear();
    void update();
    void setBrightness(uint8_t brightness);
    void displayOff();
    void displayOn();
    
    // Content display - Single Tank Mode
    void showSingleTankMain(const char* tankName, float level, float distance, 
                            const ConnectionStatus& status);
    
    // Content display - Dual Tank Mode
    void showDualTankMain(const char* tank1Name, float level1, 
                          const char* tank2Name, float level2,
                          const ConnectionStatus& status);
    
    // Status screens
    void showNetworkStatus(const ConnectionStatus& status, const char* ipAddress);
    void showSensorDetails(const SensorReading& reading1, const SensorReading* reading2 = nullptr);
    void showPumpStatus(bool pumpOn, uint32_t runTime, float targetLevel);
    void showConfigMode(const char* apSSID, const char* ipAddress);
    void showError(const char* errorMsg);
    void showBootScreen();
    
    // Screen rotation
    void nextScreen();
    void setScreen(DisplayScreen screen);
    DisplayScreen getCurrentScreen() const { return currentScreen; }
    
    // Auto-rotation
    void enableAutoRotate(bool enable, uint32_t interval = 5000);
    void checkAutoRotate();
    
private:
    Adafruit_SSD1306 display;
    DisplayScreen currentScreen;
    bool autoRotateEnabled;
    uint32_t lastScreenRotation;
    uint32_t screenRotationInterval;
    uint32_t lastUpdate;
    
    // Drawing helpers
    void drawProgressBar(int16_t x, int16_t y, int16_t width, int16_t height, float percent);
    void drawStatusIcons(const ConnectionStatus& status);
    void drawWiFiIcon(int16_t x, int16_t y, bool connected, int8_t rssi = 0);
    void drawMQTTIcon(int16_t x, int16_t y, bool connected);
    void drawBLEIcon(int16_t x, int16_t y, bool connected);
    void drawPumpIcon(int16_t x, int16_t y, bool active);
    void drawCenteredText(const char* text, int16_t y, uint8_t textSize = 1);
};

#endif // DISPLAY_OLED_H

