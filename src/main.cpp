// %Bat=109,376-109,376/(1+(Volt/3,468929)^98,99466)^0,1297592
// %Bat=109.376-109.376/(1+(Volt/3.468929)^98.99466)^0.1297592
//3.4 ((((110)/(110-x)))^(8.8)-1)^(((1)/(100)))
/******************************************************************************************************/
/*                                             FORCE GRIP                                             */
/******************************************************************************************************/
/*                                                                                                    */
/* Exemple Arduino : https://medium.com/@sdranju/19a649bf9880                                         */
/* Exemple App : https://medium.com/@shubhamhande/6c9181903915                                        */
/*                                                                                                    */
/*                                                                                                    */
/* pinOUT : https://www.wemos.cc/en/latest/_static/boards/c3_pico_v1.0.0_4_16x9.png                   */
/*                                                                                                    */
/*                                                                                                    */
/*                                                                                                    */
/******************************************************************************************************/
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <FastLED.h>

#include <Button2.h>

#include "HX711.h"
#include "EveryXTime.h"


/******************************************************************************************************/
/*                                        VARIABLE & CONSTANT                                         */
/******************************************************************************************************/
// BLE

const char* BLE_SERVICE_UUID              = "0000181D-0000-1000-8000-00805f9b34fb"; // Standard UUID's for BLE Weight Scale
const char* BLE_WEIGHT_MEASURE_CHAR_UUID  = "00002A9D-0000-1000-8000-00805f9b34fb";
const char* BLE_BATTERY_LEVEL_CHAR_UUID   = "00002A19-0000-1000-8000-00805f9b34fb";
const char* BLE_Name                      = "Force-Grip";

BLEServer* pBLEServer = NULL;
BLECharacteristic *pWeightCharacteristic = NULL, *pBatteryCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;


// Battery
// const float LUT_BATTERY_TO_PERCENT[101] = {0, 3.293038087, 3.330930117, 3.3495445, 3.362428522, 3.372522988, 3.380970002, 3.388334728, 3.394938862, 3.400983293, 3.406602085, 3.411889307, 3.41691362, 3.421726763, 3.426368775, 3.430871342, 3.435260031, 3.439555823, 3.443776198, 3.447935915, 3.452047589, 3.456122126, 3.460169052, 3.464196767, 3.468212747, 3.472223702, 3.476235704, 3.480254291, 3.48428455, 3.488331191, 3.492398601, 3.496490897, 3.500611965, 3.5047655, 3.508955033, 3.513183964, 3.517455578, 3.521773076, 3.526139587, 3.53055819, 3.535031928, 3.539563827, 3.544156905, 3.548814191, 3.553538733, 3.558333618, 3.563201976, 3.568147002, 3.573171959, 3.578280201, 3.583475178, 3.588760454, 3.594139719, 3.599616808, 3.605195709, 3.610880589, 3.616675803, 3.622585919, 3.628615734, 3.6347703, 3.641054943, 3.647475294, 3.654037312, 3.660747319, 3.667612034, 3.674638605, 3.681834659, 3.689208342, 3.69676837, 3.704524092, 3.712485549, 3.720663551, 3.729069752, 3.73771675, 3.746618189, 3.755788877, 3.765244928, 3.775003913, 3.78508505, 3.795509409, 3.806300159, 3.817482856, 3.829085781, 3.841140336, 3.853681522, 3.866748504, 3.880385294, 3.894641578, 3.909573732, 3.92524606, 3.941732352, 3.959117823, 3.977501578, 3.996999783, 4.01774978, 4.039915522, 4.063694855, 4.089329464, 4.117118737, 4.147439536, 4.180775189};
// 3090 => 4.2V
const uint16_t LUT_BATTERY_TO_PERCENT[101] =  {0, 2423, 2451, 2464, 2474, 2481, 2487, 2493, 2498, 2502, 2506,
                                                  2510, 2514, 2517, 2521, 2524, 2527, 2531, 2534, 2537, 2540,
                                                  2543, 2546, 2549, 2552, 2555, 2558, 2560, 2563, 2566, 2569,
                                                  2572, 2575, 2579, 2582, 2585, 2588, 2591, 2594, 2597, 2601,
                                                  2604, 2607, 2611, 2614, 2618, 2621, 2625, 2629, 2633, 2636,
                                                  2640, 2644, 2648, 2652, 2657, 2661, 2665, 2670, 2674, 2679,
                                                  2683, 2688, 2693, 2698, 2703, 2709, 2714, 2720, 2725, 2731,
                                                  2737, 2744, 2750, 2756, 2763, 2770, 2777, 2785, 2792, 2800,
                                                  2809, 2817, 2826, 2835, 2845, 2855, 2865, 2876, 2888, 2900,
                                                  2913, 2926, 2941, 2956, 2972, 2990, 3009, 3029, 3051, 3076};
