#!/bin/bash
#
# Publish IonConnect and Build Water-Level Project
#

set -e

echo "╔════════════════════════════════════════════════════════╗"
echo "║  Publish IonConnect & Build Water-Level Project       ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Step 1: Publish IonConnect
echo -e "${BLUE}Step 1: Publishing IonConnect to PlatformIO...${NC}"
echo ""

cd /Users/faisalsideline/Desktop/Data/repos/vtoxi/IonConnect

echo "Current directory: $(pwd)"
echo "Library version: $(grep '"version"' library.json | head -1)"
echo ""

# Check if logged in
if ! pio account show &>/dev/null; then
    echo -e "${YELLOW}⚠️  Not logged in to PlatformIO${NC}"
    echo "Please run: pio account login"
    echo ""
    read -p "Press Enter after logging in, or Ctrl+C to cancel..."
fi

echo -e "${YELLOW}Publishing library...${NC}"
if pio pkg publish --type=library --owner=vtoxi; then
    echo ""
    echo -e "${GREEN}✅ IonConnect published successfully!${NC}"
    echo ""
    echo "View at: https://registry.platformio.org/libraries/vtoxi/IonConnect"
    echo ""
else
    echo ""
    echo -e "${RED}❌ Publishing failed!${NC}"
    echo "Check error messages above."
    echo ""
    echo "Common issues:"
    echo "  • Not logged in: pio account login"
    echo "  • Already published: Library might already be in registry"
    echo "  • Network error: Check internet connection"
    echo ""
    exit 1
fi

echo -e "${YELLOW}Waiting 5 seconds for registry to update...${NC}"
sleep 5

# Step 2: Build Water-Level Project
echo ""
echo -e "${BLUE}Step 2: Building Water-Level Project...${NC}"
echo ""

cd /Users/faisalsideline/Desktop/Data/repos/vtoxi/water-level

echo "Current directory: $(pwd)"
echo ""

# Clean
echo -e "${YELLOW}Cleaning build directory...${NC}"
pio run -t clean > /dev/null 2>&1

# Remove cached libraries
echo -e "${YELLOW}Removing cached libraries...${NC}"
rm -rf .pio/libdeps

# Install dependencies
echo -e "${YELLOW}Installing dependencies (downloading IonConnect)...${NC}"
if pio pkg install; then
    echo -e "${GREEN}✅ Dependencies installed${NC}"
else
    echo -e "${RED}❌ Failed to install dependencies${NC}"
    exit 1
fi

echo ""

# Verify IonConnect installed
echo -e "${YELLOW}Verifying IonConnect installation...${NC}"
if pio pkg list | grep -q "IonConnect"; then
    VERSION=$(pio pkg list | grep "IonConnect" | awk '{print $NF}')
    echo -e "${GREEN}✅ IonConnect installed: $VERSION${NC}"
else
    echo -e "${RED}❌ IonConnect not found in dependencies${NC}"
    echo "This might mean:"
    echo "  • Registry hasn't updated yet (wait a few minutes)"
    echo "  • Library wasn't published successfully"
    echo ""
    exit 1
fi

echo ""

# Build ESP32
echo -e "${BLUE}Building ESP32...${NC}"
if pio run -e esp32dev; then
    echo -e "${GREEN}✅ ESP32 build successful${NC}"
    ESP32_SIZE=$(ls -lh .pio/build/esp32dev/firmware.bin 2>/dev/null | awk '{print $5}')
    echo "   Firmware size: $ESP32_SIZE"
else
    echo -e "${RED}❌ ESP32 build failed${NC}"
    exit 1
fi

echo ""

# Build ESP32-S2 (optional)
echo -e "${BLUE}Building ESP32-S2...${NC}"
if pio run -e esp32s2dev; then
    echo -e "${GREEN}✅ ESP32-S2 build successful${NC}"
    ESP32S2_SIZE=$(ls -lh .pio/build/esp32s2dev/firmware.bin 2>/dev/null | awk '{print $5}')
    echo "   Firmware size: $ESP32S2_SIZE"
else
    echo -e "${YELLOW}⚠️  ESP32-S2 build failed (optional)${NC}"
fi

echo ""

# Build ESP8266 (optional)
echo -e "${BLUE}Building ESP8266...${NC}"
if pio run -e d1_mini_pro; then
    echo -e "${GREEN}✅ ESP8266 build successful${NC}"
    ESP8266_SIZE=$(ls -lh .pio/build/d1_mini_pro/firmware.bin 2>/dev/null | awk '{print $5}')
    echo "   Firmware size: $ESP8266_SIZE"
else
    echo -e "${YELLOW}⚠️  ESP8266 build failed (optional)${NC}"
fi

echo ""
echo "╔════════════════════════════════════════════════════════╗"
echo -e "║  ${GREEN}✅ SUCCESS! Everything Complete!${NC}                    ║"
echo "╚════════════════════════════════════════════════════════╝"
echo ""

echo -e "${GREEN}IonConnect published:${NC}"
echo "  https://registry.platformio.org/libraries/vtoxi/IonConnect"
echo ""

echo -e "${GREEN}Water-level project built successfully!${NC}"
echo "  ESP32:     ✅ $ESP32_SIZE"
echo "  ESP32-S2:  ${ESP32S2_SIZE:-'N/A'}"
echo "  ESP8266:   ${ESP8266_SIZE:-'N/A'}"
echo ""

echo -e "${BLUE}Next steps:${NC}"
echo "  1. Upload firmware:"
echo "     pio run -t upload -e esp32dev"
echo ""
echo "  2. Monitor serial output:"
echo "     pio device monitor -b 115200"
echo ""
echo "  3. Test IonConnect captive portal:"
echo "     • Connect to 'IonConnect-XXXX' WiFi"
echo "     • Browser opens to http://192.168.4.1"
echo "     • Configure WiFi credentials"
echo ""

echo -e "${GREEN}All done! 🎉${NC}"
echo ""

