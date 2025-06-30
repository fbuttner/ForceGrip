#include "BLE_Scale.h"

void BLE_Scale::init(const std::string& deviceName) {
    BLEDescriptor *pDescr;
    BLE2902 *pBLE2902;
    BLEAdvertising *pAdvertising;
    BLEService *pService;

    _deviceName = deviceName;
    _deviceConnected = false;

    // Create the BLE Device
    log_i("Starting BLE work!");
    BLEDevice::init(_deviceName);

    // Create the BLE Server
    log_i("Creating BLE Server...");
    _pBLEServer = BLEDevice::createServer();
    _pBLEServer->setCallbacks(this);

    // Create the BLE Service
    log_i("Creating BLE Service...");
    pService = _pBLEServer->createService(_serviceUUID);

    log_i("Creating BLE Weight Characteristic...");
    // Create a BLE Weight-measurement Characteristic
    _pWeightCharacteristic = pService->createCharacteristic(_weightCharUUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

    // Create a BLE Descriptor
    pDescr = new BLEDescriptor((uint16_t)0x2901);
    pDescr->setValue(_deviceName);  // setting same as device name
    _pWeightCharacteristic->addDescriptor(pDescr);
    
    pBLE2902 = new BLE2902();
    pBLE2902->setNotifications(true);
    
    // Add all Descriptors here
    _pWeightCharacteristic->addDescriptor(pBLE2902);
    _pWeightCharacteristic->setNotifyProperty(true);

    // Create a BLE Battery level Characteristic
    log_i("Creating BLE Battery Characteristic...");
    _pBatteryCharacteristic = pService->createCharacteristic(_batteryCharUUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);                   

    // Create a BLE Descriptor
    pDescr = new BLEDescriptor((uint16_t)0x2901);
    pDescr->setValue(_deviceName);  // setting same as device name
    _pBatteryCharacteristic->addDescriptor(pDescr);
    
    pBLE2902 = new BLE2902();
    pBLE2902->setNotifications(true);
    
    // Add all Descriptors here
    _pBatteryCharacteristic->addDescriptor(pBLE2902);
    _pBatteryCharacteristic->setNotifyProperty(true);

    // Start the service
    log_i("Starting BLE Service...");
    pService->start();

    // Start advertising
    log_i("Creating BLE Advertising...");
    pAdvertising = BLEDevice::getAdvertising();
    log_i("Adding BLE Service UUID...");
    pAdvertising->addServiceUUID(_serviceUUID);
    log_i("Setting BLE Advertising parameters...");
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);

    log_i("Starting BLE Advertising...");
    pAdvertising->start();
    
    log_i("Waiting for a BLE client to notify...");
}

void BLE_Scale::updateWeight(long weightInGram) {
    if(!_deviceConnected)   return;

    uint16_t newVal = 0;              // field value: weight in BLE format
    unsigned char bytes[3] = {0U, 0U, 0U};     // encoded data for transmission

    // Important: Convert the weight into BLE representation
    newVal = (uint16_t)((weightInGram / 5) + 0.5);

    // BLE GATT multi-byte values are encoded Least-Significant Byte first
    bytes[1] = (unsigned char) newVal;
    bytes[2] = (unsigned char) (newVal >> 8);

    _pWeightCharacteristic->setValue(bytes, sizeof(bytes));
    _pWeightCharacteristic->notify();
}

void BLE_Scale::updateBatteryLevel(uint8_t batteryLevel) {
    if(!_deviceConnected)   return;
    
    _pBatteryCharacteristic->setValue(&batteryLevel, 1);
    _pBatteryCharacteristic->notify();
}

void BLE_Scale::onConnect(BLEServer* pBLEServer) {
    _deviceConnected = true;
}

void BLE_Scale::onDisconnect(BLEServer* pBLEServer) {
    _deviceConnected = false;
    startAdvertising();
}

bool BLE_Scale::isConnected() {
    return _deviceConnected;
}