const uint8_t VBAT = A3;

uint8_t batteryLevel = 100;
uint16_t batteryValue = 0;

// Load Cell HX711
const int LOADCELL_SCK_PIN = 5;
const int LOADCELL_DOUT_PIN = 6;

HX711 LoadCell;
float LoadCell_ScaleFactor = 2.27;
float WeightValue = 0;
bool WeightValueAvailable = false;

// LED
const uint8_t LED_PIN = 7;
CRGB myLED[1];

// Other
const size_t EEPROM_SIZE = 32;
const uint8_t BUTTON_PIN  = 9;
Button2 button;
uint8_t stateMode = 0;
uint8_t calibrationMode = 0;

enum _STATE_MODE_ {
  INITIALISATION,
  BLE_BINDING,
  MEASUREMENT,
  TARRING,
  CALIBRATION
};

enum _CALIBRATION_MODE_ {
  INCREASE_CALIBRATION_VALUE,
  DECREASE_CALIBRATION_VALUE,
};


/******************************************************************************************************/
/*                                        FUNCTION DECLARATION                                        */
/******************************************************************************************************/
void BLE_update_weight_caracteristic(long weightInGram);
void setup_bleserver();
void update_battery_level();
void IRAM_ATTR read_loadCell_ISR()  {WeightValue = LoadCell.get_units();WeightValueAvailable=true;}
void longClickDetected(Button2& btn);
void shortClick(Button2& btn);



/******************************************************************************************************/
/*                                                                                                    */
/******************************************************************************************************/
// BLE server callback routine
class MyBLEServerCallbacks: public BLEServerCallbacks
{
    void onConnect(BLEServer* pBLEServer)
    {
      deviceConnected = true;
      stateMode = MEASUREMENT;
    };

    void onDisconnect(BLEServer* pBLEServer)
    {
      deviceConnected = false;
      stateMode = BLE_BINDING;
    }
};


/******************************************************************************************************/
/*                                               SETUP                                                */
/******************************************************************************************************/
/*                                                                                                    */
/* Initialised the load cell and start the BLE server                                                 */
/*                                                                                                    */
/******************************************************************************************************/
void setup()
{
  EEPROM.begin(EEPROM_SIZE);
  //LoadCell_ScaleFactor = EEPROM.readFloat(0);

  stateMode = INITIALISATION;

  // setting the onboard LED
  FastLED.addLeds<WS2812, LED_PIN, RGB>(myLED, 1);
  FastLED.setBrightness(10);
  myLED[0] = CRGB::Red;
  FastLED.show();
  
  // setting up the Button
  button.begin(BUTTON_PIN);
  button.setLongClickTime(1000);
  button.setLongClickDetectedRetriggerable(true);
  button.setLongClickDetectedHandler(longClickDetected);
  button.setClickHandler(shortClick);

  delay(3000);
  update_battery_level();

  log_i("Initializing the load cell ...");
  LoadCell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN, 32);

  float EEPROM_LoadCell_ScaleFactor = EEPROM.readFloat(0);
  if(isnanf(EEPROM_LoadCell_ScaleFactor))
  {
    LoadCell.set_scale(LoadCell_ScaleFactor);
    log_i("Using generique scale factor, %f", LoadCell_ScaleFactor);
  }
  else if((EEPROM_LoadCell_ScaleFactor < 1.5f) || (EEPROM_LoadCell_ScaleFactor > 3.0f))
  {
    LoadCell.set_scale(LoadCell_ScaleFactor);
    log_e("Incorect scale factor from EEPROM, %f", EEPROM_LoadCell_ScaleFactor);
    log_i("Using generique scale factor, %f", LoadCell_ScaleFactor);
  }
  else
  {
    LoadCell.set_scale(EEPROM_LoadCell_ScaleFactor);
    log_i("Using scale factor from EEPROM, %f", EEPROM_LoadCell_ScaleFactor);
  }
  LoadCell.get_units(10);

  log_v("Tarring ...");
  LoadCell.tare(10);
  attachInterrupt(LOADCELL_DOUT_PIN, read_loadCell_ISR, FALLING);

  log_i("Initializing BlueTooth...");
  setup_bleserver();
  stateMode = BLE_BINDING;

  log_i("Setup completed...");
}



