# 💧 Smart Water Level Monitor

**ESP32-based water level monitoring system with configurable single/dual tank modes, web configuration, MQTT, BLE, and automatic pump control.**

---

## Quick Start

### 1. Hardware Setup
- Wire JSN-SR04T sensor(s) to ESP32 (see wiring diagram in docs)
- Connect OLED display (I2C)
- Connect pump relay module
- **⚠️ Important:** Use voltage divider on Echo pins (5V→3.3V)

### 2. Upload Firmware
```bash
pio run --target upload
pio device monitor
```

### 3. Configure via Web Portal
1. Connect to WiFi: `WaterMonitor_Setup` (password: `12345678`)
2. Open browser to `http://192.168.4.1`
3. Configure WiFi, tank dimensions, MQTT settings
4. Save and reboot

### 4. Monitor Water Levels
- **OLED Display:** Real-time tank levels and status
- **Web Dashboard:** `http://[device-ip]/`
- **MQTT:** Subscribe to `water/level`
- **BLE:** Connect with nRF Connect app

---

## Features

✅ Single or dual tank monitoring  
✅ Configurable via web interface (no hardcoding!)  
✅ MQTT publishing with JSON payloads  
✅ BLE GATT service  
✅ OLED display  
✅ Automatic pump control with safety features  
✅ OTA firmware updates  
✅ Captive portal for easy setup  
✅ Persistent configuration in NVS flash  

---

## Hardware Requirements

- **ESP32 DevKit** (WROOM-32 or compatible)
- **JSN-SR04T** waterproof ultrasonic sensor (1-2×)
- **SSD1306 OLED** 0.96" I2C display
- **Relay module** for pump control
- **Voltage divider** (10kΩ + 20kΩ resistors) for Echo pins
- **5V power supply** (2A recommended)

---

## Default GPIO Pins

```
Tank 1 Sensor:   TRIG=GPIO25, ECHO=GPIO26
Tank 2 Sensor:   TRIG=GPIO32, ECHO=GPIO33
OLED Display:    SDA=GPIO21, SCL=GPIO22
Pump Relay:      GPIO27
Status LED:      GPIO2
```

*All pins configurable via web interface!*

---

## Documentation

📖 **Full documentation:** [docs/README.md](docs/README.md)

Includes:
- Detailed wiring diagrams
- Complete configuration guide
- API documentation
- MQTT integration examples
- BLE characteristic reference
- Troubleshooting guide
- Home Assistant integration

---

## Project Structure

```
water-level/
├── src/
│   ├── main.cpp              # Main application
│   ├── config.h              # Configuration constants
│   ├── config_manager.*      # NVS configuration storage
│   ├── sensor_ultrasonic.*   # Sensor driver with filtering
│   ├── display_oled.*        # OLED display driver
│   ├── wifi_manager.*        # WiFi + captive portal
│   ├── web_server.*          # Web configuration interface
│   ├── mqtt_client.*         # MQTT client with reconnect
│   ├── ble_service.*         # BLE GATT service
│   └── pump_controller.*     # Pump control logic
├── docs/
│   └── README.md             # Complete documentation
├── platformio.ini            # PlatformIO configuration
└── README.md                 # This file
```

---

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

