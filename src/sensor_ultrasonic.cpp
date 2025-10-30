#include "sensor_ultrasonic.h"
#include "config.h"

UltrasonicSensor::UltrasonicSensor(uint8_t trigPin, uint8_t echoPin, float emptyCm, float fullCm)
    : trigPin(trigPin), echoPin(echoPin), emptyCm(emptyCm), fullCm(fullCm),
      sampleCount(SENSOR_SAMPLES), timeoutUs(SENSOR_TIMEOUT_US), consecutiveErrors(0) {
    
    lastReading.distanceCm = 0;
    lastReading.levelPercent = 0;
    lastReading.isValid = false;
    lastReading.timestamp = 0;
    lastReading.errorCode = ERROR_NONE;
}

bool UltrasonicSensor::begin() {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    
    digitalWrite(trigPin, LOW);
    delay(50); // Let sensor stabilize
    
    DEBUG_PRINTF("Ultrasonic sensor initialized (Trig: %d, Echo: %d)\n", trigPin, echoPin);
    return true;
}

SensorReading UltrasonicSensor::readDistance() {
    float samples[sampleCount];
    uint8_t validSamples = 0;
    
    // Take multiple samples
    for (uint8_t i = 0; i < sampleCount; i++) {
        float distance = measureSingleDistance();
        
        if (distance > 0 && validateDistance(distance)) {
            samples[validSamples++] = distance;
        }
        
        delay(10); // Small delay between samples
    }
    
    SensorReading reading;
    reading.timestamp = millis();
    
    if (validSamples >= 3) { // Need at least 3 valid samples
        // Calculate median
        reading.distanceCm = calculateMedian(samples, validSamples);
        reading.levelPercent = distanceToPercent(reading.distanceCm);
        reading.isValid = true;
        reading.errorCode = ERROR_NONE;
        
        consecutiveErrors = 0;
        lastReading = reading;
        
        #if DEBUG_SENSOR
        DEBUG_PRINTF("Sensor reading: %.2f cm (%.1f%%) [%d samples]\n", 
                     reading.distanceCm, reading.levelPercent, validSamples);
        #endif
    } else {
        // Not enough valid samples
        reading.distanceCm = 0;
        reading.levelPercent = 0;
        reading.isValid = false;
        reading.errorCode = validSamples == 0 ? ERROR_TIMEOUT : ERROR_OUT_OF_RANGE;
        
        consecutiveErrors++;
        
        #if DEBUG_SENSOR
        DEBUG_PRINTF("Sensor error: %d (valid samples: %d)\n", reading.errorCode, validSamples);
        #endif
    }
    
    return reading;
}

float UltrasonicSensor::measureSingleDistance() {
    // Send 10μs pulse
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    // Measure echo pulse width
    unsigned long duration = pulseIn(echoPin, HIGH, timeoutUs);
    
    if (duration == 0) {
        return -1; // Timeout
    }
    
    // Calculate distance: duration / 2 (round trip) * speed of sound
    float distance = (duration / 2.0) * SPEED_OF_SOUND;
    
    return distance;
}

float UltrasonicSensor::calculateMedian(float* samples, uint8_t count) {
    if (count == 0) return 0;
    
    // Create a copy to sort
    float sorted[count];
    for (uint8_t i = 0; i < count; i++) {
        sorted[i] = samples[i];
    }
    
    sortArray(sorted, count);
    
    // Return median
    if (count % 2 == 0) {
        return (sorted[count/2 - 1] + sorted[count/2]) / 2.0;
    } else {
        return sorted[count/2];
    }
}

void UltrasonicSensor::sortArray(float* arr, uint8_t count) {
    // Simple bubble sort (fine for small arrays)
    for (uint8_t i = 0; i < count - 1; i++) {
        for (uint8_t j = 0; j < count - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                float temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

bool UltrasonicSensor::validateDistance(float distance) const {
    // Check if distance is within reasonable sensor range (JSN-SR04T: 25-450cm typical)
    if (distance < 5 || distance > 500) {
        return false;
    }
    
    // Check if distance is within calibrated range (with some tolerance)
    float tolerance = 10.0; // 10cm tolerance
    if (distance < (fullCm - tolerance) || distance > (emptyCm + tolerance)) {
        #if DEBUG_SENSOR
        DEBUG_PRINTF("Distance %.2f cm out of calibrated range (%.2f - %.2f cm)\n", 
                     distance, fullCm, emptyCm);
        #endif
        // Still return true, just log the warning
    }
    
    return true;
}

float UltrasonicSensor::distanceToPercent(float distanceCm) const {
    // When distance is small = tank is full (high percentage)
    // When distance is large = tank is empty (low percentage)
    
    if (distanceCm <= fullCm) {
        return 100.0;
    } else if (distanceCm >= emptyCm) {
        return 0.0;
    }
    
    float range = emptyCm - fullCm;
    float percent = 100.0 - ((distanceCm - fullCm) / range * 100.0);
    
    return constrain(percent, 0.0, 100.0);
}

float UltrasonicSensor::percentToDistance(float percent) const {
    percent = constrain(percent, 0.0, 100.0);
    
    float range = emptyCm - fullCm;
    float distance = emptyCm - (percent / 100.0 * range);
    
    return distance;
}

void UltrasonicSensor::setCalibration(float emptyCm, float fullCm) {
    if (emptyCm > fullCm && emptyCm > 0 && fullCm > 0) {
        this->emptyCm = emptyCm;
        this->fullCm = fullCm;
        DEBUG_PRINTF("Calibration updated: Empty=%.1f cm, Full=%.1f cm\n", emptyCm, fullCm);
    }
}

void UltrasonicSensor::getCalibration(float& emptyCm, float& fullCm) const {
    emptyCm = this->emptyCm;
    fullCm = this->fullCm;
}

void UltrasonicSensor::setSampleCount(uint8_t count) {
    sampleCount = constrain(count, 3, 10);
}

void UltrasonicSensor::setTimeout(uint32_t timeoutUs) {
    this->timeoutUs = timeoutUs;
}

