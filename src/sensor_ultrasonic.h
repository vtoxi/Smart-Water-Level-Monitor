#ifndef SENSOR_ULTRASONIC_H
#define SENSOR_ULTRASONIC_H

#include <Arduino.h>

// Sensor reading structure
struct SensorReading {
    float distanceCm;
    float levelPercent;
    bool isValid;
    uint32_t timestamp;
    uint8_t errorCode;
};

// Error codes
enum SensorError {
    ERROR_NONE = 0,
    ERROR_TIMEOUT = 1,
    ERROR_OUT_OF_RANGE = 2,
    ERROR_HARDWARE = 3
};

class UltrasonicSensor {
public:
    UltrasonicSensor(uint8_t trigPin, uint8_t echoPin, float emptyCm, float fullCm);
    
    // Initialize sensor
    bool begin();
    
    // Read distance with median filtering
    SensorReading readDistance();
    
    // Update calibration values
    void setCalibration(float emptyCm, float fullCm);
    void getCalibration(float& emptyCm, float& fullCm) const;
    
    // Convert distance to percentage
    float distanceToPercent(float distanceCm) const;
    float percentToDistance(float percent) const;
    
    // Get last valid reading
    const SensorReading& getLastReading() const { return lastReading; }
    
    // Sensor status
    bool isHealthy() const { return consecutiveErrors < 5; }
    uint8_t getErrorCount() const { return consecutiveErrors; }
    void resetErrorCount() { consecutiveErrors = 0; }
    
    // Configuration
    void setSampleCount(uint8_t count);
    void setTimeout(uint32_t timeoutUs);
    
private:
    // Pin configuration
    uint8_t trigPin;
    uint8_t echoPin;
    
    // Calibration
    float emptyCm;  // Distance when tank is empty (sensor to bottom)
    float fullCm;   // Distance when tank is full (sensor to water surface)
    
    // Sensor parameters
    uint8_t sampleCount;
    uint32_t timeoutUs;
    
    // State tracking
    SensorReading lastReading;
    uint8_t consecutiveErrors;
    
    // Internal methods
    float measureSingleDistance();
    float calculateMedian(float* samples, uint8_t count);
    void sortArray(float* arr, uint8_t count);
    bool validateDistance(float distance) const;
};

#endif // SENSOR_ULTRASONIC_H

