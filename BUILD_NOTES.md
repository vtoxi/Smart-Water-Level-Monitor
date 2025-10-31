# Build Notes

## Recent Changes

### IonConnect Integration Complete ✅

**Library:** Using published [coderunner/IonConnect@^1.0.2](https://registry.platformio.org/libraries/coderunner/IonConnect)

**Files:**
- ✅ Added: `src/wifi_ionconnect.h` - IonConnect wrapper with ConfigManager integration
- ✅ Added: `src/wifi_ionconnect.cpp` - Implementation with event callbacks
- ✅ Removed: `src/wifi_manager.h` - Old WiFi manager (replaced)
- ✅ Removed: `src/wifi_manager.cpp` - Old WiFi manager (replaced)
- ✅ Updated: `platformio.ini` - Uses published library `coderunner/IonConnect@^1.0.2`
- ✅ Fixed: `config.h` - Renamed `DEBUG_WIFI` to `DEBUG_WIFI_CONN` (conflict with ESP8266WiFi.h)
- ✅ Fixed: `wifi_ionconnect.h` - Include order corrected (local headers before IonConnect.h)
- ✅ Cleaned: Build cache cleared to remove old wifi_manager artifacts

## Build Commands

```bash
# ESP32
pio run -e esp32dev

# ESP32-S2  
pio run -e esp32s2dev

# ESP8266
pio run -e d1_mini_pro

# Upload
pio run -t upload -e esp32dev

# Monitor
pio device monitor -b 115200
```

## Known Warnings (Non-Critical)

- `DEBUG_WIFI_CONN` redefinition warning - Fixed by renaming macro
- Unused variable warnings - Non-critical, can be cleaned up later

## Library Dependencies

- IonConnect @ 1.0.2 (WiFi provisioning)
- Adafruit GFX Library @ 1.11.9
- Adafruit SSD1306 @ 2.5.9
- PubSubClient @ 2.8
- ArduinoJson @ 6.21.3
- ESPAsyncWebServer @ 1.2.3
- AsyncTCP @ 1.1.1 (ESP32)
- ESPAsyncTCP @ 1.2.2 (ESP8266)

## Project Status

✅ **Ready to build and deploy**

All configurations complete. IonConnect library is published and integrated.

