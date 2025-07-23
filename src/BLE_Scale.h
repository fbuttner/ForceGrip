#pragma once

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "Config.h"
#include "HX711.h"

extern Config config;
extern HX711 LoadCell;
extern const int LOADCELL_DOUT_PIN;
extern void read_loadCell_ISR();

class BLE_Scale : public BLEServerCallbacks, public BLECharacteristicCallbacks {
    public:
    
        void init(const std::string& deviceName);
        void updateWeight(long weightInGram);
        void updateBatteryLevel(uint8_t batteryLevel);
        bool isConnected();
        void startAdvertising() { _pBLEServer->startAdvertising(); }
        void stopAdvertising() { _pBLEServer->getAdvertising()->stop(); }

    private:
        BLEServer* _pBLEServer;
        BLECharacteristic* _pWeightCharacteristic;
        BLECharacteristic* _pBatteryCharacteristic;
        BLECharacteristic* _pControlPointCharacteristic;
        BLECharacteristic* _pAckCharacteristic;

        const char* _serviceUUID = "0000181D-0000-1000-8000-00805f9b34fb";
        const char* _weightCharUUID = "00002A9D-0000-1000-8000-00805f9b34fb";
        const char* _batteryCharUUID = "00002A19-0000-1000-8000-00805f9b34fb";
        const char* _controlPointCharUUID = "00002A9F-0000-1000-8000-00805f9b34fb"; // A new UUID for the control point
        const char* _ackCharUUID = "0000181D-1000-1000-8000-00805f9b34fb";
        
        std::string _deviceName;
        bool _deviceConnected=false;

        void onConnect(BLEServer* pBLEServer) override;
        void onDisconnect(BLEServer* pBLEServer) override;
        void onWrite(BLECharacteristic *pCharacteristic) override;
};