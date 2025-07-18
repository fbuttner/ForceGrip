#include "ServerWiFi.h"

void ServerWiFi::begin()
{
    setAP_SSID(config.get_name());
    setAP_password(config.get_password());

    WiFi.mode(WIFI_AP_STA);
    WiFi.setAutoReconnect(true);
    //WiFi.setSleep(WIFI_PS_NONE);

    if(config.get_AP_Stored_count() > 0)
    {
        log_i("Start scanning for WiFi networks...");
        _current_scan_channel = 1;
        WiFi.scanNetworks(true, true, false, 1000, _current_scan_channel);
        _status |= SERVERWIFI_SCANING;
    }
    else
    {
        // No stored APs, go directly to SoftAP mode.
        WiFi.softAP(_AP_ssid, _AP_password);
        _status |= SERVERWIFI_AP_STARTED;
        
        log_i("SoftAP IP address: %s", WiFi.softAPIP().toString().c_str());

        createWebServer();
    }
    
    _startConnectingTime = millis();
}


void ServerWiFi::loop()
{
    if(!(_status&SERVERWIFI_CONNECTED) && !(_status&SERVERWIFI_AP_STARTED))
    {
        // Handle WiFi scanning state
        if(_status&SERVERWIFI_SCANING)
        {
            int16_t scan_result = WiFi.scanComplete();

            if(scan_result == WIFI_SCAN_FAILED)
            {
                log_w("Scan failed for channel %d", _current_scan_channel);
                // Treat as if no networks were found on this channel and move to the next.
                scan_result = 0; 
            }

            if(scan_result != WIFI_SCAN_RUNNING)
            {
                if (scan_result > 0)
                {
                    log_i("Found %d network(s) on channel %d:", scan_result, _current_scan_channel);
                    // A network was found, check if it's one of our stored networks.
                    for (int i = 0; i < scan_result; i++)
                    {
                        String found_ssid = WiFi.SSID(i);
                        log_i("\t- %s (%d)", found_ssid.c_str(), WiFi.RSSI(i));

                        for(int j = 0; j < config.get_AP_Stored_count(); j++)
                        {
                            if(found_ssid == config.get_AP_SSID(j)) {
                                log_i("Found matching stored network! Connecting to %s...", found_ssid.c_str());
                                WiFi.scanDelete(); // Free memory from scan results
                                
                                WiFi.begin(config.get_AP_SSID(j).c_str(), config.get_AP_password(j).c_str());
                                
                                _startConnectingTime = millis();
                                _status &= ~SERVERWIFI_SCANING;
                                _status |= SERVERWIFI_CONNECTING;

                                return; // Exit loop, will handle connection attempt on next iteration.
                            }
                        }
                    }
                }

                // Move to the next channel for scanning.
                _current_scan_channel++;

                // Assuming 2.4GHz band, channels 1-13/14 are common.
                if (_current_scan_channel > 14) 
                {
                    log_i("Finished scanning all channels. No matching stored network found.");
                    _status &= ~SERVERWIFI_SCANING;
                    WiFi.scanDelete();
                    // Fall through to create SoftAP.
                } else {
                    // Start scanning the next channel.
                    WiFi.scanDelete(); // Clear previous scan results.
                    log_d("Scanning next channel: %d", _current_scan_channel);
                    WiFi.scanNetworks(true, true, false, 1000, _current_scan_channel);
                }
            }
        }
        else
        {
            if(WiFi.status() == WL_CONNECTED)
            {
                _status |= SERVERWIFI_CONNECTED;
                _status &= ~SERVERWIFI_CONNECTING;

                log_i("Connected to WiFi. IP: %s", WiFi.localIP().toString().c_str());
            }
            else if(millis() - _startConnectingTime > 10000)
            {
                log_i("WiFi connection Timed out.");
                _status &= ~SERVERWIFI_CONNECTING;
            }
        }

        // After scanning/connecting attempts, if not connected, start SoftAP.
        if(!(_status&(SERVERWIFI_CONNECTED|SERVERWIFI_SCANING|SERVERWIFI_CONNECTING)))
        {
            log_w("Failed to connect to WiFi. Starting SoftAP mode.");

            WiFi.softAP(_AP_ssid, _AP_password);
            _status |= SERVERWIFI_AP_STARTED;
            
            log_i("SoftAP IP address: %s", WiFi.softAPIP().toString().c_str());
            log_i("WiFi status: %d", WiFi.status());
        }
    }
    else if(!(_status&SERVERWIFI_SERVERSTARTED))
        createWebServer();
}

