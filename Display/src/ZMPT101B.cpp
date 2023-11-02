

#include "ZMPT101B.h"

/// @brief ZMPT101B constructor
/// @param pin analog pin that ZMPT101B connected to.
/// @param frequency AC system frequency
ZMPT101B::ZMPT101B(uint8_t pin, uint16_t frequency)
{
	this->pin = pin;
	period = 1000000 / frequency;
	pinMode(pin, INPUT);
}

/// @brief Set sensitivity
/// @param value Sensitivity value
void ZMPT101B::setSensitivity(float value)
{
	sensitivity = value;
}

/// @brief Calculate zero point
/// @return zero / center value
int ZMPT101B::getZeroPoint()
{
	uint32_t Vsum = 0;
	uint32_t measurements_count = 0;
	uint32_t t_start = micros();

	while (micros() - t_start < period)
	{
		Vsum += analogRead(pin);
		measurements_count++;
	}

	return Vsum / measurements_count;
}

/// @brief Calculate root mean square (RMS) of AC valtage
/// @param loopCount Loop count to calculate
/// @return root mean square (RMS) of AC valtage
float ZMPT101B::getRmsVoltage(uint8_t loopCount)
{
	double readingVoltage = 0.0f;

	for (uint8_t i = 0; i < loopCount; i++)
	{
		int zeroPoint = this->getZeroPoint();

    	int32_t maxV= 0;
		int32_t vNow = 0;
		uint32_t vSum = 0;
		uint32_t measurements_count = 0;
		uint32_t t_start = micros();

		while (micros() - t_start < period)
		{
			vNow = analogRead(pin) - zeroPoint;
      		vSum += sq(vNow);
			measurements_count++;
      		maxV = vNow > maxV ? vNow : maxV;
		}

		readingVoltage += (maxV <= offSet ? 0: sqrt(vSum / measurements_count) / ADC_SCALE * VREF * sensitivity);
	}

	return readingVoltage / loopCount;
}