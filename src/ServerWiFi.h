#pragma once

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include "BLE_Scale.h"
#include "Battery.h"
#include "Config.h"
#include "HX711.h"

extern BLE_Scale bleScale;
extern Battery battery;
extern Config config;
extern HX711 LoadCell;
extern const int LOADCELL_DOUT_PIN;
extern void read_loadCell_ISR();

class ServerWiFi {
public:
    ServerWiFi(){_serverStarted = false; _server = nullptr; _webSocket = nullptr;};
    void begin(const char* ssid, const char* password);
    void stop();
    ~ServerWiFi(){stop();};
    bool isServerStarted(){return _serverStarted;};
    AsyncWebSocket::SendStatus sendWeight(float *weightInGram, int64_t *timestamp, uint8_t size, uint8_t batteryLevel);

private:
    const char* _ssid;
    const char* _password;
    AsyncWebServer* _server=nullptr;
    AsyncWebSocket* _webSocket=nullptr;
    bool _serverStarted=false;

    static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    static String HTML_processor(const String& var);
};
