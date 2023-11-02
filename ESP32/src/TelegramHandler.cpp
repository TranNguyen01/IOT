#include "TelegramHandler.h"

String sendPhotoTelegram(String botToken, String chatID) {
  WiFiClientSecure clientTCP;
  clientTCP.setCACert(TELEGRAM_CERTIFICATE_ROOT); 
  const char* telegramDoimain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  //Dispose first picture because of bad quality
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  esp_camera_fb_return(fb); // dispose the buffered image
  
  // Take a new photo
  fb = NULL;  
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return "{\"ok\":false,\"mesage\": \"Camera capture failed\"}";
  }  
  
  Serial.println("Connect to " + String(telegramDoimain));


  if (clientTCP.connect(telegramDoimain, 443)) {
    Serial.println("Connection successful");

    String textContentString = "--SmartSocket\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + chatID 
        + "\r\n--SmartSocket\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tailString = "\r\n--SmartSocket--\r\n";
    size_t imageLen = fb->len;
    size_t extraLen = textContentString.length() + tailString.length();
    size_t totalLen = imageLen + extraLen;

    clientTCP.println("POST /bot"+botToken+"/sendPhoto HTTP/1.1");
    clientTCP.println("Host: " + String(telegramDoimain));
    clientTCP.println("Content-Length: " + String(totalLen));
    clientTCP.println("Content-Type: multipart/form-data; boundary=SmartSocket");
    clientTCP.println();
    clientTCP.print(textContentString);
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
        if (n+1024<fbLen) {
            clientTCP.write(fbBuf, 1024);
            fbBuf += 1024;
        }
        else if (fbLen%1024>0) {
            size_t remainder = fbLen%1024;
            clientTCP.write(fbBuf, remainder);
        }
    }  
    clientTCP.print(tailString);
    esp_camera_fb_return(fb);
    
    int waitTime = 5000;   // timeout 5 seconds
    long startTimer = millis();
    boolean state = false;
    while (clientTCP.availableForWrite())
    {
      /* code */
    }
    
    // while ((startTimer + waitTime) > millis()){
    //   Serial.print(".");
    //   delay(100);      
    //   while (clientTCP.available()) {
    //     char c = clientTCP.read();
    //     if (state==true) getBody += String(c);        
    //     if (c == '\n') {
    //       if (getAll.length()==0) state=true; 
    //       getAll = "";
    //     } 
    //     else if (c != '\r')
    //       getAll += String(c);
    //       startTimer = millis();
    //   }
    //   if (getBody.length()>0) break;
    // }
    clientTCP.stop();
  }
  else {
    getBody= "{\"ok\":false,\"mesage\": \"Camera capture failed\"}";
    Serial.println("Connected to api.telegram.org failed.");
  }
  return getBody;
}