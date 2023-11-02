#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ZMCT103.h>
#include <ZMPT101B.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define VSENSITIVITY 365.0f
#define ECSENSITIVITY 0.62f

// StaticJsonDocument<2> doc;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ZMPT101B voltageSensor(A1, 50.0);
ZMCT103 ampSensor(A0, 50);

// DynamicJsonDocument receivedDoc(1024);
StaticJsonDocument<24> doc;

void setup() {
  Serial.begin(115200);    // start the serial port

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("error");
  }
  display.display();
  display.setTextColor(SSD1306_WHITE);

  voltageSensor.setSensitivity(VSENSITIVITY);
  ampSensor.setSensitivity(ECSENSITIVITY);
  //Setup pinMode
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
}

void Switch(uint8_t pin, uint8_t newValue){
  digitalWrite(pin, newValue);
}

void readMessage(){
  StaticJsonDocument<96> receivedDoc; 
  const auto deser_err = deserializeJson(receivedDoc, Serial);
  if(!deser_err){
    String function = receivedDoc["function"];
    if(function == "Switch" ){
      uint8_t pin = receivedDoc["params"]["pin"];
      uint8_t newValue = receivedDoc["params"]["value"];
      Switch(pin, newValue);
    }
  }
}

void loop() {
  float vrms = 0, irms = 0, prms = 0;

  vrms = voltageSensor.getRmsVoltage();
  irms = ampSensor.getRmsAmp();

  vrms = round(vrms);
  irms = round(irms * 100) / 100.0;
  prms = round(vrms * irms);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  
  display.print("U:");
  display.print(vrms, 0);
  display.println("V");

  display.setTextSize(1);
  display.println();

  display.setTextSize(2);
  display.print("I:");
  display.print(irms);
  display.println("A");

  display.setTextSize(1);
  display.println();

  display.setTextSize(2);
  display.print("P:");
  display.print(prms, 0);
  display.println("W");
  display.display();

  doc["volt"] = vrms;
  doc["amp"]  = irms;
  doc["power"] = prms;

  serializeJson(doc, Serial);
  delay(1000);
}




//424.5000000000
//348.2500000000


//365.7500000000
