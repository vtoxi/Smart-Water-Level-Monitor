#!/bin/bash
#
# Complete verification and publishing workflow
#

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  IonConnect Integration Workflow${NC}"
echo -e "${BLUE}========================================${NC}\n"

# Step 1: Verify IonConnect library structure
echo -e "${CYAN}Step 1: Verifying IonConnect library...${NC}"
ION_DIR="/Users/faisalsideline/Desktop/Data/repos/vtoxi/IonConnect"

if [ ! -d "$ION_DIR" ]; then
    echo -e "${RED}✗ Error: IonConnect directory not found${NC}"
    exit 1
fi

if [ ! -f "$ION_DIR/library.json" ]; then
    echo -e "${RED}✗ Error: library.json not found${NC}"
    exit 1
fi

if [ ! -f "$ION_DIR/library.properties" ]; then
    echo -e "${RED}✗ Error: library.properties not found${NC}"
    exit 1
fi

if [ ! -f "$ION_DIR/src/IonConnect.h" ]; then
    echo -e "${RED}✗ Error: IonConnect.h not found${NC}"
    exit 1
fi

echo -e "${GREEN}✓ IonConnect library structure valid${NC}\n"

# Step 2: Build water-level project with local IonConnect
echo -e "${CYAN}Step 2: Building water-level with local IonConnect...${NC}"
cd /Users/faisalsideline/Desktop/Data/repos/vtoxi/water-level

echo -e "${YELLOW}  Building ESP32...${NC}"
if pio run -e esp32dev > build_esp32.log 2>&1; then
    echo -e "${GREEN}  ✓ ESP32 build successful${NC}"
    ESP32_SUCCESS=1
else
    echo -e "${RED}  ✗ ESP32 build failed - see build_esp32.log${NC}"
    ESP32_SUCCESS=0
fi

echo -e "${YELLOW}  Building ESP32-S2...${NC}"
if pio run -e esp32s2dev > build_esp32s2.log 2>&1; then
    echo -e "${GREEN}  ✓ ESP32-S2 build successful${NC}"
    ESP32S2_SUCCESS=1
else
    echo -e "${RED}  ✗ ESP32-S2 build failed - see build_esp32s2.log${NC}"
    ESP32S2_SUCCESS=0
fi

echo -e "${YELLOW}  Building ESP8266...${NC}"
if pio run -e d1_mini_pro > build_esp8266.log 2>&1; then
    echo -e "${GREEN}  ✓ ESP8266 build successful${NC}"
    ESP8266_SUCCESS=1
else
    echo -e "${RED}  ✗ ESP8266 build failed - see build_esp8266.log${NC}"
    ESP8266_SUCCESS=0
fi

echo ""

# Check if all builds succeeded
TOTAL_SUCCESS=$((ESP32_SUCCESS + ESP32S2_SUCCESS + ESP8266_SUCCESS))
if [ $TOTAL_SUCCESS -eq 3 ]; then
    echo -e "${GREEN}✓ All builds successful!${NC}\n"
else
    echo -e "${RED}✗ Some builds failed. Check log files for details.${NC}"
    echo -e "${YELLOW}Do you want to continue with publishing anyway? (y/n)${NC}"
    read -r response
    if [[ ! "$response" =~ ^[Yy]$ ]]; then
        echo -e "${RED}Aborting.${NC}"
        exit 1
    fi
fi

# Step 3: Show build sizes
echo -e "${CYAN}Step 3: Build Statistics${NC}"
echo -e "${YELLOW}ESP32:${NC}"
if [ -f ".pio/build/esp32dev/firmware.bin" ]; then
    SIZE=$(ls -lh .pio/build/esp32dev/firmware.bin | awk '{print $5}')
    echo -e "  Firmware size: ${GREEN}$SIZE${NC}"
fi

echo -e "${YELLOW}ESP32-S2:${NC}"
if [ -f ".pio/build/esp32s2dev/firmware.bin" ]; then
    SIZE=$(ls -lh .pio/build/esp32s2dev/firmware.bin | awk '{print $5}')
    echo -e "  Firmware size: ${GREEN}$SIZE${NC}"
fi

echo -e "${YELLOW}ESP8266:${NC}"
if [ -f ".pio/build/d1_mini_pro/firmware.bin" ]; then
    SIZE=$(ls -lh .pio/build/d1_mini_pro/firmware.bin | awk '{print $5}')
    echo -e "  Firmware size: ${GREEN}$SIZE${NC}"
fi

echo ""

