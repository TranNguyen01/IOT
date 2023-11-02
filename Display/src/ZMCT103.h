// #ifndef ZMCT103_H
// #define ZMCT103_H

// float MeasureCurrent(uint8_t pin, int offset = 0, float ref = 5, float calibrate = 1);

// #endif


#ifndef ZMCT103_H    
#define ZMCT103_H

#include <Arduino.h>

#define DEFAULT_FREQUENCY 50.0f
#define DEFAULT_SENSITIVITY 500.0f
#define DEFAULT_OFFSET 4;


#if defined(AVR)
	#define ADC_SCALE 1023.0f
	#define VREF 5.0f
#elif defined(ESP8266)
	#define ADC_SCALE 1023.0
	#define VREF 3.3
#elif defined(ESP32)
	#define ADC_SCALE 4095.0
	#define VREF 3.3
#endif

class ZMCT103
{
    public:
        ZMCT103 (uint8_t pin, uint16_t frequency = DEFAULT_FREQUENCY);
        void     setSensitivity(float value);
        void     setOffSet(int value);
        float 	 getRmsAmp(uint8_t loopCount = 1);

    private:
        uint8_t  pin;
        uint32_t period;
        float 	 sensitivity = DEFAULT_SENSITIVITY;
        int offSet = DEFAULT_OFFSET;
};
#endif