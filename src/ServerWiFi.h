#pragma once

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <AsyncWebSocket.h>

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


enum _SERVERWIFI_STATUS_FLAGS_ {
    SERVERWIFI_SCANING       = 0b00000001,
    SERVERWIFI_CONNECTING    = 0b00000010,
    SERVERWIFI_CONNECTED     = 0b00000100,
    SERVERWIFI_AP_STARTED    = 0b00001000,
    SERVERWIFI_SERVERSTARTED = 0b10000000,
};

class ServerWiFi
{
public:
    ServerWiFi()    {_status=0; _server = nullptr; _webSocket = nullptr;};
    ~ServerWiFi()   {stop();};

    void begin();
    void stop();
    void loop();

    bool isServerStarted()  {return _status&SERVERWIFI_SERVERSTARTED;};
    bool asWebSocketClient()    {return _webSocket->count()>0;};

    void setAP_SSID(const char* userName);
    void setAP_password(const char* password)  {_AP_password = password;};

    uint8_t get_status()    {return _status;};

    AsyncWebSocket::SendStatus sendWeight(float *weightInGram, int64_t *timestamp, uint8_t size, uint8_t batteryLevel);
    
private:
    String _AP_ssid;
    String _AP_password;
    AsyncWebServer* _server;
    AsyncWebSocket* _webSocket;
    unsigned long _startConnectingTime;
    unsigned long _lastUpdate;
    uint8_t _status;
    uint8_t _current_scan_channel;

    void createWebServer();
    static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    static String HTML_processor(const String& var);
};
