#include <Arduino.h>
#include <Wire.h>
#include <ArduinoOTA.h>
#include "config.h"
#include "config_manager.h"
#include "sensor_ultrasonic.h"
#include "display_oled.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "mqtt_client.h"
#include "ble_service.h"
#include "pump_controller.h"

// ============================================================================
// GLOBAL INSTANCES
// ============================================================================
ConfigManager configManager;
WiFiManager wifiManager(configManager);
WebServer webServer(configManager);
MQTTClient mqttClient(configManager);
BLEService bleService(configManager);
PumpController pumpController(configManager);
DisplayOLED display;

// Sensors (dynamically allocated based on configuration)
UltrasonicSensor* sensor1 = nullptr;
UltrasonicSensor* sensor2 = nullptr;

// ============================================================================
// TASK HANDLES
// ============================================================================
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t displayTaskHandle = NULL;
TaskHandle_t networkTaskHandle = NULL;

// ============================================================================
// GLOBAL STATE
// ============================================================================
SensorReading tank1Reading;
SensorReading tank2Reading;
bool systemInitialized = false;
uint32_t lastMQTTPublish = 0;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================
void setupOTA();
void setupSensors();
void sensorTask(void* parameter);
void displayTask(void* parameter);
void networkTask(void* parameter);
void mqttCallback(char* topic, byte* payload, unsigned int length);
ConnectionStatus getConnectionStatus();

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    // Initialize Serial
    Serial.begin(115200);
    delay(1000);
    
    DEBUG_PRINTLN("\n\n");
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN("  Water Level Monitor");
    DEBUG_PRINTF("  Version: %s\n", FIRMWARE_VERSION);
    DEBUG_PRINTLN("========================================\n");
    
    // Initialize status LED
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH);
    
    // Initialize configuration manager
    DEBUG_PRINTLN("Initializing configuration...");
    if (!configManager.begin()) {
        DEBUG_PRINTLN("ERROR: Failed to initialize configuration!");
        while (1) {
            digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
            delay(100);
        }
    }
    configManager.printConfig();
    
    // Initialize display
    DEBUG_PRINTLN("Initializing display...");
    if (display.begin()) {
        display.showBootScreen();
        delay(2000);
    } else {
        DEBUG_PRINTLN("WARNING: Display initialization failed");
    }
    
    // Initialize sensors
    setupSensors();
    
    // Initialize pump controller
    DEBUG_PRINTLN("Initializing pump controller...");
    pumpController.begin();
    
    // Initialize WiFi
    DEBUG_PRINTLN("Initializing WiFi...");
    wifiManager.begin();
    
    if (configManager.isWiFiConfigured()) {
        DEBUG_PRINTLN("Connecting to WiFi...");
        display.showConfigMode("Connecting...", "Please wait");
        
        if (!wifiManager.connect()) {
            DEBUG_PRINTLN("WiFi connection failed, starting AP mode...");
            wifiManager.startAP();
        }
    } else {
        DEBUG_PRINTLN("No WiFi configuration, starting AP mode...");
        wifiManager.startAP();
    }
    
    // Show config portal info if in AP mode
    if (wifiManager.isAPMode()) {
        display.showConfigMode(AP_SSID, wifiManager.getAPIP().c_str());
    }
    
    // Initialize web server
    DEBUG_PRINTLN("Initializing web server...");
    webServer.setSensor1(sensor1);
    webServer.setSensor2(sensor2);
    webServer.setPumpController(&pumpController);
    webServer.begin();
    
    // Initialize MQTT client if WiFi is connected
    if (wifiManager.isConnected()) {
        DEBUG_PRINTLN("Initializing MQTT...");
        mqttClient.begin();
        mqttClient.setCallback(mqttCallback);
        mqttClient.connect();
    }
    
    // Initialize BLE
    DEBUG_PRINTLN("Initializing BLE...");
    bleService.begin();
    bleService.setPumpController(&pumpController);
    
    // Setup OTA updates
    setupOTA();
    
    // Create FreeRTOS tasks
    DEBUG_PRINTLN("Creating tasks...");
    
    xTaskCreatePinnedToCore(
        sensorTask,
        "SensorTask",
        SENSOR_TASK_STACK,
        NULL,
        SENSOR_TASK_PRIORITY,
        &sensorTaskHandle,
        0  // Core 0
    );
    
    xTaskCreatePinnedToCore(
        displayTask,
        "DisplayTask",
        DISPLAY_TASK_STACK,
        NULL,
        DISPLAY_TASK_PRIORITY,
        &displayTaskHandle,
        0  // Core 0
    );
    
    xTaskCreatePinnedToCore(
        networkTask,
        "NetworkTask",
        NETWORK_TASK_STACK,
        NULL,
        NETWORK_TASK_PRIORITY,
        &networkTaskHandle,
        1  // Core 1
    );
    
    systemInitialized = true;
    digitalWrite(STATUS_LED_PIN, LOW);
    
    DEBUG_PRINTLN("\n========================================");
    DEBUG_PRINTLN("  System Initialized Successfully!");
    DEBUG_PRINTLN("========================================\n");
}

