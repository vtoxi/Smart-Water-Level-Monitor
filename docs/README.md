# 💧 Smart Water Level Monitor

**Version 1.0.0**

A production-ready, industrial-grade water level monitoring system for ESP32 with JSN-SR04T ultrasonic sensors. Features configurable single/dual tank modes, web-based captive portal configuration, MQTT/BLE connectivity, OLED display, and automatic pump control.

---

## 📋 Table of Contents

- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Wiring Diagram](#wiring-diagram)
- [Getting Started](#getting-started)
- [First-Time Setup](#first-time-setup)
- [Configuration](#configuration)
- [Usage](#usage)
- [API Documentation](#api-documentation)
- [MQTT Integration](#mqtt-integration)
- [BLE Integration](#ble-integration)
- [OTA Updates](#ota-updates)
- [Troubleshooting](#troubleshooting)

---

## ✨ Features

### Core Features
- ✅ **Single or Dual Tank Mode** - Monitor one or two tanks independently
- ✅ **JSN-SR04T Waterproof Sensors** - Industrial-grade ultrasonic sensors with median filtering
- ✅ **OLED Display** - Real-time visual feedback with connection status
- ✅ **Web Configuration Portal** - Complete setup via captive portal (no hardcoding!)
- ✅ **WiFi Connectivity** - Auto-reconnect with fallback to AP mode
- ✅ **MQTT Publishing** - JSON payloads with auto-reconnect and offline buffering
- ✅ **BLE GATT Service** - Read levels and control pump via Bluetooth
- ✅ **Automatic Pump Control** - Smart pump management with safety features
- ✅ **OTA Updates** - Over-the-air firmware updates
- ✅ **Persistent Configuration** - All settings saved to ESP32 NVS flash

### Safety Features
- 🛡️ Pump dry-run protection
- 🛡️ Maximum run time limits
- 🛡️ Cooldown periods between pump cycles
- 🛡️ Emergency stop on error conditions
- 🛡️ Sensor health monitoring

### Configurability
- 📊 Adjustable tank dimensions (any depth, any units)
- 📊 Customizable sensor GPIO pins
- 📊 Configurable pump thresholds
- 📊 Flexible MQTT topics and broker settings
- 📊 Tank naming for multi-tank installations

---

## 🔧 Hardware Requirements

### Required Components

| Component | Specification | Quantity | Notes |
|-----------|--------------|----------|-------|
| **ESP32 DevKit** | ESP32-WROOM-32 | 1 | Any ESP32 board with WiFi/BLE |
| **JSN-SR04T** | Waterproof ultrasonic sensor | 1-2 | 5V operation, range: 25-450cm |
| **OLED Display** | SSD1306 0.96" I2C | 1 | 128x64 resolution |
| **Relay Module** | 5V/3.3V compatible | 1 | For pump control |
| **Voltage Divider** | 10kΩ + 20kΩ resistors | 1-2 | For JSN-SR04T Echo pins |
| **Power Supply** | 5V 2A | 1 | For ESP32 and sensors |
| **Breadboard/PCB** | - | 1 | For connections |
| **Jumper Wires** | - | Various | For wiring |

### Optional Components
- Enclosure (weatherproof if outdoor installation)
- Additional power supply for pump
- Status LED (uses built-in LED by default)

---

## 🔌 Wiring Diagram

### ESP32 Default Pin Assignments

```
ESP32 GPIO Pins:
├── Tank 1 Sensor
│   ├── Trigger → GPIO 25
│   └── Echo    → GPIO 26 (via voltage divider!)
│
├── Tank 2 Sensor (Dual mode only)
│   ├── Trigger → GPIO 32
│   └── Echo    → GPIO 33 (via voltage divider!)
│
├── OLED Display (I2C)
│   ├── SDA     → GPIO 21
│   └── SCL     → GPIO 22
│
├── Pump Relay  → GPIO 27
└── Status LED  → GPIO 2 (built-in)
```

### JSN-SR04T Sensor Wiring

**⚠️ IMPORTANT: JSN-SR04T uses 5V logic on Echo pin!**

```
JSN-SR04T → ESP32
├── VCC (Red)    → 5V
├── GND (Black)  → GND
├── TRIG (White) → GPIO 25 (direct connection OK)
└── ECHO (Yellow)→ GPIO 26 (MUST use voltage divider!)

Voltage Divider for Echo Pin:
    Echo Pin (5V) ──┬─── 20kΩ resistor ─── ESP32 GPIO
                    │
                    └─── 10kΩ resistor ─── GND
    
    Output voltage = 5V × (10kΩ / 30kΩ) = 1.67V ✓ (Safe for 3.3V ESP32)
```

### OLED Display Wiring

```
SSD1306 OLED → ESP32
├── VCC → 3.3V
├── GND → GND
├── SDA → GPIO 21
└── SCL → GPIO 22
```

### Pump Relay Wiring

```
Relay Module → ESP32
├── VCC → 5V
├── GND → GND
└── IN  → GPIO 27

Relay Output → Water Pump
├── COM → Pump power supply (+)
├── NO  → Pump positive terminal
└── Pump negative terminal → Power supply (-)
```

### Complete Wiring Diagram

```
                           ┌───────────────┐
                           │   ESP32       │
                           │   DevKit      │
                           ├───────────────┤
         ┌─────────────────┤ 5V            │
         │      ┌──────────┤ 3.3V          │
         │      │    ┌─────┤ GND           │
         │      │    │     │               │
         │      │    │     │ GPIO 25 ◄─────┼─── TRIG (Tank 1)
         │      │    │     │ GPIO 26 ◄─────┼─── ECHO (Tank 1) via divider
         │      │    │     │ GPIO 32 ◄─────┼─── TRIG (Tank 2)
         │      │    │     │ GPIO 33 ◄─────┼─── ECHO (Tank 2) via divider
         │      │    │     │ GPIO 21 ◄─────┼─── SDA (OLED)
         │      │    │     │ GPIO 22 ◄─────┼─── SCL (OLED)
         │      │    │     │ GPIO 27 ───►─┼─── Relay IN
         │      │    │     │ GPIO 2  ───►─┼─── LED
         │      │    │     └───────────────┘
         │      │    │
         │      │    └───┬───┬───────────────┐
         │      │        │   │               │
         │      │      ┌─┴─┐ │         ┌─────┴─────┐
         │      │      │GND│ │         │  JSN-SR04T│
         │      │      └───┘ │         │  Sensor 1 │
         │      │            │         ├───────────┤
         │      └────────────┼─────────┤ VCC       │
         │                   │         │ GND       │
         │                   └─────────┤ TRIG      │
         │                             │ ECHO      │
         │                             └───────────┘
         │
         │                   ┌───────────┐
         │                   │ Relay     │
         │                   │ Module    │
         │                   ├───────────┤
         └───────────────────┤ VCC       │
                             │ GND       │
                             │ IN        │
                             └───────────┘
```

---

## 🚀 Getting Started

### Prerequisites

1. **PlatformIO IDE** (VS Code extension) or PlatformIO Core CLI
2. **USB Cable** for ESP32 programming
3. **Driver** for ESP32 USB-to-Serial chip (CP2102/CH340)

### Installation Steps

1. **Clone or download this repository**
   ```bash
   cd water-level-monitor
   ```

2. **Open in PlatformIO**
   - Open VS Code
   - File → Open Folder → Select `water-level` directory
   - PlatformIO will automatically detect the project

3. **Install Dependencies**
   ```bash
   pio lib install
   ```
   Libraries will be automatically installed from `platformio.ini`

4. **Connect ESP32 via USB**

5. **Build and Upload**
   ```bash
   # Build project
   pio run
   
   # Upload to ESP32
   pio run --target upload
   
   # Monitor serial output
   pio device monitor
   ```

---

## 🔐 First-Time Setup

### Step 1: Initial Boot

On first boot, the device will:
1. Initialize with factory defaults
2. Start WiFi Access Point mode
3. Display configuration portal info on OLED

### Step 2: Connect to Configuration Portal

1. **Connect to WiFi Network:**
   - SSID: `WaterMonitor_Setup`
   - Password: `12345678`

2. **Open Web Browser:**
   - Navigate to: `http://192.168.4.1`
   - Or any website (captive portal will redirect)

### Step 3: Configure Device

The web interface provides tabs for configuration:

#### **WiFi Configuration**
- Enter your WiFi SSID
- Enter WiFi password (minimum 8 characters)
- Click "Scan" to see available networks

#### **Tank Configuration**
- Select mode: Single Tank or Dual Tank
- Set tank dimensions:
  - **Empty Distance**: Distance from sensor to tank bottom (cm)
  - **Full Distance**: Distance from sensor to water surface when full (cm)
  - Example: If sensor is mounted 200cm above tank bottom and water surface is 10cm below sensor when full:
    - Empty = 200cm
    - Full = 10cm

#### **MQTT Configuration**
- Broker address (e.g., `mqtt.myserver.com` or IP address)
- Port (default: 1883)
- Username (optional)
- Password (optional)
- Topic (default: `water/level`)

#### **Pump Configuration**
- Mode: Manual, Automatic, or Scheduled
- Auto-start threshold (e.g., 20%)
- Auto-stop threshold (e.g., 90%)

### Step 4: Save and Reboot

1. Click "Save Configuration"
2. Device will reboot automatically
3. Connect to configured WiFi network
4. Access web interface via IP address shown on OLED

---

## ⚙️ Configuration

### Configuration via Web Interface

Access the web interface at device IP address:

```
http://[device-ip]/
```

### Configuration via Serial

Connect via USB and use serial monitor (115200 baud) to view configuration:

```
========== Current Configuration ==========
Device ID: WaterMonitor_12345678
Tank Mode: Single
Unit System: Metric (cm)
Tank 1: Tank 1 (Empty: 200.0 cm, Full: 10.0 cm)
WiFi: MyNetwork (configured)
MQTT: mqtt.example.com:1883
Pump Mode: Automatic
==========================================
```

### Calibration Guide

#### Method 1: Measurement

1. Measure total tank depth from sensor mounting point to bottom
2. Fill tank to desired "full" level
3. Measure distance from sensor to water surface
4. Enter values in web interface

#### Method 2: Live Calibration

1. Monitor live sensor readings in web interface
2. Note "Empty" reading (tank empty)
3. Fill tank to desired level
4. Note "Full" reading (tank full)
5. Enter calibrated values

### Factory Reset

To reset device to factory defaults:

1. **Via Web Interface:** System → Factory Reset
2. **Via Serial:** Not implemented (use web interface)
3. **Manual:** Erase flash:
   ```bash
   pio run --target erase
   pio run --target upload
   ```

---

## 📱 Usage

### Reading Water Levels

#### On OLED Display

**Single Tank Mode:**
```
Tank 1
  87%
[████████████░░] 
  18.4 cm
W M B ✓
```

**Dual Tank Mode:**
```
Tank 1    │ Tank 2
  87%     │   45%
[████░░]  │ [██░░]
```

Status Icons:
- `W` = WiFi connected
- `M` = MQTT connected
- `B` = BLE active

#### Via Web Interface

Navigate to `http://[device-ip]/` for real-time dashboard with:
- Large percentage display
- Visual progress bars
- Connection status
- Pump controls
- Historical data (if implemented)

#### Via MQTT

Subscribe to MQTT topic (default: `water/level`):

```json
{
  "device_id": "WaterMonitor_12345678",
  "timestamp": 1678901234,
  "tank_mode": "single",
  "tank1": {
    "name": "Main Tank",
    "level_percent": 87.2,
    "distance_cm": 18.4,
    "valid": true
  }
}
```

#### Via BLE

Use BLE scanner app (e.g., nRF Connect):

1. Scan for devices
2. Connect to `WaterMonitor_[ID]`
3. Read characteristics:
   - Tank 1 Level: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
   - Tank 2 Level: `beb5483e-36e1-4688-b7f5-ea07361b26a9`
   - Pump Status: `beb5483e-36e1-4688-b7f5-ea07361b26aa`

### Pump Control

#### Manual Control

**Web Interface:**
1. Navigate to Status tab
2. Click "Turn On" or "Turn Off"

**MQTT:**
Publish to command topic (default: `water/command`):
```json
{"command": "pump_on"}
{"command": "pump_off"}
```

**BLE:**
Write to Pump characteristic:
- `1` = Turn on
- `0` = Turn off

#### Automatic Control

Configure thresholds in web interface:
- Pump starts when level ≤ ON threshold (e.g., 20%)
- Pump stops when level ≥ OFF threshold (e.g., 90%)

Safety features:
- Minimum run time: 10 seconds
- Maximum run time: 60 minutes
- Cooldown period: 1 minute
- Dry-run protection: stops if source tank < 5%

---

## 🌐 API Documentation

### REST API Endpoints

Base URL: `http://[device-ip]`

#### GET /api/status

Returns current system status.

**Response:**
```json
{
  "tankMode": 0,
  "wifi": true,
  "mqtt": true,
  "ble": true,
  "pump": false,
  "tank1": {
    "name": "Tank 1",
    "level": 87.2,
    "distance": 18.4,
    "valid": true
  }
}
```

#### GET /api/config

Returns current configuration.

**Response:**
```json
{
  "tankMode": 0,
  "wifiSSID": "MyNetwork",
  "mqttBroker": "mqtt.example.com",
  "mqttPort": 1883,
  "tank1Empty": 200.0,
  "tank1Full": 10.0,
  "pumpMode": 1
}
```

#### POST /api/config

Update configuration.

**Request Body:**
```json
{
  "wifiSSID": "NewNetwork",
  "wifiPassword": "NewPassword123",
  "mqttBroker": "mqtt.myserver.com",
  "tank1Empty": 180.0,
  "tank1Full": 15.0
}
```

#### POST /api/pump

Control pump.

**Request:**
```json
{"action": "on"}   // or "off"
```

#### POST /api/restart

Restart device.

#### POST /api/reset

Factory reset (erase all configuration).

#### GET /api/wifi/scan

Scan for WiFi networks.

**Response:**
```json
{
  "networks": [
    {"ssid": "Network1", "rssi": -45, "encryption": "encrypted"},
    {"ssid": "Network2", "rssi": -67, "encryption": "open"}
  ]
}
```

---

## 📡 MQTT Integration

### Connection Parameters

Configure via web interface:
- **Broker:** Hostname or IP address
- **Port:** Default 1883 (1883 = unencrypted, 8883 = TLS)
- **Username/Password:** Optional authentication
- **Client ID:** Automatically set to device ID

### Topics

Default topics (configurable):
- **Publish:** `water/level` - Sensor readings (every 10 seconds)
- **Publish:** `water/level/status` - System status
- **Subscribe:** `water/command` - Control commands

### Payload Format

**Sensor Data (Published):**
```json
{
  "device_id": "WaterMonitor_12345678",
  "timestamp": 1678901234,
  "tank_mode": "single",
  "tank1": {
    "name": "Main Tank",
    "level_percent": 87.2,
    "distance_cm": 18.4,
    "valid": true
  }
}
```

**Commands (Subscribe):**
```json
{"command": "pump_on"}       // Turn pump on
{"command": "pump_off"}      // Turn pump off
{"command": "status"}        // Request status update
```

### Home Assistant Integration

Add to `configuration.yaml`:

```yaml
mqtt:
  sensor:
    - name: "Water Tank Level"
      state_topic: "water/level"
      unit_of_measurement: "%"
      value_template: "{{ value_json.tank1.level_percent }}"
      
    - name: "Water Tank Distance"
      state_topic: "water/level"
      unit_of_measurement: "cm"
      value_template: "{{ value_json.tank1.distance_cm }}"
      
  switch:
    - name: "Water Pump"
      command_topic: "water/command"
      state_topic: "water/level/status"
      payload_on: '{"command":"pump_on"}'
      payload_off: '{"command":"pump_off"}'
      value_template: "{{ value_json.pump }}"
```

---

## 📶 BLE Integration

### Service UUID

```
Service: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
```

### Characteristics

| Characteristic | UUID | Properties | Description |
|---------------|------|------------|-------------|
| Tank 1 Level | `beb5483e-36e1-4688-b7f5-ea07361b26a8` | Read, Notify | Tank 1 level (0-100) |
| Tank 2 Level | `beb5483e-36e1-4688-b7f5-ea07361b26a9` | Read, Notify | Tank 2 level (0-100) |
| Pump Status | `beb5483e-36e1-4688-b7f5-ea07361b26aa` | Read, Write | Pump on/off (0/1) |
| Configuration | `beb5483e-36e1-4688-b7f5-ea07361b26ab` | Write | Config updates |

### Reading with nRF Connect

1. Install nRF Connect app (iOS/Android)
2. Scan for devices
3. Connect to `WaterMonitor_[ID]`
4. Expand service `4fafc...`
5. Read or subscribe to characteristics

---

## 🔄 OTA Updates

### Over-the-Air Firmware Update

#### Method 1: Web Interface

1. Navigate to System tab
2. Click "Choose File"
3. Select `.bin` firmware file
4. Click "Upload Firmware"
5. Wait for upload and reboot

#### Method 2: Arduino OTA (PlatformIO)

```bash
# Upload via network
pio run --target upload --upload-port [device-ip]
```

Default OTA credentials:
- **Hostname:** `waterlevel-monitor`
- **Password:** `admin123` (change in `config.h`)

⚠️ **Security Note:** Change OTA password in production!

---

## 🐛 Troubleshooting

### Sensor Issues

**Problem:** Sensor readings invalid or erratic

**Solutions:**
1. Check wiring (especially voltage divider)
2. Verify sensor power (5V)
3. Ensure clear path (no obstacles)
4. Check sensor distance limits (25-450cm for JSN-SR04T)
5. Increase sample count in code if noisy environment

**Problem:** Distance readings out of range

**Solutions:**
1. Recalibrate tank dimensions
2. Check sensor mounting (must be vertical, level)
3. Verify water surface is within sensor range

### WiFi Issues

**Problem:** Cannot connect to captive portal

**Solutions:**
1. Verify AP SSID: `WaterMonitor_Setup`
2. Check password: `12345678`
3. Manually navigate to `192.168.4.1`
4. Reset device and try again

**Problem:** WiFi connection drops frequently

**Solutions:**
1. Move closer to router
2. Check WiFi signal strength (RSSI in web interface)
3. Use 2.4GHz network (ESP32 doesn't support 5GHz)
4. Disable WiFi power saving in code

### MQTT Issues

**Problem:** MQTT not connecting

**Solutions:**
1. Verify broker address and port
2. Check network connectivity
3. Test broker with MQTT client (e.g., MQTT Explorer)
4. Check username/password if authentication enabled
5. Review serial output for error messages

### Display Issues

**Problem:** OLED display blank

**Solutions:**
1. Check I2C wiring (SDA/SCL)
2. Verify display address (0x3C or 0x3D)
3. Check power (3.3V, not 5V!)
4. Enable display in configuration

### Pump Issues

**Problem:** Pump won't turn on

**Solutions:**
1. Check manual control works (web interface)
2. Verify relay wiring
3. Check pump mode (manual vs automatic)
4. Review thresholds (may not be met)
5. Check cooldown period
6. Review error message in logs

**Problem:** Pump runs too long

**Solutions:**
1. Check OFF threshold is being reached
2. Verify sensor readings are accurate
3. Reduce max run time in configuration

### General Debugging

**Enable Debug Output:**

Edit `config.h`:
```cpp
#define DEBUG_SERIAL    true
#define DEBUG_SENSOR    true
#define DEBUG_WIFI      true
#define DEBUG_MQTT      true
#define DEBUG_BLE       true
```

**View Serial Monitor:**
```bash
pio device monitor --baud 115200
```

**Check Logs:**
- Boot sequence
- WiFi connection attempts
- MQTT connection status
- Sensor readings
- Error messages

---

## 📄 License

This project is open-source and available under the MIT License.

---

## 🤝 Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Test your changes
4. Submit a pull request

---

## 📞 Support

For issues, questions, or suggestions:
- Open an issue on GitHub
- Check troubleshooting section above
- Review serial debug output

---

## 🎯 Roadmap

### Priority 1: Offline Configuration Console 🎮

**Physical Button Interface for Field Setup**

Perfect for installations without WiFi access or when web configuration isn't practical.

**Hardware:**
- 4-6 tactile push buttons
- Optional rotary encoder
- Uses existing OLED display

**Features:**
- Complete device setup without WiFi
- Guided calibration wizard
- Menu-driven navigation
- Real-time sensor preview
- All settings accessible
- Save changes immediately to NVS

**Button Layout:**
```
[UP]     - Navigate up / Increment value
[DOWN]   - Navigate down / Decrement value  
[SELECT] - Confirm / Enter submenu
[BACK]   - Cancel / Return to previous
[MENU]   - Quick access (long-press)
```

**Menu Structure:**
```
Main Menu
├── Tank Configuration (mode, calibration, names)
├── WiFi Setup (SSID selection, password entry)
├── MQTT Settings (enable, broker, topics)
├── Pump Control (mode, thresholds, safety)
├── Display Settings (brightness, timeout)
├── System (status, factory reset, restart)
└── Calibration Wizard (step-by-step guided setup)
```

**Implementation:**
- Non-blocking FreeRTOS task
- Interrupt-based button handling
- Debouncing and long-press detection
- Menu timeout with auto-return to status
- T9-style text entry or character wheel
- Visual feedback on OLED
- Confirmation dialogs for destructive actions

**Use Cases:**
- Initial setup in remote locations
- Field recalibration without laptop
- Backup configuration if WiFi fails
- Quick threshold adjustments
- Emergency system control

**GPIO Pin Assignments:**
```
Default Pins:
- UP Button:     GPIO 13
- DOWN Button:   GPIO 14
- SELECT Button: GPIO 15
- BACK Button:   GPIO 16
- MENU Button:   GPIO 17

Optional Rotary Encoder:
- CLK Pin:  GPIO 18
- DT Pin:   GPIO 19
- SW Pin:   GPIO 15 (shared with SELECT)
```

---

### Other Future Features

- [ ] Blynk cloud integration
- [ ] Home Assistant MQTT discovery
- [ ] Pressure sensor support (submerged applications)
- [ ] Data logging to SD card with timestamps
- [ ] Email/SMS alerts via IFTTT webhooks
- [ ] Multi-language web interface
- [ ] Mobile companion app (iOS/Android)
- [ ] Battery voltage monitoring
- [ ] Solar panel integration with MPPT
- [ ] Historical data graphs (24hr/7day/30day)
- [ ] Advanced analytics and water usage predictions
- [ ] Voice assistant integration (Alexa/Google Home)
- [ ] Modbus RTU support for industrial PLCs
- [ ] LoRaWAN connectivity option
- [ ] Multiple device coordination

---

**Built with ❤️ for the maker community**


