#include "esp_camera.h"
#define BLYNK_TEMPLATE_ID "TMPL6-Lf-9xW8"
#define BLYNK_TEMPLATE_NAME "SmartSocket"

#include <WiFi.h>
#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <BlynkSimpleEsp32.h>
#include "TelegramHandler.h"
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"
#include "nvs.h"

// DEFINE_CONNECTION_SETTING
char *ssid = "VNPT_CHI BINH";
char *password = "77777777";
char *blynkAuthToken = "1whkAJMV9AlaM97me0C4gyOcSlcMuvBn";
String botToken = "6226362939:AAFPJ8VMQToTbmOMmmDc9pY5szMkQ74Ra30";
String chatID = "6495659737";
BlynkTimer timer;

// CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22
#define FLASH_PIN 14
#define FLASH_LED 4

// DEFINE_BLYNK_VIRTUAL_PIN
//  #define   VOLT            V0
#define AMP V1
#define POWER V2
#define MAX_POWER V3
#define WARNING_POWER V4
#define SYNC_STATE V0

// DEFINE_WIFI_SETTING
#define AP_SSID "Smart Socket"
#define AP_PASSWORD "12345678"

// DECLARE_GLOBAL_VIRIALBLE
bool relayState = true;
bool sendImage = false;
bool syncState = true;
int maxPower = 100;
int warningPower = 500;
int volt = 0;
float amp = 0.0f, power = 0.0f;
bool apMode = true;

// DEFINE_FUNCTION
void startCameraServer();
void flashLed();

void configInitCamera()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_UXGA;
    config.pixel_format = PIXFORMAT_JPEG;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 5;
    config.fb_count = 1;

    if (config.pixel_format == PIXFORMAT_JPEG)
    {
        if (psramFound())
        {
            config.jpeg_quality = 10;
            config.fb_count = 1;
            config.grab_mode = CAMERA_GRAB_LATEST;
        }
        else
        {
            // Limit the frame size when PSRAM is not available
            config.frame_size = FRAMESIZE_SVGA;
            config.fb_location = CAMERA_FB_IN_DRAM;
        }
    }
    else
    {
        // Best option for face detection/recognition
        config.frame_size = FRAMESIZE_240X240;
    }

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x", err);
        delay(1000);
        ESP.restart();
    }
}

void updateBlynk()
{
    // Blynk.virtualWrite(VOLT, volt);
    Blynk.virtualWrite(AMP, amp);
    Blynk.virtualWrite(POWER, power);
}

BLYNK_WRITE(MAX_POWER)
{
    // reads the slider value when it changes in the app
    maxPower = param.asInt();
}

BLYNK_WRITE(WARNING_POWER)
{
    // reads the slider value when it changes in the app
    warningPower = param.asInt();
}

BLYNK_WRITE(SYNC_STATE)
{
    // reads the slider value when it changes in the app
    syncState = (bool)param.asInt();
}

BLYNK_CONNECTED()
{
    Blynk.syncVirtual(MAX_POWER);
    Blynk.syncVirtual(WARNING_POWER);
    Blynk.syncVirtual(SYNC_STATE);
}

void setup()
{
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    // Init Serial Monitor
    Serial.begin(115200);

    // Config and init the camera
    configInitCamera();

    //Config GPIO pin
    pinMode(12, OUTPUT);
    digitalWrite(12, HIGH);
    pinMode(FLASH_LED, OUTPUT);
    digitalWrite(FLASH_LED, LOW);
    pinMode(14, INPUT);

    //Settup reset setting data 
    unsigned long pressTime = millis();
    bool flashPressed = false;
    bool flashState = (bool)digitalRead(FLASH_PIN);
    if (flashState)
        pressTime = millis();
    while (flashState && millis() - pressTime <= 3000)
    {
        flashState = digitalRead(FLASH_PIN);
        flashPressed = flashState;
        if (flashPressed == false)
        {
            break;
        }
    }
    if (flashPressed)
    {
        flashLed(2, 10);
        Serial.println("Flashing data");
        erase_nvs_flash();
        delay(1000);
        flashLed(4, 10);
        delay(1000);
        ESP.restart();
    }
    //Load data from nvs
    NvsData settingData = load_data();
    if (settingData.hasData)
    {
        apMode = false;
        ssid = settingData.ssid;
        password = settingData.password;
        botToken = settingData.botToken;
        chatID = settingData.messageID;
        blynkAuthToken = settingData.blynkAuthToken;
    }

    //Determine Wifi mode
    if (apMode)
    {
        //Wifi Access point and Station mode
        WiFi.mode(WIFI_AP_STA);
        WiFi.softAP("smart_socket");
        startCameraServer();
    }
    else
    {
        //Wifi station mode
       
        // Serial.print(ssid);
        // Serial.print("<==>");
        // Serial.println(password);

        // Serial.print(botToken);
        // Serial.print("<==>");
        // Serial.println(chatID);

        // Serial.println(blynkAuthToken);

        // Serial.print(strcmp(ssid, "VNPT_CHI BINH"));
        // Serial.print("<==>");
        // Serial.print(strcmp(password, "77777777"));
        // Serial.print("<==>");
        // Serial.print(botToken == "6226362939:AAFPJ8VMQToTbmOMmmDc9pY5szMkQ74Ra30");
        // Serial.print("<==>");
        // Serial.print(chatID == "6495659737");
        // Serial.print("<==>");
        // Serial.println(strcmp(blynkAuthToken, "1whkAJMV9AlaM97me0C4gyOcSlcMuvBn"));

        //Connect to Blynk server
        Blynk.begin(blynkAuthToken, ssid, password);
        timer.setInterval(1000L, updateBlynk);
        Serial.println(WiFi.localIP());
        flashLed(10, 10);
    }
}

StaticJsonDocument<200> doc;

void loop()
{
    Blynk.run();
    timer.run();
    
    if (Serial.available())
    {
        DeserializationError error = deserializeJson(doc, Serial);
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        }
        else
        {
            volt = doc["volt"];
            amp = doc["amp"];
            power = doc["power"];
            // Serial.print(volt);
            // Serial.print(",");
            // Serial.print(amp);
            // Serial.print(",");
            // Serial.println(volt * amp);
            if (!syncState && relayState)
            {
                digitalWrite(12, LOW);
                relayState = false;
                Blynk.virtualWrite(SYNC_STATE, (int)relayState);
            }

            int currentPower = volt * amp;

            if (currentPower >= maxPower && relayState)
            {
                digitalWrite(12, LOW);
                relayState = false;
                syncState = false;
                Blynk.virtualWrite(SYNC_STATE, (int)relayState);
            }
            else if (currentPower < maxPower && !relayState && syncState)
            {
                digitalWrite(12, HIGH);
                relayState = true;
                Blynk.virtualWrite(SYNC_STATE, (int)relayState);
            }

            if (currentPower >= warningPower && !sendImage)
            {
                sendPhotoTelegram(botToken, chatID);
                sendImage = true;
                while (!Serial.available())
                {
                  
                }
            }
            else if (currentPower < warningPower && sendImage)
            {
                sendImage = false;
            }
        }
    }
}

void flashLed(int times, int frequency){
    for (int i = 0; i < times; i++)
    {
        digitalWrite(FLASH_LED, HIGH);
         delayMicroseconds(500000/frequency);
        digitalWrite(FLASH_LED, LOW);
        delayMicroseconds(500000/frequency);
    }
}