// ============================================================================
// MAIN LOOP
// ============================================================================
void loop() {
    // Handle OTA updates
    ArduinoOTA.handle();
    
    // Small delay to prevent watchdog issues
    delay(10);
}

// ============================================================================
// SENSOR TASK
// ============================================================================
void sensorTask(void* parameter) {
    const SystemConfig& config = configManager.getConfig();
    
    DEBUG_PRINTLN("Sensor task started");
    
    while (1) {
        // Read sensor 1
        if (sensor1) {
            tank1Reading = sensor1->readDistance();
            
            if (!tank1Reading.isValid) {
                DEBUG_PRINTLN("WARNING: Tank 1 sensor reading invalid");
            }
        }
        
        // Read sensor 2 if in dual-tank mode
        if (config.tankMode == DUAL_TANK && sensor2) {
            tank2Reading = sensor2->readDistance();
            
            if (!tank2Reading.isValid) {
                DEBUG_PRINTLN("WARNING: Tank 2 sensor reading invalid");
            }
        }
        
        // Update pump controller (automatic mode)
        if (tank1Reading.isValid) {
            float sourceLevel = (config.tankMode == DUAL_TANK && tank2Reading.isValid) 
                                ? tank2Reading.levelPercent 
                                : 100.0;
            pumpController.update(tank1Reading.levelPercent, sourceLevel);
        }
        
        // Update BLE characteristics
        if (tank1Reading.isValid) {
            bleService.updateTank1Level(tank1Reading.levelPercent);
        }
        if (config.tankMode == DUAL_TANK && tank2Reading.isValid) {
            bleService.updateTank2Level(tank2Reading.levelPercent);
        }
        bleService.updatePumpStatus(pumpController.isRunning());
        
        // Blink LED to indicate activity
        digitalWrite(STATUS_LED_PIN, HIGH);
        vTaskDelay(50 / portTICK_PERIOD_MS);
        digitalWrite(STATUS_LED_PIN, LOW);
        
        // Wait for next reading
        vTaskDelay(config.sensorReadInterval / portTICK_PERIOD_MS);
    }
}

// ============================================================================
// DISPLAY TASK
// ============================================================================
void displayTask(void* parameter) {
    const SystemConfig& config = configManager.getConfig();
    
    DEBUG_PRINTLN("Display task started");
    
    while (1) {
        if (!config.displayEnabled) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        
        ConnectionStatus status = getConnectionStatus();
        
        // Show appropriate screen based on mode
        if (wifiManager.isAPMode()) {
            display.showConfigMode(AP_SSID, wifiManager.getAPIP().c_str());
        } else if (config.tankMode == SINGLE_TANK) {
            display.showSingleTankMain(
                config.tank1Name,
                tank1Reading.levelPercent,
                tank1Reading.distanceCm,
                status
            );
        } else {
            display.showDualTankMain(
                config.tank1Name,
                tank1Reading.levelPercent,
                config.tank2Name,
                tank2Reading.levelPercent,
                status
            );
        }
        
        // Check for auto-rotation
        display.checkAutoRotate();
        
        vTaskDelay(DISPLAY_UPDATE_INTERVAL / portTICK_PERIOD_MS);
    }
}

