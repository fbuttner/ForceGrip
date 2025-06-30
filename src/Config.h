#pragma once

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

class Config
{
    private:
        /* data */
        const char *filename = "/config.json";
        JsonDocument doc;

        String Name, Password;
        uint8_t startingMode;
        float ScaleFactor;
    public:
        bool readConfig();
        bool writeConfig();

        float get_scaleFactor() {return ScaleFactor;};
        void set_scaleFactor(float val) {ScaleFactor = val;doc["ScaleFactor"] = val;};

        uint8_t get_startingMode() {return startingMode;};
        void set_startingMode(uint8_t val) {startingMode = val;doc["StartMode"] = val;};

        const char* get_name() {return Name.c_str();};
        void set_name(const String &val) {Name = val;doc["Name"] = val;};

        const char*  get_password() {return Password.c_str();};
        void set_password(const String &val) {Password = val;doc["Password"] = val;};
};