/******************************************************************************************************/
/*                                             MAIN LOOP                                              */
/******************************************************************************************************/
/*                                                                                                    */
/*                                                                                                    */
/*                                                                                                    */
/******************************************************************************************************/
void loop()
{
  button.loop();

  // notify the weight if a device is connected
  if (WeightValueAvailable)
  {
    WeightValueAvailable=false;

    if(deviceConnected)
      BLE_update_weight_caracteristic(WeightValue>0 ?WeightValue:-WeightValue);
    
    log_d("Weight : %.2f kg", WeightValue/1000);
  }

  // if disconnecting start advertising again
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);
    pBLEServer->startAdvertising();

    oldDeviceConnected = deviceConnected;
  }

  // update the connection status
  if (deviceConnected && !oldDeviceConnected)
    oldDeviceConnected = deviceConnected;
  
  // Update the battery Level every second
  EVERY_X_SECONDS(1)
  {
    update_battery_level();

    log_d("Battery Level : %d\%, Analog Read : %d", batteryLevel, batteryValue);
  }

  // Update the LED color
  switch (stateMode)
  {
    case TARRING:
      break;
    
    case CALIBRATION:
      myLED[0] = (calibrationMode==INCREASE_CALIBRATION_VALUE)?CRGB::Magenta:CRGB::Cyan;
      break;

    default:
      EVERY_X_SECONDS(1)
      {
        static uint8_t blink_state=false;
        blink_state = !blink_state;

        if((stateMode == BLE_BINDING) && blink_state)
        {
          myLED[0] = CRGB::Blue;
        }
        else
        {
          if(batteryLevel>50)
          {
            myLED[0].r = 0xFF*(100-batteryLevel)/50;
            myLED[0].g = 0xFF;
            myLED[0].b = 0;
          }
          else
          {
            myLED[0].r = 0xFF;
            myLED[0].g = 0xFF*batteryLevel/50;
            myLED[0].b = 0;
          }
        }
      }
      break;
  }
  log_d("led color (r, g, b): (%d, %d, %d)", myLED[0].r, myLED[0].g, myLED[0].b);

  FastLED.show();
}



/******************************************************************************************************/
/*                                        update_battery_level                                        */
/******************************************************************************************************/
/*                                                                                                    */
/* Read the battery voltage and update the BLE characteristics                                        */
/*                                                                                                    */
/******************************************************************************************************/
void update_battery_level()
{
  batteryValue = analogRead(VBAT);

  // Convert the value into a percentage
  while (batteryLevel > 0 && LUT_BATTERY_TO_PERCENT[batteryLevel]>batteryValue)  batteryLevel--;
  while (batteryLevel < 100 && LUT_BATTERY_TO_PERCENT[batteryLevel+1]<=batteryValue)  batteryLevel++;

  // notify only if a device is connected
  if(deviceConnected)
  {
    pBatteryCharacteristic->setValue(&batteryLevel, 1);
    pBatteryCharacteristic->notify();
  }
}



