#pragma once

#include <Arduino.h>

// Forward-declare this class to avoid including the full BLE header here.
// This is a good practice to speed up compilation.
class BLECharacteristic;

class Battery {
    public:
        Battery(uint8_t pin);
        void update();
        uint8_t getLevel() const;
        int getRawValue() const;

    private:
        uint8_t _pin;
        uint8_t _level;
        int _rawValue;

        // This Look-Up Table (LUT) needs to be filled with your ADC values corresponding to each percentage point.
        static const int LUT_BATTERY_TO_PERCENT[101]; 
};

