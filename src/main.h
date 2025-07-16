#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <SPIFFS.h>

#include <FastLED.h>

#include <Button2.h>

#include "HX711.h"
#include "EveryXTime.h"

#include "Battery.h"
#include "BLE_Scale.h"
#include "Config.h"
#include "ServerWiFi.h"


/******************************************************************************************************/
/*                                        VARIABLE & CONSTANT                                         */
/******************************************************************************************************/
// Battery
Battery battery(A3);

// BLE
BLE_Scale ble_scale;
bool oldDeviceConnected;

ServerWiFi serverWiFi;

// Config file
Config config;

// Load Cell HX711
const int LOADCELL_SCK_PIN = 5;
const int LOADCELL_DOUT_PIN = 6;

HX711 LoadCell;
#define WEIGHT_DATA_LENGHT 50
float WeightValue[WEIGHT_DATA_LENGHT] = {0.0};
int64_t timestamp[WEIGHT_DATA_LENGHT] = {0};
uint8_t WeightValueAvailable = 0;

// LED
const uint8_t LED_PIN = 7;
CRGB myLED[1];

// Button
const uint8_t BUTTON_PIN  = 9;
Button2 button;

// State
uint8_t stateMode = 0;

enum _STATE_MODE_ {
  INITIALISATION,
  BLE_BINDING,
  BLE_MEASUREMENT,
  TARRING,
  WIFI_CONNECTING,
  WIFI_CONNECTED
};


/******************************************************************************************************/
/*                                        FUNCTION DECLARATION                                        */
/******************************************************************************************************/

void IRAM_ATTR read_loadCell_ISR();
void longClickDetected(Button2& btn);
void shortClick(Button2& btn);