// ============================================================================
// NETWORK TASK
// ============================================================================
void networkTask(void* parameter) {
    DEBUG_PRINTLN("Network task started");
    
    uint32_t lastWiFiCheck = 0;
    uint32_t lastMQTTCheck = 0;
    
    while (1) {
        // Handle DNS for captive portal
        if (wifiManager.isAPMode()) {
            wifiManager.handleDNS();
        }
        
        // Check WiFi connection (every 5 seconds)
        if (millis() - lastWiFiCheck >= 5000) {
            wifiManager.checkConnection();
            lastWiFiCheck = millis();
        }
        
        // MQTT handling
        if (wifiManager.isConnected()) {
            // Check MQTT connection (every 5 seconds)
            if (millis() - lastMQTTCheck >= 5000) {
                mqttClient.checkConnection();
                lastMQTTCheck = millis();
            }
            
            // Process MQTT messages
            mqttClient.loop();
            
            // Publish sensor data periodically
            if (tank1Reading.isValid) {
                const SystemConfig& config = configManager.getConfig();
                const SensorReading* tank2Ptr = (config.tankMode == DUAL_TANK && tank2Reading.isValid) 
                                                ? &tank2Reading 
                                                : nullptr;
                mqttClient.publishSensorData(tank1Reading, tank2Ptr);
            }
        }
        
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// ============================================================================
// SETUP FUNCTIONS
// ============================================================================
void setupSensors() {
    const SystemConfig& config = configManager.getConfig();
    
    DEBUG_PRINTLN("Initializing sensors...");
    
    // Initialize sensor 1 (always present)
    sensor1 = new UltrasonicSensor(
        config.trigPin1,
        config.echoPin1,
        config.tank1EmptyCm,
        config.tank1FullCm
    );
    sensor1->begin();
    
    // Initialize sensor 2 if in dual-tank mode
    if (config.tankMode == DUAL_TANK) {
        sensor2 = new UltrasonicSensor(
            config.trigPin2,
            config.echoPin2,
            config.tank2EmptyCm,
            config.tank2FullCm
        );
        sensor2->begin();
    }
    
    DEBUG_PRINTLN("Sensors initialized");
}

void setupOTA() {
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setPassword(OTA_PASSWORD);
    
    ArduinoOTA.onStart([]() {
        String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
        DEBUG_PRINTLN("OTA: Start updating " + type);
        
        // Stop tasks during OTA
        if (sensorTaskHandle) vTaskSuspend(sensorTaskHandle);
        if (displayTaskHandle) vTaskSuspend(displayTaskHandle);
        if (networkTaskHandle) vTaskSuspend(networkTaskHandle);
        
        display.clear();
        display.showConfigMode("OTA Update", "Please wait...");
    });
    
    ArduinoOTA.onEnd([]() {
        DEBUG_PRINTLN("\nOTA: Update complete!");
        display.showConfigMode("Update Complete", "Rebooting...");
        delay(1000);
    });
    
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        int percent = (progress / (total / 100));
        DEBUG_PRINTF("OTA Progress: %u%%\r", percent);
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        DEBUG_PRINTF("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) DEBUG_PRINTLN("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) DEBUG_PRINTLN("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTLN("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTLN("Receive Failed");
        else if (error == OTA_END_ERROR) DEBUG_PRINTLN("End Failed");
        
        display.showError("OTA Failed!");
        delay(3000);
    });
    
    ArduinoOTA.begin();
    DEBUG_PRINTLN("OTA initialized");
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================
ConnectionStatus getConnectionStatus() {
    ConnectionStatus status;
    status.wifiConnected = wifiManager.isConnected();
    status.mqttConnected = mqttClient.isConnected();
    status.bleConnected = bleService.isClientConnected();
    status.wifiRSSI = status.wifiConnected ? wifiManager.getRSSI() : 0;
    status.wifiSSID = status.wifiConnected ? wifiManager.getSSID() : "";
    return status;
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Parse incoming MQTT messages
    DEBUG_PRINTF("MQTT: Message received [%s]: ", topic);
    
    // Create null-terminated string from payload
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';
    
    DEBUG_PRINTLN(message);
    
    const SystemConfig& config = configManager.getConfig();
    
    // Handle commands
    if (strcmp(topic, config.mqttCmdTopic) == 0) {
        // Parse JSON command
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, message);
        
        if (!error) {
            const char* cmd = doc["command"];
            
            if (cmd && strcmp(cmd, "pump_on") == 0) {
                DEBUG_PRINTLN("MQTT: Pump ON command received");
                pumpController.turnOn();
            } else if (cmd && strcmp(cmd, "pump_off") == 0) {
                DEBUG_PRINTLN("MQTT: Pump OFF command received");
                pumpController.turnOff();
            } else if (cmd && strcmp(cmd, "status") == 0) {
                DEBUG_PRINTLN("MQTT: Status request received");
                mqttClient.publishStatus(
                    wifiManager.isConnected(),
                    mqttClient.isConnected(),
                    bleService.isRunning(),
                    pumpController.isRunning()
                );
            }
        }
    }
}

