#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// BOARD DETECTION
// ============================================================================
#if defined(CONFIG_IDF_TARGET_ESP32S2) || defined(ESP32_S2)
    #define BOARD_ESP32_S2
    #define BOARD_NAME "ESP32-S2"
    #define HAS_BLE false
#elif defined(CONFIG_IDF_TARGET_ESP32) || defined(ESP32_CLASSIC)
    #define BOARD_ESP32_CLASSIC
    #define BOARD_NAME "ESP32"
    #define HAS_BLE true
#else
    // Default to ESP32 Classic
    #define BOARD_ESP32_CLASSIC
    #define BOARD_NAME "ESP32"
    #define HAS_BLE true
#endif

// ============================================================================
// FIRMWARE VERSION
// ============================================================================
#define FIRMWARE_VERSION "1.1.0"
#define DEVICE_NAME "WaterMonitor"

// ============================================================================
// DEFAULT GPIO PIN ASSIGNMENTS
// ============================================================================

#ifdef BOARD_ESP32_S2
    // ESP32-S2 specific pins (avoiding strapping pins)
    // Tank 1 Ultrasonic Sensor (JSN-SR04T)
    #define DEFAULT_TRIG_PIN_1      10
    #define DEFAULT_ECHO_PIN_1      11
    
    // Tank 2 Ultrasonic Sensor (JSN-SR04T) - Optional for dual-tank mode
    #define DEFAULT_TRIG_PIN_2      12
    #define DEFAULT_ECHO_PIN_2      13
    
    // OLED Display (I2C)
    #define OLED_SDA_PIN            8
    #define OLED_SCL_PIN            9
    #define OLED_RESET_PIN          -1
    #define OLED_WIDTH              128
    #define OLED_HEIGHT             64
    #define OLED_ADDRESS            0x3C
    
    // Pump Control Relay
    #define DEFAULT_PUMP_RELAY_PIN  7
    
    // Status LED
    #define STATUS_LED_PIN          15    // Built-in LED on ESP32-S2
#else
    // ESP32 Classic pins
    // Tank 1 Ultrasonic Sensor (JSN-SR04T)
    #define DEFAULT_TRIG_PIN_1      25
    #define DEFAULT_ECHO_PIN_1      26
    
    // Tank 2 Ultrasonic Sensor (JSN-SR04T) - Optional for dual-tank mode
    #define DEFAULT_TRIG_PIN_2      32
    #define DEFAULT_ECHO_PIN_2      33
    
    // OLED Display (I2C)
    #define OLED_SDA_PIN            21
    #define OLED_SCL_PIN            22
    #define OLED_RESET_PIN          -1    // Reset pin (or -1 if sharing Arduino reset pin)
    #define OLED_WIDTH              128
    #define OLED_HEIGHT             64
    #define OLED_ADDRESS            0x3C
    
    // Pump Control Relay
    #define DEFAULT_PUMP_RELAY_PIN  27
    
    // Status LED (optional)
    #define STATUS_LED_PIN          2     // Built-in LED on most ESP32 boards
#endif

// ============================================================================
// SENSOR CONFIGURATION
// ============================================================================
#define SENSOR_TIMEOUT_US       30000  // 30ms timeout for ultrasonic sensor
#define SENSOR_SAMPLES          5      // Number of samples for median filter
#define SENSOR_READ_INTERVAL    5000   // Sensor reading interval in ms
#define SPEED_OF_SOUND          0.0343 // Speed of sound in cm/μs (at 20°C)

// ============================================================================
// DEFAULT TANK CALIBRATION (User configurable via web interface)
// ============================================================================
#define DEFAULT_TANK1_EMPTY_CM  200.0  // Distance when tank is empty (sensor to bottom)
#define DEFAULT_TANK1_FULL_CM   10.0   // Distance when tank is full (sensor to water surface)
#define DEFAULT_TANK2_EMPTY_CM  200.0
#define DEFAULT_TANK2_FULL_CM   10.0

