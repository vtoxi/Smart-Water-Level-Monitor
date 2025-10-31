#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <Arduino.h>
#include "config.h"

// Only include BLE libraries on boards that support it
#if HAS_BLE
    #include <BLEDevice.h>
    #include <BLEServer.h>
    #include <BLEUtils.h>
    #include <BLE2902.h>
#endif

#include "config_manager.h"

// Forward declaration
class PumpController;

class WaterLevelBLE {
public:
    WaterLevelBLE(ConfigManager& configManager);
    
    // Initialize BLE service
    bool begin();
    void stop();
    
    // Update characteristic values
    void updateTank1Level(float levelPercent);
    void updateTank2Level(float levelPercent);
    void updatePumpStatus(bool isOn);
    
    // Status
    bool isRunning() const { 
        #if HAS_BLE
            return running; 
        #else
            return false;
        #endif
    }
    bool isClientConnected() const { 
        #if HAS_BLE
            return deviceConnected; 
        #else
            return false;
        #endif
    }
    
    // Set pump controller reference for pump control characteristic
    void setPumpController(PumpController* pump) { 
        #if HAS_BLE
            pumpController = pump; 
        #endif
    }
    
private:
    ConfigManager& configManager;
    
    #if HAS_BLE
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
    #endif
    
    #if HAS_BLE
        // Server callbacks
        class ServerCallbacks : public BLEServerCallbacks {
        public:
            ServerCallbacks(WaterLevelBLE* service) : bleService(service) {}
            void onConnect(BLEServer* pServer);
            void onDisconnect(BLEServer* pServer);
        private:
            WaterLevelBLE* bleService;
        };
        
        // Pump control characteristic callbacks
        class PumpCallbacks : public BLECharacteristicCallbacks {
        public:
            PumpCallbacks(WaterLevelBLE* service) : bleService(service) {}
            void onWrite(BLECharacteristic* pCharacteristic);
        private:
            WaterLevelBLE* bleService;
        };
        
        friend class ServerCallbacks;
        friend class PumpCallbacks;
    #endif
};

#endif // BLE_SERVICE_H

