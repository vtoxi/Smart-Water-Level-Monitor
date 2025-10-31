#!/bin/bash

echo "======================================"
echo "Building Water Level Monitor"
echo "======================================"
echo ""

cd /Users/faisalsideline/Desktop/Data/repos/vtoxi/water-level

echo "Building for ESP32..."
pio run -e esp32dev 2>&1 | tee build_esp32.log
ESP32_RESULT=$?

echo ""
echo "Building for ESP32-S2..."
pio run -e esp32s2dev 2>&1 | tee build_esp32s2.log
ESP32S2_RESULT=$?

echo ""
echo "Building for ESP8266..."
pio run -e d1_mini_pro 2>&1 | tee build_esp8266.log
ESP8266_RESULT=$?

echo ""
echo "======================================"
echo "Build Summary"
echo "======================================"
echo "ESP32:     $([ $ESP32_RESULT -eq 0 ] && echo '✓ SUCCESS' || echo '✗ FAILED')"
echo "ESP32-S2:  $([ $ESP32S2_RESULT -eq 0 ] && echo '✓ SUCCESS' || echo '✗ FAILED')"
echo "ESP8266:   $([ $ESP8266_RESULT -eq 0 ] && echo '✓ SUCCESS' || echo '✗ FAILED')"
echo "======================================"

exit $(( ESP32_RESULT + ESP32S2_RESULT + ESP8266_RESULT ))

