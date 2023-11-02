#include <Arduino.h>

void setup() {
 Serial.begin(115200);

}
int time = 0;
void loop() {
  int min = 10001, max = 0;
  unsigned long startTime = micros();
  while(micros() - startTime < 40000){
    int sensorValue = analogRead(A0);
    min = min > sensorValue ? sensorValue : min;
    max = max < sensorValue ? sensorValue : max;
  }
  Serial.print(max - min);
  Serial.print(",");
  time++;
  if(time == 200){
    Serial.println();
    time =0;
  }
}
