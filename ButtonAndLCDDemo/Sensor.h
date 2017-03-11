// Sensor.h

#ifndef _SENSOR_h
#define _SENSOR_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
class Sensor {
private:
	int pin, adc;
public:
	//Constructor
	Sensor(int p = 0);
	//---------------------------------
	int getPin() const;
	void setPin(int p);
	void converseAD();
	int getADC() const;
	float getMilliVolts() const;
	float getCelsius() const;
	float getFahrenheit() const;
};


#endif