# Step 4: Publish IonConnect to PlatformIO
echo -e "${CYAN}Step 4: Publishing IonConnect to PlatformIO...${NC}"
echo -e "${YELLOW}Ready to publish IonConnect library to PlatformIO registry.${NC}"
echo -e "${YELLOW}Continue? (y/n)${NC}"
read -r response
if [[ ! "$response" =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}Skipping publication. Stopping here.${NC}"
    exit 0
fi

cd "$ION_DIR"

echo -e "${YELLOW}Publishing to PlatformIO...${NC}"
if bash scripts/publish_platformio.sh; then
    echo -e "${GREEN}✓ IonConnect published to PlatformIO!${NC}\n"
else
    echo -e "${RED}✗ Publication failed${NC}"
    exit 1
fi

# Step 5: Update water-level to use published library
echo -e "${CYAN}Step 5: Updating water-level to use published library...${NC}"
cd /Users/faisalsideline/Desktop/Data/repos/vtoxi/water-level

# Backup current platformio.ini
cp platformio.ini platformio.ini.backup

# Update platformio.ini
echo -e "${YELLOW}Updating platformio.ini...${NC}"

# Remove lib_extra_dirs line
sed -i.tmp '/lib_extra_dirs = \.\.\/IonConnect/d' platformio.ini
sed -i.tmp '/lib_extra_dirs = \${common\.lib_extra_dirs}/d' platformio.ini

# Add IonConnect to lib_deps_core
if ! grep -q "vtoxi/IonConnect" platformio.ini; then
    sed -i.tmp '/lib_deps_core =/a\
    vtoxi/IonConnect@^1.0.2
' platformio.ini
fi

rm -f platformio.ini.tmp

echo -e "${GREEN}✓ platformio.ini updated${NC}\n"

# Step 6: Clean and rebuild with published library
echo -e "${CYAN}Step 6: Rebuilding with published library...${NC}"
echo -e "${YELLOW}Cleaning build cache...${NC}"
pio run -t clean > /dev/null 2>&1

echo -e "${YELLOW}Installing dependencies...${NC}"
pio pkg install

echo -e "${YELLOW}Building ESP32 with published library...${NC}"
if pio run -e esp32dev > build_published_esp32.log 2>&1; then
    echo -e "${GREEN}✓ ESP32 build successful with published library!${NC}"
else
    echo -e "${RED}✗ ESP32 build failed with published library${NC}"
    echo -e "${YELLOW}Restoring backup...${NC}"
    mv platformio.ini.backup platformio.ini
    exit 1
fi

echo -e "${YELLOW}Building ESP32-S2 with published library...${NC}"
if pio run -e esp32s2dev > build_published_esp32s2.log 2>&1; then
    echo -e "${GREEN}✓ ESP32-S2 build successful with published library!${NC}"
else
    echo -e "${RED}✗ ESP32-S2 build failed with published library${NC}"
    echo -e "${YELLOW}Restoring backup...${NC}"
    mv platformio.ini.backup platformio.ini
    exit 1
fi

echo -e "${YELLOW}Building ESP8266 with published library...${NC}"
if pio run -e d1_mini_pro > build_published_esp8266.log 2>&1; then
    echo -e "${GREEN}✓ ESP8266 build successful with published library!${NC}"
else
    echo -e "${RED}✗ ESP8266 build failed with published library${NC}"
    echo -e "${YELLOW}Restoring backup...${NC}"
    mv platformio.ini.backup platformio.ini
    exit 1
fi

echo ""
echo -e "${GREEN}✓ All builds successful with published library!${NC}\n"

# Clean up backup
rm -f platformio.ini.backup

echo -e "${BLUE}========================================${NC}"
echo -e "${GREEN}     🎉 SUCCESS! 🎉${NC}"
echo -e "${BLUE}========================================${NC}\n"

echo -e "${GREEN}✓ IonConnect library published to PlatformIO${NC}"
echo -e "${GREEN}✓ Water-level project updated to use published version${NC}"
echo -e "${GREEN}✓ All boards build successfully${NC}\n"

echo -e "${CYAN}Library URL:${NC}"
echo -e "${YELLOW}https://registry.platformio.org/libraries/vtoxi/IonConnect${NC}\n"

echo -e "${CYAN}Next steps:${NC}"
echo -e "1. Test firmware on hardware"
echo -e "2. Verify IonConnect captive portal works"
echo -e "3. Commit changes to git"
echo -e "4. Create release tag\n"

echo -e "${CYAN}To upload firmware:${NC}"
echo -e "${YELLOW}pio run -t upload -e esp32dev${NC}"
echo -e "${YELLOW}pio device monitor -b 115200${NC}\n"

echo -e "${GREEN}All done! 🚀${NC}\n"

