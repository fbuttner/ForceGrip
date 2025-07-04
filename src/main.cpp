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
#include "main.h"


/******************************************************************************************************/
/*                                               SETUP                                                */
/******************************************************************************************************/
/*                                                                                                    */
/* Initialised the load cell and start the BLE server                                                 */
/*                                                                                                    */
/******************************************************************************************************/
void setup()
{
    stateMode = INITIALISATION;

    log_i("Initializing SPIFFS...");
    if (!SPIFFS.begin(true)) {
        log_i("Erreur SPIFFS");
        return;
    }
    
    log_i("loading the config file");
    config.readConfig();

    // setting the onboard LED
    FastLED.addLeds<WS2812, LED_PIN, RGB>(myLED, 1);
    FastLED.setBrightness(10);
    myLED[0] = CRGB::Red;
    FastLED.show();

    // setting up the Button
    button.begin(BUTTON_PIN);
    button.setLongClickTime(2000);
    button.setLongClickDetectedRetriggerable(false);
    button.setLongClickDetectedHandler(longClickDetected);
    button.setClickHandler(shortClick);

    battery.update();

    log_i("Initializing the load cell ...");
    LoadCell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN, 32);
    LoadCell.set_scale(config.get_scaleFactor());
    LoadCell.get_units(10);

    log_v("Tarring ...");
    delay(500);
    LoadCell.tare(10);

    if(config.get_startingMode() == 0)
    {
        log_i("Initializing BlueTooth...");
        ble_scale.init(config.get_name());

        stateMode = BLE_BINDING;
    } else if(config.get_startingMode() == 1) {
        log_i("Initializing the Web Server...");
        serverWiFi.begin(config.get_name(), config.get_password());
        
        stateMode = WIFI;
    }

    attachInterrupt(LOADCELL_DOUT_PIN, read_loadCell_ISR, FALLING);
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
    if(ble_scale.isConnected())
    {
        while (WeightValueAvailable > 0)
        {
            WeightValueAvailable--;
                ble_scale.updateWeight(WeightValue[WeightValueAvailable]>0 ?WeightValue[WeightValueAvailable]:-WeightValue[WeightValueAvailable]);
            
            log_d("Weight : %.2f kg", WeightValue[WeightValueAvailable]/1000);
        }
    }
    
    if(serverWiFi.isServerStarted())
    {
        EVERY_N_MILLISECONDS(100)
        {
            if(WeightValueAvailable > 0)
            {
                serverWiFi.sendWeight(WeightValue, timestamp, WeightValueAvailable, battery.getLevel());
                WeightValueAvailable = 0;
            }
        }
    }
    
    // update the connection status
    if (!ble_scale.isConnected() && oldDeviceConnected)
    {
        stateMode = BLE_BINDING;
        oldDeviceConnected = false;
    }

    if (ble_scale.isConnected() && !oldDeviceConnected)
    {
        stateMode = BLE_MEASUREMENT;
        oldDeviceConnected = true;
    }
    // Update the battery Level every second
    EVERY_X_SECONDS(1)
    {
        battery.update();

        if(ble_scale.isConnected())
            ble_scale.updateBatteryLevel(battery.getLevel());

        log_d("Battery Level : %d\%, Analog Read : %d", battery.getLevel(), battery.getRawValue());
    }

    // Update the LED color
    switch (stateMode)
    {
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
                if(battery.getLevel()>50)
                {
                    myLED[0].r = 0xFF*(100-battery.getLevel())/50;
                    myLED[0].g = 0xFF;
                    myLED[0].b = 0;
                }
                else
                {
                    myLED[0].r = 0xFF;
                    myLED[0].g = 0xFF*battery.getLevel()/50;
                    myLED[0].b = 0;
                }
            }
        }
        break;
    }

    FastLED.show();
}


void IRAM_ATTR read_loadCell_ISR() 
{
    int64_t time = esp_timer_get_time();
    if(WeightValueAvailable == WEIGHT_DATA_LENGHT)
    {
        for(int i=1; i<WEIGHT_DATA_LENGHT; i++)
        {
            WeightValue[i-1] = WeightValue[i];
            timestamp[i-1] = timestamp[i];
        }
        WeightValue[WEIGHT_DATA_LENGHT-1] = LoadCell.get_units();
        timestamp[WEIGHT_DATA_LENGHT-1] = time;
    }
    else
    {
        WeightValue[WeightValueAvailable] = LoadCell.get_units();
        timestamp[WeightValueAvailable] = time;
        WeightValueAvailable++;
    }
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
        case BLE_BINDING:
            detachInterrupt(LOADCELL_DOUT_PIN);
            ble_scale.stopAdvertising();
            serverWiFi.begin(config.get_name(), config.get_password());
            attachInterrupt(LOADCELL_DOUT_PIN, read_loadCell_ISR, FALLING);
            stateMode = WIFI;
        break;

        case WIFI:
            detachInterrupt(LOADCELL_DOUT_PIN);
            serverWiFi.stop();
            ble_scale.startAdvertising();
            attachInterrupt(LOADCELL_DOUT_PIN, read_loadCell_ISR, FALLING);
            stateMode = BLE_BINDING;
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
        default:
        break;
    }
}