// ============================================================================
// WIFI & CAPTIVE PORTAL
// ============================================================================
#define AP_SSID                 "WaterMonitor_Setup"
#define AP_PASSWORD             "12345678"          // Minimum 8 characters
#define AP_IP                   IPAddress(192, 168, 4, 1)
#define AP_GATEWAY              IPAddress(192, 168, 4, 1)
#define AP_SUBNET               IPAddress(255, 255, 255, 0)
#define WIFI_CONNECT_TIMEOUT    20000               // 20 seconds
#define WIFI_RECONNECT_INTERVAL 30000               // 30 seconds

// ============================================================================
// MQTT DEFAULT CONFIGURATION (User configurable)
// ============================================================================
#define DEFAULT_MQTT_BROKER     "mqtt.example.com"
#define DEFAULT_MQTT_PORT       1883
#define DEFAULT_MQTT_USER       ""
#define DEFAULT_MQTT_PASSWORD   ""
#define DEFAULT_MQTT_TOPIC      "water/level"
#define DEFAULT_MQTT_CMD_TOPIC  "water/command"
#define MQTT_RECONNECT_INTERVAL 5000                // 5 seconds
#define MQTT_PUBLISH_INTERVAL   10000               // 10 seconds

// ============================================================================
// BLE CONFIGURATION
// ============================================================================
#define BLE_SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BLE_TANK1_CHAR_UUID     "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_TANK2_CHAR_UUID     "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define BLE_PUMP_CHAR_UUID      "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define BLE_CONFIG_CHAR_UUID    "beb5483e-36e1-4688-b7f5-ea07361b26ab"

// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================
#define DISPLAY_UPDATE_INTERVAL 1000                // 1 second
#define SCREEN_ROTATION_INTERVAL 5000               // 5 seconds for rotating screens
#define DISPLAY_TIMEOUT         0                   // 0 = never timeout, >0 = timeout in ms

// ============================================================================
// PUMP CONTROL SAFETY LIMITS
// ============================================================================
#define PUMP_MIN_RUN_TIME       10000               // Minimum 10 seconds
#define PUMP_MAX_RUN_TIME       3600000             // Maximum 60 minutes
#define PUMP_COOLDOWN_TIME      60000               // 1 minute cooldown between runs
#define PUMP_AUTO_ON_THRESHOLD  20.0                // Auto-start pump at 20% level
#define PUMP_AUTO_OFF_THRESHOLD 90.0                // Auto-stop pump at 90% level
#define PUMP_DRY_RUN_THRESHOLD  5.0                 // Stop if source tank below 5%

// ============================================================================
// TASK PRIORITIES & STACK SIZES (FreeRTOS)
// ============================================================================
#define SENSOR_TASK_PRIORITY    2
#define SENSOR_TASK_STACK       4096
#define DISPLAY_TASK_PRIORITY   1
#define DISPLAY_TASK_STACK      4096
#define NETWORK_TASK_PRIORITY   2
#define NETWORK_TASK_STACK      8192
#define WEB_TASK_PRIORITY       1
#define WEB_TASK_STACK          8192

// ============================================================================
// OTA CONFIGURATION
// ============================================================================
#define OTA_HOSTNAME            "waterlevel-monitor"
#define OTA_PASSWORD            "admin123"          // Change this!

// ============================================================================
// WATCHDOG TIMER
// ============================================================================
#define WATCHDOG_TIMEOUT        30                  // 30 seconds

// ============================================================================
// POWER MANAGEMENT
// ============================================================================
#define DEEP_SLEEP_ENABLED      false               // Enable deep sleep mode
#define DEEP_SLEEP_DURATION     300                 // 5 minutes in seconds

// ============================================================================
// DEBUGGING
// ============================================================================
#define DEBUG_SERIAL            true
#define DEBUG_SENSOR            true
#define DEBUG_WIFI              true
#define DEBUG_MQTT              true
#define DEBUG_BLE               true

// Debug macro
#if DEBUG_SERIAL
    #define DEBUG_PRINT(x)      Serial.print(x)
    #define DEBUG_PRINTLN(x)    Serial.println(x)
    #define DEBUG_PRINTF(...)   Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H

