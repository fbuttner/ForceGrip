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
    
    return false;
}