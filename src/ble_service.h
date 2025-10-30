#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "config_manager.h"

// Forward declaration
class PumpController;

class BLEService {
public:
    BLEService(ConfigManager& configManager);
    
    // Initialize BLE service
    bool begin();
    void stop();
    
    // Update characteristic values
    void updateTank1Level(float levelPercent);
    void updateTank2Level(float levelPercent);
    void updatePumpStatus(bool isOn);
    
    // Status
    bool isRunning() const { return running; }
    bool isClientConnected() const { return deviceConnected; }
    
    // Set pump controller reference for pump control characteristic
    void setPumpController(PumpController* pump) { pumpController = pump; }
    
private:
    ConfigManager& configManager;
    PumpController* pumpController;
    
    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pTank1Char;
    BLECharacteristic* pTank2Char;
    BLECharacteristic* pPumpChar;
    BLECharacteristic* pConfigChar;
    
    bool running;
    bool deviceConnected;
    bool oldDeviceConnected;
    
    // Server callbacks
    class ServerCallbacks : public BLEServerCallbacks {
    public:
        ServerCallbacks(BLEService* service) : bleService(service) {}
        void onConnect(BLEServer* pServer);
        void onDisconnect(BLEServer* pServer);
    private:
        BLEService* bleService;
    };
    
    // Pump control characteristic callbacks
    class PumpCallbacks : public BLECharacteristicCallbacks {
    public:
        PumpCallbacks(BLEService* service) : bleService(service) {}
        void onWrite(BLECharacteristic* pCharacteristic);
    private:
        BLEService* bleService;
    };
    
    friend class ServerCallbacks;
    friend class PumpCallbacks;
};

#endif // BLE_SERVICE_H

