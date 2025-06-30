#include "ServerWiFi.h"

void ServerWiFi::begin(const char* ssid, const char* password)
{
    _ssid = ssid;
    _password = password;

    WiFi.softAP(_ssid, _password);

    log_i("SoftAP IP address: %s", WiFi.softAPIP().toString().c_str());

    if((_server == nullptr) || (_webSocket == nullptr))
    {
        if(_server == nullptr)
            _server = new AsyncWebServer(80);
        
        if(_webSocket == nullptr)
            _webSocket = new AsyncWebSocket("/ws");
            
        log_d("WebSockets adding onEvent");
        _webSocket->onEvent(ServerWiFi::onWsEvent);

        log_d("Web Server adding Handler");
        _server->addHandler(_webSocket);

        log_d("Web Server adding page");
        _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            log_d("index.html requested");
            request->send(SPIFFS, "/index.html", "text/html", false, ServerWiFi::HTML_processor);
        });
        _server->on("/config", HTTP_ANY, [](AsyncWebServerRequest *request){
            log_d("config requested");
            String response;

            // processing the post data
            if (request->hasParam("tare", true) || request->hasParam("tare", false)) {
                detachInterrupt(LOADCELL_DOUT_PIN);
                LoadCell.tare(10);
                attachInterrupt(LOADCELL_DOUT_PIN, read_loadCell_ISR, FALLING);
                response = "Tare done";
                log_d("Tare done");
            }

            bool configChanged = false;
            if (request->hasParam("device_name", true) || request->hasParam("device_name", false)) {
                String _device_name = request->getParam("device_name", true)->value();

                config.set_name(_device_name);
                configChanged = true;

                response += "\nDevice name set to: " + _device_name;
                log_d("Device name set to: %s", _device_name);
            }
            if (request->hasParam("scale_factor", true) || request->hasParam("scale_factor", false)) {
                float _scaleFactor = request->getParam("scale_factor", true)->value().toFloat();
                
                config.set_scaleFactor(_scaleFactor);
                LoadCell.set_scale(_scaleFactor);
                configChanged = true;

                response += "\nScale factor set to: " + String(_scaleFactor);
                log_d("Scale factor set to: %f", _scaleFactor);
            }
            if (request->hasParam("startup_mode", true) || request->hasParam("startup_mode", false)) {
                int _startup_mode = request->getParam("startup_mode", true)->value().toInt();

                config.set_startingMode(_startup_mode);
                configChanged = true;

                response += "\nStartup mode set to: ";
                response += (_startup_mode?(_startup_mode==1?"WIFI AP":"WIFI"):"BLE");
                log_d("Startup mode set to: %d", _startup_mode);
            }
            if ((request->hasParam("old_password", true) || request->hasParam("old_password", false)) &&
                (request->hasParam("new_password", true) || request->hasParam("new_password", false)) &&
                (request->hasParam("new_password_bis", true) || request->hasParam("new_password_bis", false))) {
                
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
            if (request->hasParam("Calibration", true) || request->hasParam("Calibration", false)) {
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

            /*
            */
            // echo post param
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
            for (int i = 0; i < request->params(); i++) {
                const AsyncWebParameter *p = request->getParam(i);
                log_d("POST[%s]: %s", p->name().c_str(), p->value().c_str());
            }
#endif
            request->send(200, "text/plain", response);
            //request->send(SPIFFS, "/index.html", "text/html", false, HTML_processor);
        });
        _server->serveStatic("/", SPIFFS, "/");
    }
    _server->begin();
    _serverStarted = true;
    log_i("Web Server started");
}


void ServerWiFi::stop()
{
    if(_webSocket != nullptr)
    {
        log_d("Stopping webSocket...");
        _webSocket->closeAll();
    }

    if(_server != nullptr)
    {
        log_d("Stopping server...");
        _server->end();
        //delete _server;
        //_server = nullptr;
    }
    _serverStarted = false;
    log_i("Web Server stopped");
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

void ServerWiFi::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
                            AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    log_i("Client WebSocket connecté");
  } else if (type == WS_EVT_DISCONNECT) {
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
    else if(var == "STARTUP_MODE_WIFI_AP_SELECTED")
        return (config.get_startingMode() == 1) ? "selected" : "";
    else if(var == "STARTUP_MODE_WIFI_SELECTED")
        return (config.get_startingMode() == 2) ? "selected" : "";
    
    return String();
}
