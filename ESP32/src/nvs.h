#include <WiFi.h>
#include <Arduino.h>
#include <nvs_flash.h>

// DEFINE_DATA_TYPE
struct NvsData
{
    uint8_t hasData;
    char ssid[32];
    char password[32];
    char botToken[64];
    char messageID[64];
    char blynkAuthToken[64];
};

NvsData load_data();
void erase_nvs_flash();