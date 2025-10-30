#include "pump_controller.h"
#include "config.h"

PumpController::PumpController(ConfigManager& configManager)
    : configManager(configManager),
      state(PUMP_OFF),
      pumpStartTime(0),
      pumpStopTime(0),
      lastError("") {
}

bool PumpController::begin() {
    const SystemConfig& config = configManager.getConfig();
    
    pinMode(config.pumpRelayPin, OUTPUT);
    setRelay(false); // Ensure pump is off on startup
    
    setState(PUMP_OFF);
    
    DEBUG_PRINTF("Pump controller initialized (Pin: %d, Mode: %d)\n", 
                 config.pumpRelayPin, config.pumpMode);
    
    return true;
}

bool PumpController::turnOn() {
    if (!canStart()) {
        DEBUG_PRINTF("Pump: Cannot start - %s\n", lastError.c_str());
        return false;
    }
    
    setRelay(true);
    setState(PUMP_ON);
    pumpStartTime = millis();
    
    DEBUG_PRINTLN("Pump: Turned ON (manual)");
    return true;
}

bool PumpController::turnOff() {
    if (state == PUMP_ON) {
        setRelay(false);
        setState(PUMP_COOLDOWN);
        pumpStopTime = millis();
        
        DEBUG_PRINTLN("Pump: Turned OFF (manual)");
        return true;
    }
    
    return false;
}

void PumpController::update(float currentLevel, float sourceLevel) {
    const SystemConfig& config = configManager.getConfig();
    
    // Only auto-control if in automatic mode
    if (config.pumpMode != PUMP_AUTOMATIC) {
        // Still enforce safety limits even in manual mode
        if (state == PUMP_ON && shouldStop()) {
            emergencyStop("Safety limit reached");
        }
        return;
    }
    
    // State machine
    switch (state) {
        case PUMP_OFF:
            // Check if we should start
            if (currentLevel <= config.pumpAutoOnThreshold) {
                if (canStart() && isSafe(sourceLevel)) {
                    DEBUG_PRINTF("Pump: Auto-starting (level: %.1f%% <= %.1f%%)\n", 
                                currentLevel, config.pumpAutoOnThreshold);
                    turnOn();
                }
            }
            break;
            
        case PUMP_ON:
            // Check if we should stop
            if (currentLevel >= config.pumpAutoOffThreshold) {
                DEBUG_PRINTF("Pump: Auto-stopping (level: %.1f%% >= %.1f%%)\n", 
                            currentLevel, config.pumpAutoOffThreshold);
                turnOff();
            } else if (shouldStop()) {
                emergencyStop("Safety limit reached");
            } else if (!isSafe(sourceLevel)) {
                emergencyStop("Source tank too low");
            }
            break;
            
        case PUMP_COOLDOWN:
            // Check if cooldown period is over
            if (millis() - pumpStopTime >= config.pumpCooldownTime) {
                DEBUG_PRINTLN("Pump: Cooldown complete");
                setState(PUMP_OFF);
            }
            break;
            
        case PUMP_ERROR:
            // Require manual intervention
            break;
    }
}

bool PumpController::canStart() const {
    const SystemConfig& config = configManager.getConfig();
    
    // Check if pump is already running
    if (state == PUMP_ON) {
        lastError = "Already running";
        return false;
    }
    
    // Check if in cooldown
    if (state == PUMP_COOLDOWN) {
        uint32_t cooldownRemaining = getCooldownRemaining();
        if (cooldownRemaining > 0) {
            lastError = "In cooldown";
            return false;
        }
    }
    
    // Check if in error state
    if (state == PUMP_ERROR) {
        lastError = "Error state - manual reset required";
        return false;
    }
    
    return true;
}

bool PumpController::shouldStop() const {
    const SystemConfig& config = configManager.getConfig();
    
    if (state != PUMP_ON) {
        return false;
    }
    
    uint32_t runtime = millis() - pumpStartTime;
    
    // Check minimum run time
    if (runtime < PUMP_MIN_RUN_TIME) {
        return false; // Too soon to stop
    }
    
    // Check maximum run time
    if (runtime >= config.pumpMaxRunTime) {
        lastError = "Maximum run time exceeded";
        return true;
    }
    
    return false;
}

bool PumpController::isSafe(float sourceLevel) const {
    // Check if source tank has enough water
    if (sourceLevel < PUMP_DRY_RUN_THRESHOLD) {
        lastError = "Source tank too low (dry-run protection)";
        return false;
    }
    
    return true;
}

uint32_t PumpController::getCooldownRemaining() const {
    const SystemConfig& config = configManager.getConfig();
    
    if (state != PUMP_COOLDOWN) {
        return 0;
    }
    
    uint32_t elapsed = millis() - pumpStopTime;
    if (elapsed >= config.pumpCooldownTime) {
        return 0;
    }
    
    return config.pumpCooldownTime - elapsed;
}

void PumpController::setMode(PumpMode mode) {
    SystemConfig& config = configManager.getConfigRef();
    config.pumpMode = mode;
    DEBUG_PRINTF("Pump: Mode set to %d\n", mode);
}

PumpMode PumpController::getMode() const {
    return configManager.getConfig().pumpMode;
}

void PumpController::setThresholds(float onThreshold, float offThreshold) {
    SystemConfig& config = configManager.getConfigRef();
    
    if (onThreshold < offThreshold && onThreshold >= 0 && offThreshold <= 100) {
        config.pumpAutoOnThreshold = onThreshold;
        config.pumpAutoOffThreshold = offThreshold;
        DEBUG_PRINTF("Pump: Thresholds set (ON: %.1f%%, OFF: %.1f%%)\n", 
                     onThreshold, offThreshold);
    }
}

void PumpController::setState(PumpState newState) {
    if (state != newState) {
        state = newState;
        
        #ifdef DEBUG_SERIAL
        const char* stateNames[] = {"OFF", "ON", "COOLDOWN", "ERROR"};
        DEBUG_PRINTF("Pump: State changed to %s\n", stateNames[newState]);
        #endif
    }
}

void PumpController::emergencyStop(const char* reason) {
    DEBUG_PRINTF("Pump: EMERGENCY STOP - %s\n", reason);
    
    setRelay(false);
    setState(PUMP_ERROR);
    lastError = reason;
    
    // Could also trigger an alarm, send notification, etc.
}

void PumpController::setRelay(bool on) {
    const SystemConfig& config = configManager.getConfig();
    digitalWrite(config.pumpRelayPin, on ? HIGH : LOW);
}

