#include "BLE_Scale.h"

void BLE_Scale::init(const std::string& deviceName) {
    BLEDescriptor *pDescr;
    BLE2902 *pBLE2902;
    BLEAdvertising *pAdvertising;
    BLEService *pService;

    if(deviceName.length() > 0)
        _deviceName = "ForceGrip - " + deviceName;
    else
    {
        uint8_t mac[6];
        char macStr[7] = { 0 };

        esp_read_mac(mac, ESP_MAC_BT);

        sprintf(macStr, "%02X%02X%02X",  mac[3], mac[4], mac[5]);
        _deviceName = "ForceGrip_";
        _deviceName += macStr;
    }
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

    // Create a BLE Control Point Characteristic
    log_i("Creating BLE Control Point Characteristic...");
    _pControlPointCharacteristic = pService->createCharacteristic(_controlPointCharUUID, BLECharacteristic::PROPERTY_WRITE);
    _pControlPointCharacteristic->setCallbacks(this);

    // Create a BLE Ack Characteristic
    log_i("Creating BLE Ack Characteristic...");
    _pAckCharacteristic = pService->createCharacteristic(_ackCharUUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    pBLE2902 = new BLE2902();
    pBLE2902->setNotifications(true);
    _pAckCharacteristic->addDescriptor(pBLE2902);
    _pAckCharacteristic->setNotifyProperty(true);

    // Start the service
    log_i("Starting BLE Service...");
    pService->start();

    // Creating advertising
    log_i("Creating BLE Advertising...");
    pAdvertising = BLEDevice::getAdvertising();
    log_d("Adding BLE Service UUID...");
    pAdvertising->addServiceUUID(_serviceUUID);
    log_d("Setting BLE Advertising parameters...");
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    
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


void BLE_Scale::onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();

    if (value.length() > 0)
    {
        log_i("Received command: %s", value.c_str());
        if (value == "tare")
        {
            detachInterrupt(LOADCELL_DOUT_PIN);
            LoadCell.tare(10);
            attachInterrupt(LOADCELL_DOUT_PIN, read_loadCell_ISR, FALLING);
            log_d("BLE Tare done");
            _pAckCharacteristic->setValue("OK");
            _pAckCharacteristic->notify();
        }
        else
        {
            // split the string at :
            size_t colonPos = value.find(':');
            if (colonPos != std::string::npos)
            {
                std::string command = value.substr(0, colonPos);
                std::string param = value.substr(colonPos + 1);
                bool configChanged = false;

                if (command == "set_scale")
                {
                    float scaleFactor = std::stof(param);

                    detachInterrupt(LOADCELL_DOUT_PIN);
                    LoadCell.set_scale(scaleFactor);
                    attachInterrupt(LOADCELL_DOUT_PIN, read_loadCell_ISR, FALLING);
                    
                    config.set_scaleFactor(scaleFactor);
                    configChanged = true;

                    log_d("BLE Scale factor set to: %f", scaleFactor);
                }
                else if (command == "set_name")
                {
                    config.set_name(param.c_str());
                    configChanged = true;

                    log_d("BLE Device name set to: %s", param.c_str());
                }
                else if (command == "calibrate")
                {
                    float calibration_weight = std::stof(param) * 1000;

                    detachInterrupt(LOADCELL_DOUT_PIN);

                    double val = LoadCell.get_value(10);
                    float _scaleFactor = val / calibration_weight;
                    LoadCell.set_scale(_scaleFactor);
                    attachInterrupt(LOADCELL_DOUT_PIN, read_loadCell_ISR, FALLING);

                    config.set_scaleFactor(_scaleFactor);
                    configChanged = true;
                    
                    log_d("Calibration value set to: %f", _scaleFactor);
                }
                else if (command == "startup_mode")
                {
                    int _startup_mode = std::stoi(param);

                    config.set_startingMode(_startup_mode);
                    configChanged = true;

                    log_d("Startup mode set to: %s", (_startup_mode?"WIFI":"BLE"));
                }
                else
                {
                    log_d("Unknown command: %s:%s", command.c_str(), param.c_str());
                }
                
                if(configChanged)
                {
                    config.writeConfig();
                    _pAckCharacteristic->setValue("OK");
                    _pAckCharacteristic->notify();
                }
                else
                {
                    _pAckCharacteristic->setValue("ERROR");
                    _pAckCharacteristic->notify();
                }
            }
            else
            {
                _pAckCharacteristic->setValue("ERROR");
                _pAckCharacteristic->notify();
                
                log_d("Unknown command: %s", value.c_str());
            }
        }
    }
}
