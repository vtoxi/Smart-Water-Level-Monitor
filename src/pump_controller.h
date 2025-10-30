#ifndef PUMP_CONTROLLER_H
#define PUMP_CONTROLLER_H

#include <Arduino.h>
#include "config_manager.h"
#include "sensor_ultrasonic.h"

// Pump state
enum PumpState {
    PUMP_OFF = 0,
    PUMP_ON = 1,
    PUMP_COOLDOWN = 2,
    PUMP_ERROR = 3
};

class PumpController {
public:
    PumpController(ConfigManager& configManager);
    
    // Initialize pump controller
    bool begin();
    
    // Manual control
    bool turnOn();
    bool turnOff();
    
    // Automatic control (must be called regularly)
    void update(float currentLevel, float sourceLevel = 100.0);
    
    // Status
    bool isRunning() const { return (state == PUMP_ON); }
    PumpState getState() const { return state; }
    uint32_t getRunTime() const { return isRunning() ? (millis() - pumpStartTime) : 0; }
    uint32_t getCooldownRemaining() const;
    
    // Configuration
    void setMode(PumpMode mode);
    PumpMode getMode() const;
    void setThresholds(float onThreshold, float offThreshold);
    
    // Safety checks
    bool isSafe(float sourceLevel) const;
    String getLastError() const { return lastError; }
    
private:
    ConfigManager& configManager;
    PumpState state;
    uint32_t pumpStartTime;
    uint32_t pumpStopTime;
    String lastError;
    
    // Internal helpers
    bool canStart() const;
    bool shouldStop() const;
    void setState(PumpState newState);
    void emergencyStop(const char* reason);
    void setRelay(bool on);
};

#endif // PUMP_CONTROLLER_H

