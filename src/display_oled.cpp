#include "display_oled.h"
#include "config.h"

DisplayOLED::DisplayOLED() 
    : display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET_PIN),
      currentScreen(SCREEN_MAIN),
      autoRotateEnabled(false),
      lastScreenRotation(0),
      screenRotationInterval(5000),
      lastUpdate(0) {
}

bool DisplayOLED::begin() {
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        DEBUG_PRINTLN("SSD1306 allocation failed");
        return false;
    }
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
    
    DEBUG_PRINTLN("OLED display initialized");
    return true;
}

void DisplayOLED::clear() {
    display.clearDisplay();
}

void DisplayOLED::update() {
    display.display();
    lastUpdate = millis();
}

void DisplayOLED::setBrightness(uint8_t brightness) {
    display.ssd1306_command(SSD1306_SETCONTRAST);
    display.ssd1306_command(brightness);
}

void DisplayOLED::displayOff() {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
}

void DisplayOLED::displayOn() {
    display.ssd1306_command(SSD1306_DISPLAYON);
}

void DisplayOLED::showSingleTankMain(const char* tankName, float level, float distance, 
                                     const ConnectionStatus& status) {
    clear();
    
    // Tank name at top
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(tankName);
    
    // Status icons in top right
    drawStatusIcons(status);
    
    // Large level percentage
    display.setTextSize(3);
    display.setCursor(20, 20);
    display.printf("%.0f%%", level);
    
    // Progress bar
    drawProgressBar(0, 48, 128, 10, level);
    
    // Distance at bottom
    display.setTextSize(1);
    display.setCursor(30, 56);
    display.printf("%.1f cm", distance);
    
    update();
}

void DisplayOLED::showDualTankMain(const char* tank1Name, float level1, 
                                   const char* tank2Name, float level2,
                                   const ConnectionStatus& status) {
    clear();
    
    // Status icons at top
    drawStatusIcons(status);
    
    // Split screen for two tanks
    // Tank 1 (left half)
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(tank1Name);
    
    display.setTextSize(2);
    display.setCursor(5, 18);
    display.printf("%.0f%%", level1);
    
    drawProgressBar(0, 38, 60, 8, level1);
    
    // Tank 2 (right half)
    display.setTextSize(1);
    display.setCursor(68, 0);
    display.print(tank2Name);
    
    display.setTextSize(2);
    display.setCursor(73, 18);
    display.printf("%.0f%%", level2);
    
    drawProgressBar(68, 38, 60, 8, level2);
    
    // Divider line
    display.drawLine(64, 10, 64, 45, SSD1306_WHITE);
    
    update();
}

void DisplayOLED::showNetworkStatus(const ConnectionStatus& status, const char* ipAddress) {
    clear();
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Network Status");
    display.println();
    
    // WiFi status
    display.print("WiFi: ");
    if (status.wifiConnected) {
        display.println("Connected");
        display.print("  SSID: ");
        display.println(status.wifiSSID.c_str());
        display.print("  RSSI: ");
        display.print(status.wifiRSSI);
        display.println(" dBm");
        display.print("  IP: ");
        display.println(ipAddress);
    } else {
        display.println("Disconnected");
    }
    
    display.println();
    
    // MQTT status
    display.print("MQTT: ");
    display.println(status.mqttConnected ? "Connected" : "Disconnected");
    
    // BLE status
    display.print("BLE:  ");
    display.println(status.bleConnected ? "Active" : "Inactive");
    
    update();
}

void DisplayOLED::showSensorDetails(const SensorReading& reading1, const SensorReading* reading2) {
    clear();
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Sensor Details");
    display.println();
    
    // Tank 1
    display.println("Tank 1:");
    display.print("  Dist: ");
    display.print(reading1.distanceCm, 1);
    display.println(" cm");
    display.print("  Level: ");
    display.print(reading1.levelPercent, 1);
    display.println("%");
    display.print("  Valid: ");
    display.println(reading1.isValid ? "Yes" : "No");
    
    if (reading2) {
        display.println();
        display.println("Tank 2:");
        display.print("  Dist: ");
        display.print(reading2->distanceCm, 1);
        display.println(" cm");
        display.print("  Level: ");
        display.print(reading2->levelPercent, 1);
        display.println("%");
        display.print("  Valid: ");
        display.println(reading2->isValid ? "Yes" : "No");
    }
    
    update();
}

void DisplayOLED::showPumpStatus(bool pumpOn, uint32_t runTime, float targetLevel) {
    clear();
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Pump Status");
    display.println();
    
    display.setTextSize(2);
    display.setCursor(20, 20);
    display.println(pumpOn ? "ON" : "OFF");
    
    display.setTextSize(1);
    display.setCursor(0, 45);
    
    if (pumpOn) {
        display.print("Running: ");
        display.print(runTime / 1000);
        display.println(" s");
    } else {
        display.print("Target: ");
        display.print(targetLevel, 0);
        display.println("%");
    }
    
    update();
}

