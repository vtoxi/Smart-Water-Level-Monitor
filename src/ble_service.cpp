#include "ble_service.h"
#include "pump_controller.h"
#include "config.h"

BLEService::BLEService(ConfigManager& configManager)
    : configManager(configManager),
      pumpController(nullptr),
      pServer(nullptr),
      pService(nullptr),
      pTank1Char(nullptr),
      pTank2Char(nullptr),
      pPumpChar(nullptr),
      pConfigChar(nullptr),
      running(false),
      deviceConnected(false),
      oldDeviceConnected(false) {
}

bool BLEService::begin() {
    const SystemConfig& config = configManager.getConfig();
    
    DEBUG_PRINTLN("BLE: Initializing...");
    
    // Create BLE device
    BLEDevice::init(config.deviceId);
    
    // Create BLE server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(this));
    
    // Create BLE service
    pService = pServer->createService(BLE_SERVICE_UUID);
    
    // Tank 1 Level characteristic (read + notify)
    pTank1Char = pService->createCharacteristic(
        BLE_TANK1_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pTank1Char->addDescriptor(new BLE2902());
    pTank1Char->setValue("0");
    
    // Tank 2 Level characteristic (read + notify)
    pTank2Char = pService->createCharacteristic(
        BLE_TANK2_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pTank2Char->addDescriptor(new BLE2902());
    pTank2Char->setValue("0");
    
    // Pump Status characteristic (read + write)
    pPumpChar = pService->createCharacteristic(
        BLE_PUMP_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    pPumpChar->setCallbacks(new PumpCallbacks(this));
    pPumpChar->setValue("0");
    
    // Config characteristic (write only)
    pConfigChar = pService->createCharacteristic(
        BLE_CONFIG_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    
    // Start service
    pService->start();
    
    // Start advertising
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMaxPreferred(0x12);
    BLEDevice::startAdvertising();
    
    running = true;
    DEBUG_PRINTF("BLE: Service started (Device: %s)\n", config.deviceId);
    
    return true;
}

void BLEService::stop() {
    if (running) {
        BLEDevice::deinit(false);
        running = false;
        DEBUG_PRINTLN("BLE: Service stopped");
    }
}

void BLEService::updateTank1Level(float levelPercent) {
    if (!running || !pTank1Char) return;
    
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.1f", levelPercent);
    pTank1Char->setValue(buffer);
    
    if (deviceConnected) {
        pTank1Char->notify();
        #if DEBUG_BLE
        DEBUG_PRINTF("BLE: Tank1 level updated: %.1f%%\n", levelPercent);
        #endif
    }
}

void BLEService::updateTank2Level(float levelPercent) {
    if (!running || !pTank2Char) return;
    
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.1f", levelPercent);
    pTank2Char->setValue(buffer);
    
    if (deviceConnected) {
        pTank2Char->notify();
        #if DEBUG_BLE
        DEBUG_PRINTF("BLE: Tank2 level updated: %.1f%%\n", levelPercent);
        #endif
    }
}

void BLEService::updatePumpStatus(bool isOn) {
    if (!running || !pPumpChar) return;
    
    pPumpChar->setValue(isOn ? "1" : "0");
    
    #if DEBUG_BLE
    DEBUG_PRINTF("BLE: Pump status updated: %s\n", isOn ? "ON" : "OFF");
    #endif
}

// Server callbacks implementation
void BLEService::ServerCallbacks::onConnect(BLEServer* pServer) {
    bleService->deviceConnected = true;
    DEBUG_PRINTLN("BLE: Client connected");
}

void BLEService::ServerCallbacks::onDisconnect(BLEServer* pServer) {
    bleService->deviceConnected = false;
    DEBUG_PRINTLN("BLE: Client disconnected");
    
    // Restart advertising
    delay(500);
    pServer->startAdvertising();
    DEBUG_PRINTLN("BLE: Advertising restarted");
}

// Pump control callbacks implementation
void BLEService::PumpCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    
    if (value.length() > 0 && bleService->pumpController) {
        char cmd = value[0];
        
        DEBUG_PRINTF("BLE: Pump control command: %c\n", cmd);
        
        if (cmd == '1') {
            bleService->pumpController->turnOn();
        } else if (cmd == '0') {
            bleService->pumpController->turnOff();
        }
    }
}