void ServerWiFi::stop()
{
    MDNS.end();
    log_i("MDNS responder stopped");

    if(WiFi.isConnected())
        WiFi.disconnect();

    if(_webSocket != nullptr)
    {
        log_d("Stopping webSocket...");
        _webSocket->closeAll();
    }

    if(_server != nullptr)
    {
        log_d("Stopping server...");
        _server->end();
    }

    _status = 0;
    
    log_i("Web Server stopped");
}

void ServerWiFi::setAP_SSID(const char* userName=nullptr)
{
    if((userName != nullptr) && (strlen(userName) == 0))
    {
        uint8_t mac[6];
        char macStr[7] = { 0 };
        
        esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);

        sprintf(macStr, "%02X%02X%02X", mac[3], mac[4], mac[5]);
    
        _AP_ssid = "ForceGrip_";
        _AP_ssid += macStr;
    }
    else
    {
        _AP_ssid = "ForceGrip_";
        _AP_ssid += userName;
    }
}

AsyncWebSocket::SendStatus ServerWiFi::sendWeight(float *weightInGram, int64_t *timestamp, uint8_t size, uint8_t batteryLevel)
{
    uint16_t msgLenght = 12*size+1;

    uint8_t message[msgLenght];
    memcpy(message, weightInGram, size*sizeof(float));
    memcpy(message + size*sizeof(float), timestamp, size*sizeof(int64_t));
    memcpy(message + size*(sizeof(float)+sizeof(int64_t)), &batteryLevel, 1);

    return _webSocket->binaryAll(message, msgLenght);
}