void DisplayOLED::showConfigMode(const char* apSSID, const char* ipAddress) {
    clear();
    
    display.setTextSize(1);
    drawCenteredText("CONFIG MODE", 0, 2);
    
    display.setCursor(0, 20);
    display.println("Connect to WiFi:");
    display.setTextSize(1);
    display.setCursor(0, 32);
    display.println(apSSID);
    
    display.setCursor(0, 44);
    display.println("Then open:");
    display.setCursor(0, 54);
    display.println(ipAddress);
    
    update();
}

void DisplayOLED::showError(const char* errorMsg) {
    clear();
    
    display.setTextSize(1);
    drawCenteredText("ERROR", 0, 2);
    
    display.setCursor(0, 24);
    display.println(errorMsg);
    
    update();
}

void DisplayOLED::showBootScreen() {
    clear();
    
    display.setTextSize(2);
    drawCenteredText("Water", 10, 2);
    drawCenteredText("Monitor", 30, 2);
    
    display.setTextSize(1);
    drawCenteredText(FIRMWARE_VERSION, 50, 1);
    
    update();
}

void DisplayOLED::nextScreen() {
    int nextScreen = (int)currentScreen + 1;
    if (nextScreen > SCREEN_PUMP_STATUS) {
        nextScreen = SCREEN_MAIN;
    }
    currentScreen = (DisplayScreen)nextScreen;
    lastScreenRotation = millis();
}

void DisplayOLED::setScreen(DisplayScreen screen) {
    currentScreen = screen;
    lastScreenRotation = millis();
}

void DisplayOLED::enableAutoRotate(bool enable, uint32_t interval) {
    autoRotateEnabled = enable;
    screenRotationInterval = interval;
}

void DisplayOLED::checkAutoRotate() {
    if (autoRotateEnabled && (millis() - lastScreenRotation >= screenRotationInterval)) {
        nextScreen();
    }
}

void DisplayOLED::drawProgressBar(int16_t x, int16_t y, int16_t width, int16_t height, float percent) {
    // Draw outline
    display.drawRect(x, y, width, height, SSD1306_WHITE);
    
    // Fill based on percentage
    int16_t fillWidth = (int16_t)((width - 2) * (percent / 100.0));
    if (fillWidth > 0) {
        display.fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
    }
}

void DisplayOLED::drawStatusIcons(const ConnectionStatus& status) {
    int16_t x = 128 - 8;  // Start from right edge
    
    // BLE icon
    drawBLEIcon(x, 0, status.bleConnected);
    x -= 12;
    
    // MQTT icon
    drawMQTTIcon(x, 0, status.mqttConnected);
    x -= 12;
    
    // WiFi icon
    drawWiFiIcon(x, 0, status.wifiConnected, status.wifiRSSI);
}

void DisplayOLED::drawWiFiIcon(int16_t x, int16_t y, bool connected, int8_t rssi) {
    if (connected) {
        // Draw WiFi waves
        display.drawPixel(x + 3, y + 5, SSD1306_WHITE);
        display.drawLine(x + 1, y + 3, x + 5, y + 3, SSD1306_WHITE);
        display.drawLine(x, y + 1, x + 6, y + 1, SSD1306_WHITE);
    } else {
        // Draw X
        display.drawLine(x, y, x + 6, y + 6, SSD1306_WHITE);
        display.drawLine(x + 6, y, x, y + 6, SSD1306_WHITE);
    }
}

void DisplayOLED::drawMQTTIcon(int16_t x, int16_t y, bool connected) {
    if (connected) {
        // Draw 'M' simplified
        display.drawLine(x, y + 6, x, y, SSD1306_WHITE);
        display.drawLine(x, y, x + 3, y + 3, SSD1306_WHITE);
        display.drawLine(x + 3, y + 3, x + 6, y, SSD1306_WHITE);
        display.drawLine(x + 6, y, x + 6, y + 6, SSD1306_WHITE);
    } else {
        // Draw empty square
        display.drawRect(x, y, 7, 7, SSD1306_WHITE);
    }
}

void DisplayOLED::drawBLEIcon(int16_t x, int16_t y, bool connected) {
    if (connected) {
        // Draw Bluetooth symbol (simplified)
        display.drawLine(x + 3, y, x + 3, y + 6, SSD1306_WHITE);
        display.drawLine(x + 3, y, x + 6, y + 2, SSD1306_WHITE);
        display.drawLine(x + 3, y + 6, x + 6, y + 4, SSD1306_WHITE);
        display.drawLine(x, y + 2, x + 6, y + 4, SSD1306_WHITE);
    } else {
        // Draw circle
        display.drawCircle(x + 3, y + 3, 3, SSD1306_WHITE);
    }
}

void DisplayOLED::drawPumpIcon(int16_t x, int16_t y, bool active) {
    // Simple pump representation
    display.drawRect(x, y + 2, 6, 4, SSD1306_WHITE);
    if (active) {
        display.fillRect(x + 1, y + 3, 4, 2, SSD1306_WHITE);
    }
}

void DisplayOLED::drawCenteredText(const char* text, int16_t y, uint8_t textSize) {
    display.setTextSize(textSize);
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((OLED_WIDTH - w) / 2, y);
    display.print(text);
}

