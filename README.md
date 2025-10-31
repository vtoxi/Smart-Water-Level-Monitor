# 💧 Smart Water Level Monitor

**ESP32-based water level monitoring system with configurable single/dual tank modes, web configuration, MQTT, BLE, and automatic pump control.**

---

## Quick Start

### 1. Build the Project

```bash
pio run -e esp32dev
```

The project uses the published [IonConnect library](https://registry.platformio.org/libraries/coderunner/IonConnect) from PlatformIO registry.

### 2. Hardware Setup
- Wire JSN-SR04T sensor(s) to ESP32 (see wiring diagram in docs)
- Connect OLED display (I2C)
- Connect pump relay module
- **⚠️ Important:** Use voltage divider on Echo pins (5V→3.3V)

### 3. Upload Firmware
```bash
pio run --target upload -e esp32dev
pio device monitor
```

### 4. Configure via Web Portal (IonConnect)
1. Connect to WiFi: `IonConnect-XXXX` 
2. Browser automatically opens to `http://192.168.4.1`
3. Configure WiFi, tank dimensions, MQTT settings
4. Save and device connects automatically

### 5. Monitor Water Levels
- **OLED Display:** Real-time tank levels and status
- **Web Dashboard:** `http://[device-ip]/`
- **MQTT:** Subscribe to `water/level`
- **BLE:** Connect with nRF Connect app

---

## Features

### Water Monitoring
✅ Single or dual tank monitoring  
✅ JSN-SR04T waterproof ultrasonic sensors  
✅ Real-time measurements with median filtering  
✅ Configurable tank depth, units, and thresholds  
✅ OLED display with visual progress bars  

### Connectivity (Powered by IonConnect)
✨ **Modern captive portal** with beautiful responsive UI  
✨ **Live dashboard** with real-time status updates (SSE)  
✨ **BLE provisioning** for ESP32 (configure via Bluetooth)  
✨ **REST API** for programmatic access  
✨ **Multi-network support** with priority management  
✨ **mDNS discovery** (access via devicename.local)  
✨ **Security features** (password protection, API tokens)  

### Data & Control
✅ **MQTT publishing** with JSON payloads and QoS  
✅ **Automatic pump control** with safety features:  
  - Dry-run protection (sensor error detection)  
  - Maximum runtime limits (prevents overflow)  
  - Cooldown periods (prevents motor damage)  
  - Auto/manual modes  

### Management
✅ **OTA firmware updates** (via web interface)  
✅ **Configuration backup/restore** (JSON export/import)  
✅ **Diagnostics dashboard** (memory, WiFi stats, errors)  
✅ **Persistent storage** (NVS on ESP32, LittleFS on ESP8266)  
✅ **Web configuration** (no hardcoding required!)  

---

## Hardware Requirements

### Supported Boards
- **ESP32 DevKit** (WROOM-32 or compatible) - Dual core with BLE
- **ESP32-S2** (Saola-1 or compatible) - Single core, WiFi only (no BLE)
- **ESP8266** (Wemos D1 Mini Pro) - Single core, WiFi only, 80MHz

### Components
- **JSN-SR04T** waterproof ultrasonic sensor (1-2×)
- **SSD1306 OLED** 0.96" I2C display
- **Relay module** for pump control
- **Voltage divider** (10kΩ + 20kΩ resistors) for Echo pins
- **5V power supply** (2A recommended)

---

## Default GPIO Pins

### ESP32 Classic
```
Tank 1 Sensor:   TRIG=GPIO25, ECHO=GPIO26
Tank 2 Sensor:   TRIG=GPIO32, ECHO=GPIO33
OLED Display:    SDA=GPIO21, SCL=GPIO22
Pump Relay:      GPIO27
Status LED:      GPIO2
```

### ESP32-S2
```
Tank 1 Sensor:   TRIG=GPIO10, ECHO=GPIO11
Tank 2 Sensor:   TRIG=GPIO12, ECHO=GPIO13
OLED Display:    SDA=GPIO8,  SCL=GPIO9
Pump Relay:      GPIO7
Status LED:      GPIO15
```

### ESP8266 (Wemos D1 Mini Pro)
```
Tank 1 Sensor:   TRIG=GPIO14 (D5), ECHO=GPIO12 (D6)
Tank 2 Sensor:   TRIG=GPIO13 (D7), ECHO=GPIO15 (D8)
OLED Display:    SDA=GPIO4   (D2), SCL=GPIO5  (D1)
Pump Relay:      GPIO0  (D3)
Status LED:      GPIO2  (D4)
```

*All pins configurable via web interface!*

---

## Documentation

### Core Documentation
📖 **[docs/README.md](docs/README.md)** - Complete project documentation  
🧪 **[TESTING_GUIDE.md](TESTING_GUIDE.md)** - Comprehensive testing procedures (11 scenarios)  
📘 **[ION_CONNECT_INTEGRATION.md](ION_CONNECT_INTEGRATION.md)** - IonConnect integration details  
📋 **[COMPLETE_SUMMARY.md](COMPLETE_SUMMARY.md)** - Project overview and status  

### Setup & Deployment
🚀 **[EXECUTE_WORKFLOW.md](EXECUTE_WORKFLOW.md)** - Complete build and deployment guide  
📦 **[PUBLISHING_STEPS.md](PUBLISHING_STEPS.md)** - Library publishing instructions  

### Included Guides
- Detailed wiring diagrams for all supported boards
- Complete configuration guide via web interface
- API documentation (REST & MQTT)
- MQTT integration examples with QoS
- BLE characteristic reference (ESP32)
- Troubleshooting guide with solutions
- Home Assistant integration examples
- OTA update procedures

---

## Project Structure

```
water-level/
├── src/
│   ├── main.cpp              # Main application
│   ├── config.h              # Configuration constants
│   ├── config_manager.*      # NVS/LittleFS configuration storage
│   ├── sensor_ultrasonic.*   # Sensor driver with median filtering
│   ├── display_oled.*        # OLED display driver
│   ├── wifi_ionconnect.*     # IonConnect WiFi wrapper (NEW)
│   ├── web_server.*          # Web configuration interface
│   ├── mqtt_client.*         # MQTT client with auto-reconnect
│   ├── ble_service.*         # BLE GATT service (ESP32 only)
│   └── pump_controller.*     # Pump control logic
├── docs/
│   └── README.md             # Complete documentation
├── platformio.ini            # PlatformIO configuration
└── README.md                 # This file
```

---

## Board Selection

Choose your board in `platformio.ini`:
```ini
# For ESP32 Classic (with BLE support):
default_envs = esp32dev

# For ESP32-S2 (WiFi only, no BLE):
default_envs = esp32s2dev

# For ESP8266 (Wemos D1 Mini Pro):
default_envs = d1_mini_pro
```

### Feature Matrix
```
Feature              | ESP32 Classic | ESP32-S2  | ESP8266
---------------------|---------------|-----------|----------
WiFi                 |      ✅       |    ✅     |    ✅
MQTT                 |      ✅       |    ✅     |    ✅
BLE                  |      ✅       |    ❌     |    ❌
Web Server           |      ✅       |    ✅     |    ✅
OLED Display         |      ✅       |    ✅     |    ✅
Pump Control         |      ✅       |    ✅     |    ✅
Dual Core Tasks      |      ✅       |    ❌     |    ❌
Native USB           |      ❌       |    ✅     |    ❌
CPU Speed            |   240MHz      |  240MHz   |   80MHz
RAM                  |   327KB       |  327KB    |   80KB
Flash Usage          |   55% (1.7MB) | 28% (883KB)| 42% (437KB)
```

## Configuration

### Web Interface

Access at `http://[device-ip]/` for:
- WiFi setup
- Tank calibration (dimensions, units)
- MQTT broker settings
- Pump control thresholds
- System status and monitoring
- OTA firmware upload

### Tank Calibration

1. Measure distance from sensor to tank bottom when **empty** → Enter as "Empty Distance"
2. Fill tank to desired "full" level
3. Measure distance from sensor to water surface → Enter as "Full Distance"

Example:
- Sensor mounted 200cm above tank bottom
- Full level is 10cm below sensor
- Configuration: Empty=200cm, Full=10cm

---

## API Examples

### REST API

```bash
# Get status
curl http://[device-ip]/api/status

# Control pump
curl -X POST http://[device-ip]/api/pump -d '{"action":"on"}'
```

### MQTT

```bash
# Subscribe to readings
mosquitto_sub -h mqtt.example.com -t "water/level"

# Control pump
mosquitto_pub -h mqtt.example.com -t "water/command" -m '{"command":"pump_on"}'
```

### BLE

Connect to service UUID: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- Tank 1 Level: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- Tank 2 Level: `beb5483e-36e1-4688-b7f5-ea07361b26a9`
- Pump Control: `beb5483e-36e1-4688-b7f5-ea07361b26aa`

---

## Safety Features

🛡️ **Pump Protection:**
- Dry-run protection (stops if source tank low)
- Maximum run time limits
- Minimum run time enforcement
- Cooldown periods between cycles
- Emergency stop on errors

🛡️ **Sensor Monitoring:**
- Median filtering (5 samples)
- Timeout detection
- Health status tracking
- Error recovery

---

## Troubleshooting

**Sensor not reading?**
- Check voltage divider on Echo pin (must reduce 5V to ~1.6V)
- Verify sensor has clear line-of-sight to water
- Ensure sensor is within range (25-450cm for JSN-SR04T)

**Can't connect to WiFi portal?**
- SSID: `WaterMonitor_Setup`
- Password: `12345678`
- Direct URL: `http://192.168.4.1`

**MQTT not connecting?**
- Verify broker address and port
- Check username/password (if authentication enabled)
- Test with MQTT client tool first

See [docs/README.md](docs/README.md) for comprehensive troubleshooting.

---

## OTA Updates

### Via Web Interface
1. Navigate to System tab
2. Upload `.bin` firmware file

### Via PlatformIO
```bash
pio run --target upload --upload-port [device-ip]
```

Default OTA password: `admin123` (change in config.h!)

---

## Roadmap

Future enhancements:
- [ ] **Offline Configuration Console** - Physical button interface (4-6 buttons + OLED menu) for WiFi-free setup in field deployments
- [ ] Blynk integration
- [ ] Pressure sensor support
- [ ] Data logging to SD card
- [ ] Email/SMS alerts via IFTTT
- [ ] Multi-language web interface
- [ ] Mobile app (iOS/Android)
- [ ] Battery monitoring
- [ ] Solar panel integration
- [ ] Historical data graphs

---

## 🌐 IonConnect WiFi Library

This project uses **[IonConnect](https://registry.platformio.org/libraries/coderunner/IonConnect)** - a next-generation WiFi provisioning library for ESP32/ESP8266.

### Why IonConnect?

**Modern User Experience:**
- 🎨 Beautiful responsive web interface (Tailwind CSS)
- 📱 Mobile-first design, perfect on all devices
- 🔄 Live status updates via Server-Sent Events
- ⚡ Fast, async, non-blocking operation

**Advanced Features:**
- 📡 **BLE Provisioning** - Configure via Bluetooth (ESP32)
- 🌐 **REST API** - Programmatic configuration access
- 💾 **Config Backup/Restore** - JSON export/import
- 🔍 **mDNS Support** - Access via `devicename.local`
- 📊 **Diagnostics Dashboard** - System health monitoring
- 🔐 **Security** - Password protection and API tokens
- 📡 **Multi-Network** - Store and prioritize multiple WiFi networks

**Developer Friendly:**
- 🔌 Plugin architecture for extensibility
- 📚 Comprehensive documentation
- 🎯 9 working examples included
- 🐛 Active maintenance and updates
- 🤝 Community-driven improvements

### Installation

**PlatformIO (Already included):**
```ini
lib_deps = coderunner/IonConnect@^1.0.2
```

**Arduino IDE:**
```bash
# Library Manager: Search "IonConnect"
```

### Resources

- **Library:** https://registry.platformio.org/libraries/coderunner/IonConnect
- **Documentation:** See `/IonConnect/README.md` in workspace  
- **Integration Guide:** See `/IonConnect/INTEGRATION.md`

---

## License

MIT License - See LICENSE file for details

---

## Credits

Built with:
- ESP32 Arduino Framework
- PlatformIO
- Adafruit GFX & SSD1306 libraries
- PubSubClient (MQTT)
- ESPAsyncWebServer
- ArduinoJson

---

**Made with ❤️ for the IoT community**

Questions? Issues? Open a GitHub issue or check the [full documentation](docs/README.md).

