#include "nvs.h"

NvsData load_data() {
    // Open
    nvs_handle_t my_handle;
    nvs_open("storage", NVS_READONLY, &my_handle);

    uint8_t hasData;
    char ssid[32];
    char password[32];
    char botToken[64];
    char botToken1[64];
    char messageID[64];
    char blynkAuthToken[64];
    // Read
    size_t required_size;  // Adjust according to your needs
    nvs_get_str(my_handle, "ssid", NULL, &required_size);
    nvs_get_str(my_handle, "ssid", ssid, &required_size);
        
    nvs_get_str(my_handle, "password", NULL, &required_size);
    nvs_get_str(my_handle, "password", password, &required_size);

    nvs_get_str(my_handle, "botToken", NULL, &required_size);
    nvs_get_str(my_handle, "botToken", botToken, &required_size);

    nvs_get_str(my_handle, "messageID", NULL, &required_size);
    nvs_get_str(my_handle, "messageID", messageID, &required_size);

    nvs_get_str(my_handle, "blynkAuthToken", NULL, &required_size);
    nvs_get_str(my_handle, "blynkAuthToken", blynkAuthToken, &required_size);

    nvs_get_u8(my_handle, "hasData", &hasData);

    Serial.println();
    Serial.println("Data--------");
    Serial.println(ssid);
    Serial.println(password);
    Serial.println(botToken);
    Serial.println(messageID);
    Serial.println(blynkAuthToken);
    Serial.println(hasData);
    Serial.println("Data--------");

    // Close
    nvs_close(my_handle);
    NvsData data;
    data.hasData = hasData;
    strncpy(data.ssid, ssid, sizeof(data.ssid));
    strncpy(data.password, password, sizeof(data.password));
    strncpy(data.botToken, botToken, sizeof(data.botToken));
    strncpy(data.messageID, messageID, sizeof(data.messageID));
    strncpy(data.blynkAuthToken, blynkAuthToken, sizeof(data.blynkAuthToken));
    return data;
}


void erase_nvs_flash() {

    nvs_handle_t my_handle;
    nvs_open("storage", NVS_READWRITE, &my_handle);
    nvs_erase_key(my_handle, "ssid");
    nvs_erase_key(my_handle, "password");
    nvs_erase_key(my_handle, "botToken");
    nvs_erase_key(my_handle, "messageID");
    nvs_erase_key(my_handle, "hasData");
    nvs_erase_key(my_handle, "blynkAuthToken");
    esp_err_t  err = nvs_set_u8(my_handle, "hasData", 0);
    if (err != ESP_OK) {
        Serial.println("Error setting value!\n");
    }
    nvs_commit(my_handle);
    nvs_close(my_handle);
}