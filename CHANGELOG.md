# Changelog

All notable changes to the Water Level Monitor project will be documented in this file.

## [1.2.0] - 2025-10-31

### Added
- **ESP32-S2 Support**: Full compatibility with ESP32-S2 boards (WiFi-only variant)
  - Conditional compilation for BLE (available on ESP32 Classic only)
  - Single-core task scheduling for S2 (vs dual-core for Classic)
  - Board-specific GPIO pin mappings optimized for each variant
  - Native USB support for S2 debugging
- **ESP8266 Support**: Full compatibility with ESP8266 boards (Wemos D1 Mini Pro)
  - LittleFS + JSON for configuration storage (replaces NVS/Preferences)
  - Schedule library for cooperative multitasking (replaces FreeRTOS tasks)
  - ESP8266WiFi library compatibility layer
  - Board-specific API differences handled (WiFi modes, OTA, chip ID)
  - Reduced stack sizes optimized for limited RAM (80KB vs 327KB)
  - GPIO assignments mapped to Wemos D1 Mini pins (D1-D8)
- Multi-board PlatformIO configuration with separate environments (`esp32dev`, `esp32s2dev`, `d1_mini_pro`)
- Feature comparison matrix in documentation

### Changed
- Firmware version bumped to 1.2.0
- Refactored BLE code with `#if HAS_BLE` preprocessor guards
- Updated FreeRTOS task creation with board-specific core pinning
- GPIO pins optimized for each board (avoiding strapping pins)
- WiFi library includes now board-specific (ESP8266WiFi vs WiFi.h)
- ConfigManager uses LittleFS on ESP8266, NVS/Preferences on ESP32
- Renamed `WiFiState` enum to `WaterMonitorWiFiState` to avoid ESP8266 library conflicts

### Technical Details
- **ESP32-S2 GPIO**: I2C (GPIO8/9), Sensors (GPIO10/11/12/13), Relay (GPIO7), LED (GPIO15)
- **ESP8266 GPIO**: I2C (D2/D1), Sensors (D5/D6/D7/D8), Relay (D3), LED (D4)
- **ESP32 Classic GPIO**: Unchanged from v1.0.0
- **Flash Usage**: ESP8266 ~42% (437KB), ESP32-S2 ~28% (883KB), ESP32 ~55% (1.7MB)
- **RAM Usage**: ESP8266 ~59% (48KB/80KB), ESP32-S2 ~18%, ESP32 ~20%

## [1.0.0] - 2025-10-30

### Initial Release

#### Core Features
- ✅ Single and dual tank monitoring modes
- ✅ JSN-SR04T ultrasonic sensor support with 5-sample median filtering
- ✅ SSD1306 OLED display with multiple screen layouts
- ✅ Full web-based configuration interface
- ✅ Captive portal for easy first-time setup
- ✅ Persistent configuration storage in ESP32 NVS flash

#### Connectivity
- ✅ WiFi with auto-reconnect and AP fallback mode
- ✅ MQTT client with JSON payloads and offline buffering
- ✅ BLE GATT service with level characteristics
- ✅ OTA firmware updates (web and network)
- ✅ RESTful API for remote control

#### Pump Control
- ✅ Manual pump control (web, MQTT, BLE)
- ✅ Automatic pump control with configurable thresholds
- ✅ Safety features: dry-run protection, max runtime, cooldown
- ✅ Emergency stop on error conditions

#### Architecture
- ✅ Modular C++ architecture with clean separation of concerns
- ✅ FreeRTOS task-based design for concurrent operations
- ✅ Comprehensive error handling and recovery
- ✅ Debug logging with configurable levels
- ✅ Production-ready code with SOLID principles

#### Documentation
- ✅ Comprehensive README with quick start guide
- ✅ Detailed wiring diagrams and schematics
- ✅ Complete API documentation
- ✅ Configuration and calibration guides
- ✅ Troubleshooting section
- ✅ Home Assistant integration examples

### Hardware Support
- ESP32-WROOM-32 and compatible boards
- JSN-SR04T waterproof ultrasonic sensors
- SSD1306 0.96" OLED I2C displays
- Standard relay modules for pump control

### Configuration Options
- Tank dimensions (empty/full distances)
- Sensor GPIO pin assignments
- WiFi credentials
- MQTT broker settings
- Pump control thresholds and timing
- Display preferences
- All configurable via web interface!

### Safety & Reliability
- Voltage divider protection for 5V sensors
- Sensor timeout detection
- Consecutive error tracking
- Watchdog timer support
- Graceful degradation on component failure

---

## Future Roadmap

### Planned Features

#### High Priority
- [ ] **Offline Configuration Console with Hard Buttons**
  - 4-6 physical buttons for WiFi-free setup
  - OLED-based menu system for complete configuration
  - Guided calibration wizard
  - Perfect for remote/field installations
  - Button layout: UP/DOWN/SELECT/BACK/MENU
  - Optional rotary encoder support
  - Step-by-step tank calibration
  - Non-blocking menu task
  - All settings accessible offline

#### Medium Priority
- [ ] Blynk integration
- [ ] Data logging to SD card with timestamp
- [ ] Email/SMS alerts via IFTTT webhooks
- [ ] Scheduled pump operation (time-based rules)
- [ ] Historical data graphs (24hr/7day/30day)
- [ ] Advanced analytics and water usage predictions

#### Low Priority
- [ ] Multiple language support in web interface
- [ ] Mobile companion app (iOS/Android)
- [ ] Pressure sensor alternative for submerged applications
- [ ] Battery and solar panel monitoring
- [ ] Voice assistant integration (Alexa/Google Home)

### Under Consideration
- [ ] Modbus RTU support for industrial PLCs
- [ ] LoRaWAN connectivity option
- [ ] Multiple device coordination
- [ ] Cloud dashboard integration
- [ ] Voice assistant integration (Alexa/Google Home)

---

## Version History

- **1.0.0** (2025-10-30) - Initial release with full feature set

---

## Contributing

Contributions are welcome! Please see our contribution guidelines and submit pull requests for:
- Bug fixes
- New features
- Documentation improvements
- Example integrations

---

## License

MIT License - See LICENSE file for details.

