#include "ZMCT103.h"  

/// @brief ZMPT101B constructor
/// @param pin analog pin that ZMPT101B connected to.
/// @param frequency AC system frequency
ZMCT103::ZMCT103(uint8_t pin, uint16_t frequency)
{
	this->pin = pin;
	period = 2000000 / frequency;
	pinMode(pin, INPUT);
}

void ZMCT103::setSensitivity(float value)
{
	this->sensitivity = value;
}

void ZMCT103::setOffSet(int value){
  this->offSet = value;
}

float ZMCT103::getRmsAmp(uint8_t loopCount) {
  float readingAmp = 0.0f;
  for(int i =0; i< loopCount ; i++){
    int32_t maxAnalogValue = 0;
    unsigned long startTime = micros();
    while (micros() - startTime <= period) {
      int32_t analogValue = analogRead(pin);
      if (analogValue > maxAnalogValue) {
        maxAnalogValue = analogValue;
      }
    }
    float currentRms = maxAnalogValue < offSet ? 0 : (maxAnalogValue / ADC_SCALE) * sensitivity;
    readingAmp+=currentRms;
  }
  return readingAmp/loopCount;
}