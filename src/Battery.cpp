#include "Battery.h"
#include <BLECharacteristic.h>

// The Look-Up Table is part of the class implementation.
const int Battery::LUT_BATTERY_TO_PERCENT[101] = {0, 2423, 2451, 2464, 2474, 2481, 2487, 2493, 2498, 2502, 2506,
                                                  2510, 2514, 2517, 2521, 2524, 2527, 2531, 2534, 2537, 2540,
                                                  2543, 2546, 2549, 2552, 2555, 2558, 2560, 2563, 2566, 2569,
                                                  2572, 2575, 2579, 2582, 2585, 2588, 2591, 2594, 2597, 2601,
                                                  2604, 2607, 2611, 2614, 2618, 2621, 2625, 2629, 2633, 2636,
                                                  2640, 2644, 2648, 2652, 2657, 2661, 2665, 2670, 2674, 2679,
                                                  2683, 2688, 2693, 2698, 2703, 2709, 2714, 2720, 2725, 2731,
                                                  2737, 2744, 2750, 2756, 2763, 2770, 2777, 2785, 2792, 2800,
                                                  2809, 2817, 2826, 2835, 2845, 2855, 2865, 2876, 2888, 2900,
                                                  2913, 2926, 2941, 2956, 2972, 2990, 3009, 3029, 3051, 3076};

Battery::Battery(uint8_t pin) : _pin(pin), _level(0), _rawValue(0) {}

void Battery::update() {
    _rawValue = analogRead(_pin);

    // Convert the value into a percentage using the LUT
    while (_level > 0 && LUT_BATTERY_TO_PERCENT[_level] > _rawValue)  _level--;
    while (_level < 100 && LUT_BATTERY_TO_PERCENT[_level+1] <= _rawValue)  _level++;
}

uint8_t Battery::getLevel() const { return _level; }
int Battery::getRawValue() const { return _rawValue; }