void ServerWiFi::createWebServer()
{
    // starting mDNS
    if (!MDNS.begin("forcegrip"))
    {
        log_w("Error setting up MDNS responder!");
    }
    else
    {
        log_i("MDNS responder started");
        MDNS.addService("http", "tcp", 80);
    }

    // Adding Websocket capability to the server
    if(_webSocket == nullptr)
    {
        _webSocket = new AsyncWebSocket("/ws");
        
        log_d("WebSockets adding onEvent");
        _webSocket->onEvent(ServerWiFi::onWsEvent);
    }

    if(_server == nullptr)
    {
        _server = new AsyncWebServer(80);

        log_i("Web Server adding page");
        _server->on("/", HTTP_ANY, [](AsyncWebServerRequest *request){
            log_i("index.html requested");
            bool configChanged = false;

            if (request->hasParam("New_WiFi_SSID", true) && request->hasParam("New_WiFi_password", true))
            {
                String _ssid = request->getParam("New_WiFi_SSID", true)->value();
                String _password = request->getParam("New_WiFi_password", true)->value();
                
                if(_ssid.length() > 0 && _password.length() > 0)
                {
                    config.add_AP(request->getParam("New_WiFi_SSID", true)->value(), 
                                    request->getParam("New_WiFi_password", true)->value());
                    configChanged = true;
                    
                    //response += "\nNew WiFi Network added";
                    log_i("New WiFi Network added");
                }
            }

            if(request->hasParam("delete", true))
            {
                int index = request->getParam("delete", true)->value().toInt();

                config.remove_AP(index);
                configChanged = true;
                
                log_i("AP %d removed", index);
            }


            if(configChanged)
                config.writeConfig();

            request->send(SPIFFS, "/index.html", "text/html", false, ServerWiFi::HTML_processor);
        });
        _server->on("/config", HTTP_ANY, [](AsyncWebServerRequest *request){
            log_i("config requested");
            String response;

            // processing the post data
            if (request->hasParam("tare", true) || request->hasParam("tare", false))
            {
                detachInterrupt(LOADCELL_DOUT_PIN);
                LoadCell.tare(10);
                attachInterrupt(LOADCELL_DOUT_PIN, read_loadCell_ISR, FALLING);
                response = "Tare done";
                log_d("Tare done");
            }

            bool configChanged = false;
            if (request->hasParam("device_name", true))
            {
                String _device_name = request->getParam("device_name", true)->value();

                config.set_name(_device_name);
                configChanged = true;

                response += "\nDevice name set to: " + _device_name;
                log_d("Device name set to: %s", _device_name);
            }

            if (request->hasParam("scale_factor", true))
            {
                float _scaleFactor = request->getParam("scale_factor", true)->value().toFloat();
                
                config.set_scaleFactor(_scaleFactor);
                LoadCell.set_scale(_scaleFactor);
                configChanged = true;

                response += "\nScale factor set to: " + String(_scaleFactor);
                log_d("Scale factor set to: %f", _scaleFactor);
            }

            if (request->hasParam("startup_mode", true))
            {
                int _startup_mode = request->getParam("startup_mode", true)->value().toInt();

                config.set_startingMode(_startup_mode);
                configChanged = true;

                response += "\nStartup mode set to: ";
                response += (_startup_mode?(_startup_mode==1?"WIFI AP":"WIFI"):"BLE");
                log_d("Startup mode set to: %d", _startup_mode);
            }

            if (request->hasParam("old_password", true) && request->hasParam("new_password", true) && request->hasParam("new_password_bis", true))
            {
                String _old_password = request->getParam("old_password", true)->value();
                String _new_password = request->getParam("new_password", true)->value();
                String _new_password_bis = request->getParam("new_password_bis", true)->value();

                if (_old_password == config.get_password() && _new_password == _new_password_bis) {
                    config.set_password(_new_password);
                    configChanged = true;

                    response += "\nPassword changed";
                    log_d("Password changed");
                }
                else {
                    response += "\nPassword not changed";
                    log_d("Password not changed");
                }
            }

            if (request->hasParam("New_WiFi_SSID", true) && request->hasParam("New_WiFi_password", true))
            {
                config.add_AP(request->getParam("New_WiFi_SSID", true)->value(), 
                                request->getParam("New_WiFi_password", true)->value());
                configChanged = true;
                
                response += "\nNew WiFi Network added";
                log_d("New AP added");
            }

            if (request->hasParam("Calibration", true))
            {
                float calibration_weight = request->getParam("Calibration", true)->value().toFloat()*1000;
                
                detachInterrupt(LOADCELL_DOUT_PIN);
                
                double val = LoadCell.get_value(10);
                float _scaleFactor = val / calibration_weight;
                LoadCell.set_scale(_scaleFactor);
                attachInterrupt(LOADCELL_DOUT_PIN, read_loadCell_ISR, FALLING);

                config.set_scaleFactor(_scaleFactor);
                configChanged = true;
                
                response += "\nCalibration value set to: " + String(_scaleFactor);
                log_d("Calibration value set to: %f", _scaleFactor);
            }

            if(configChanged)
                config.writeConfig();

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
            for (int i = 0; i < request->params(); i++) {
                const AsyncWebParameter *p = request->getParam(i);
                log_d("POST[%s]: %s", p->name().c_str(), p->value().c_str());
            }
#endif
            request->send(200, "text/plain", response);
        });
        _server->serveStatic("/", SPIFFS, "/");

        log_d("Web Server adding Handler");
        _server->addHandler(_webSocket);
    }

    // Starting the server
    _server->begin();
    _status |= SERVERWIFI_SERVERSTARTED;

    log_i("Web Server started");
}

void ServerWiFi::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                            AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        log_i("Client WebSocket connecté");
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        log_i("Client WebSocket déconnecté");
    }
}

String ServerWiFi::HTML_processor(const String& var)
{
    if(var == "DEVICE_NAME")
        return config.get_name();
    else if(var == "DEVICE_PASSWORD")
        return config.get_password();
    else if(var == "SCALE_FACTOR")
        return String(config.get_scaleFactor());
    else if(var == "DEFAULT_MODE")
        return String(config.get_startingMode());
    else if(var == "STARTUP_MODE_BLE_SELECTED") 
        return (config.get_startingMode() == 0) ? "selected" : "";
    else if(var == "STARTUP_MODE_WIFI_SELECTED")
        return (config.get_startingMode() == 1) ? "selected" : "";
    else if(var == "WIFI_NETWORKS")
    {
        String s_networks = "";
        if(config.get_AP_Stored_count() > 0)
        {
            for(int i = 0; i < config.get_AP_Stored_count(); i++)
            {
                s_networks += "<div class=\"wifi-item\">";
                s_networks += "<span>" + config.get_AP_SSID(i) + "</span>";
                s_networks += "<button type=\"submit\" name=\"delete\" value=\"" + String(i) + "\">X</button>";
                s_networks += "</div>";
            }
        }
        else
        {
            s_networks += "<span>No Network stored</span>";
        }
        return s_networks;
    }
    
    return String();
}
