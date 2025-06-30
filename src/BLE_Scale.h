#pragma once

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

class BLE_Scale : public BLEServerCallbacks {
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

        const char* _serviceUUID = "0000181D-0000-1000-8000-00805f9b34fb";
        const char* _weightCharUUID = "00002A9D-0000-1000-8000-00805f9b34fb";
        const char* _batteryCharUUID = "00002A19-0000-1000-8000-00805f9b34fb";
        
        std::string _deviceName;
        bool _deviceConnected=false;

        void onConnect(BLEServer* pBLEServer) override;
        void onDisconnect(BLEServer* pBLEServer) override;
};