/******************************************************************************************************/
/*                                          setup_bleserver                                           */
/******************************************************************************************************/
/*                                                                                                    */
/* Creat the BLE server with the Weight and Battery level characteristics                             */
/*                                                                                                    */
/******************************************************************************************************/
void setup_bleserver()
{
  BLEService *pService;
  BLEDescriptor *pDescr;
  BLE2902 *pBLE2902;
  BLEAdvertising *pAdvertising;

  // Create the BLE Device
  BLEDevice::init(BLE_Name);

  // Create the BLE Server
  pBLEServer = BLEDevice::createServer();
  pBLEServer->setCallbacks(new MyBLEServerCallbacks());

  // Create the BLE Service
  pService = pBLEServer->createService(BLE_SERVICE_UUID);

  // Create a BLE Weight-measurement Characteristic
  pWeightCharacteristic = pService->createCharacteristic(BLE_WEIGHT_MEASURE_CHAR_UUID,
                                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  // Create a BLE Descriptor
  pDescr = new BLEDescriptor((uint16_t)0x2901);
  pDescr->setValue(BLE_Name);  // setting same as device name
  pWeightCharacteristic->addDescriptor(pDescr);
  
  pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);
  
  // Add all Descriptors here
  pWeightCharacteristic->addDescriptor(pBLE2902);
  pWeightCharacteristic->setNotifyProperty(true);

  // Create a BLE Battery level Characteristic
  pBatteryCharacteristic = pService->createCharacteristic(BLE_BATTERY_LEVEL_CHAR_UUID,
                                      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);                   

  // Create a BLE Descriptor
  pDescr = new BLEDescriptor((uint16_t)0x2901);
  pDescr->setValue(BLE_Name);  // setting same as device name
  pBatteryCharacteristic->addDescriptor(pDescr);
  
  pBLE2902 = new BLE2902();
  pBLE2902->setNotifications(true);
  
  // Add all Descriptors here
  pBatteryCharacteristic->addDescriptor(pBLE2902);
  pBatteryCharacteristic->setNotifyProperty(true);

  // Start the service
  pService->start();

  // Start advertising
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(BLE_SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
  
  log_i("Waiting for a BLE client to notify...");
}



/******************************************************************************************************/
/*                                  BLE_update_weight_caracteristic                                   */
/******************************************************************************************************/
/*                                                                                                    */
/* Convert the weight value in grams to the stantard used in the BLE GATT standard                    */
/*   and update the value of the BLE characteristic                                                   */
/*                                                                                                    */
/* Set the flags:                                                                                     */
/*     bit 0 => 0 means we're reporting in SI units (kg and meters)                                   */
/*     bit 1 => 0 means there is no timestamp in our report                                           */
/*     bit 2 => 0 means there is no User ID in our report                                             */
/*     bit 3 => 0 means no BMI and Height data are present in our report                              */
/*     bits 4 to 7 are reserved, and set to zero                                                      */
/*                                                                                                    */
/******************************************************************************************************/
void BLE_update_weight_caracteristic(long weightInGram)
{
  #define _FLAGS 0b00000000
  uint16_t newVal = 0;              // field value: weight in BLE format
  unsigned char bytes[3] = {_FLAGS, 0, 0};     // encoded data for transmission

  // Important: Convert the weight into BLE representation
  newVal = (uint16_t)((weightInGram / 5) + 0.5);

  // BLE GATT multi-byte values are encoded Least-Significant Byte first
  bytes[1] = (unsigned char) newVal;
  bytes[2] = (unsigned char) (newVal >> 8);

  pWeightCharacteristic->setValue(bytes, sizeof(bytes));
  pWeightCharacteristic->notify();
}


/******************************************************************************************************/
/*                                         longClickDetected                                          */
/******************************************************************************************************/
/*                                                                                                    */
/*                                                                                                    */
/******************************************************************************************************/
void longClickDetected(Button2& btn)
{
  switch (stateMode)
  {
    case CALIBRATION:
      // switching between increasing and decresing the scalefactor
      if(btn.getLongClickCount() ==1)
        calibrationMode = (calibrationMode==INCREASE_CALIBRATION_VALUE)?DECREASE_CALIBRATION_VALUE:INCREASE_CALIBRATION_VALUE;
      
      // leaving the calibration mode & save to eeprom the calibration value
      if(btn.getLongClickCount() == 3)
      {
        size_t s = EEPROM.writeFloat(0, LoadCell.get_scale());
        EEPROM.commit();

        stateMode = MEASUREMENT;
      }
      break;

    case MEASUREMENT:
      if(btn.getLongClickCount() == 3)
        stateMode = CALIBRATION;
      break;

    default:
      break;
  }
}


/******************************************************************************************************/
/*                                             shortClick                                             */
/******************************************************************************************************/
/*                                                                                                    */
/*                                                                                                    */
/******************************************************************************************************/
void shortClick(Button2& btn)
{
  switch (stateMode)
  {
    case CALIBRATION:
      switch (calibrationMode)
      {
        case INCREASE_CALIBRATION_VALUE:
            LoadCell.set_scale(LoadCell.get_scale()*WeightValue/(WeightValue+100));
          break;

        case DECREASE_CALIBRATION_VALUE:
            LoadCell.set_scale(LoadCell.get_scale()*WeightValue/(WeightValue-100));
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }
}