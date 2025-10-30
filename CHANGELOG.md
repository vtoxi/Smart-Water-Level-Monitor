# Changelog

All notable changes to the Water Level Monitor project will be documented in this file.

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

