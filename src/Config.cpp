#include "Config.h"

bool Config::readConfig()
{
    File configFile = SPIFFS.open(filename, "r");
    if (!configFile)
    {
        log_d("Failed to open config file");
        return false;
    }

    DeserializationError error = deserializeJson(doc, configFile);
    if(error)
    {
        log_d("Failed to parse c, default configuration\n%s", error.c_str());

        configFile.close();

        return false;
    }
    
    Name = doc["Name"].as<String>();
    Password = doc["Password"].as<String>();
    ScaleFactor = doc["ScaleFactor"].as<float>();
    startingMode = doc["StartMode"].as<uint8_t>();

    AP_Count = doc["AP_list"].size();
    
    configFile.close();
    log_i("Configuration loaded successfully.");

    return true;
}

bool Config::writeConfig()
{
    File configFile = SPIFFS.open(filename, "w");
    if (!configFile)
    {
        log_d("Failed to open config file for writing");
        return false;
    }

    if (serializeJsonPretty(doc, configFile) == 0)
    {
        log_d("Failed to write to file");
        return false;
    }

    configFile.close();
    log_i("Configuration saved successfully.");

    return true;
}

void Config::remove_AP(uint8_t index)
{
    if(index < AP_Count)
    {
        JsonArray apList = doc["AP_list"];
        apList.remove(index);
        
        AP_Count--;
    }
}

void Config::add_AP(const String &ssid, const String &password)
{
    doc["AP_list"][AP_Count]["SSID"] = ssid;
    doc["AP_list"][AP_Count]["Password"] = password;

    AP_Count++;
}