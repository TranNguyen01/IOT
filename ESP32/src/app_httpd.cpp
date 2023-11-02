#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_camera.h"
#include "fb_gfx.h"
#include "sdkconfig.h"
#include "camera_index.h"
#include <esp_vfs.h>
#include "SPIFFS.h"
#include <WiFi.h>
#include <nvs_flash.h>
#include <cJSON.h>
#include "nvs.h"

#define FILE_PATH "/index.html"
#define CHUNK_SIZE 1024

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#endif

httpd_handle_t stream_httpd = NULL;
httpd_handle_t camera_httpd = NULL;
#define CHUNK_SIZE 1024
static esp_err_t index_handler(httpd_req_t *req)
{
    if(!SPIFFS.begin(true)){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return ESP_FAIL;
    }
  
    File file = SPIFFS.open("/index.html");
    if(!file || !file.available()){
        Serial.println("Failed to open file for reading or file is empty");
        return ESP_FAIL;
    }

    char buf[CHUNK_SIZE];
    ssize_t read_bytes;
    Serial.println("File Content:");
    while(file.available()){
        memset(buf, 0, sizeof(buf));  // Clear the buffer
        read_bytes = file.readBytes(buf, sizeof(buf) - 1);  // Leave room for null terminator
        if (read_bytes > 0) {
            httpd_resp_send_chunk(req, buf, read_bytes);
        }
    }
    file.close();
    httpd_resp_send_chunk(req, NULL, 0);  // End the chunked response
    return ESP_OK;
}

static esp_err_t get_ssid_handler(httpd_req_t *req)
{
    String ssids = "[";
    int n = WiFi.scanNetworks();
    for(int i =0; i < n; i++){
        ssids += ("\"" + WiFi.SSID(i) + "\"");
    }
    ssids += "]";
    char* char_array = new char[ssids.length() + 1];
    strcpy(char_array, ssids.c_str());
    httpd_resp_send(req, char_array, strlen(char_array)); 
    delete[] char_array;  // End the chunked response
    return ESP_OK;
}

static esp_err_t get_default_setting_handler(httpd_req_t *req)
{
    NvsData setting = load_data();
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "ssid", setting.ssid);
    cJSON_AddStringToObject(root, "password", setting.password);
    cJSON_AddStringToObject(root, "botToken", setting.botToken);
    cJSON_AddStringToObject(root, "messageID", setting.messageID);
    cJSON_AddStringToObject(root, "blynkAuthToken", setting.blynkAuthToken);

    char *json = cJSON_Print(root);
    httpd_resp_send(req, json, strlen(json));
    free(json);
    cJSON_Delete(root);
}

static esp_err_t post_save_handler(httpd_req_t *req){
    // Buffer to store the data
    char* buf;

    // Allocate memory for the buffer
    buf = (char*)malloc(req->content_len + 1);
    if (buf == NULL) {
        return ESP_FAIL;
    }

    // Receive the data
    int ret = httpd_req_recv(req, buf, req->content_len);
    if (ret <= 0) {  // 0 return means connection closed, <0 means error
        free(buf);
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            // Retry receiving if timeout occurred
            return ESP_ERR_TIMEOUT;
        }
        return ESP_FAIL;
    }

    char ssid[32];
    char password[32];
    char botToken[64];
    char messageID[64];
    char blynkAuthToken[64];

    cJSON *root = cJSON_Parse(buf);
    strcpy(ssid, cJSON_GetObjectItem(root,"ssid")->valuestring);
    strcpy(password, cJSON_GetObjectItem(root,"password")->valuestring);
    strcpy(botToken, cJSON_GetObjectItem(root,"botToken")->valuestring);
    strcpy(messageID, cJSON_GetObjectItem(root,"messageID")->valuestring);
    strcpy(blynkAuthToken, cJSON_GetObjectItem(root,"blynkAuthToken")->valuestring);
    cJSON_Delete(root);
    // Free the buffer
    free(buf);
  
    nvs_handle_t my_handle;
    nvs_open("storage", NVS_READWRITE, &my_handle);

    // Write
    nvs_set_str(my_handle, "ssid", ssid);
    nvs_set_str(my_handle, "password", password);
    nvs_set_str(my_handle, "botToken", botToken);
    nvs_set_str(my_handle, "messageID", messageID);
    nvs_set_str(my_handle, "blynkAuthToken", blynkAuthToken);
    nvs_set_u8(my_handle, "hasData", 1);

    // Commit written value
    nvs_commit(my_handle);

    // Close
    nvs_close(my_handle);

    // Send response
    const char* resp_str = "{\"success\": true, \"message\": \"Your data have been saved!\"}";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}

void startCameraServer()
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 16;

    httpd_uri_t index_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = index_handler,
        .user_ctx = NULL
    };

    httpd_uri_t get_ssid_uri = {
        .uri = "/ssid",
        .method = HTTP_GET,
        .handler = get_ssid_handler,
        .user_ctx = NULL
    };

    httpd_uri_t save_data_uri = {
        .uri = "/save",
        .method = HTTP_POST,
        .handler = post_save_handler,
        .user_ctx = NULL
    };

    log_i("Starting web server on port: '%d'", config.server_port);
    if (httpd_start(&camera_httpd, &config) == ESP_OK)
    {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &get_ssid_uri);
        httpd_register_uri_handler(camera_httpd, &save_data_uri);
    }
}

void setupLedFlash(int pin) 
{
}
