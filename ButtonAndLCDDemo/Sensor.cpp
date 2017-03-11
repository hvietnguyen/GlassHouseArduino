// 
// 
// 

#include "Sensor.h"
// Constructor with initialiser list
Sensor::Sensor(int p) :pin(p) {
	adc = 0;
}

int Sensor::getPin() const {
	return pin;
}

void Sensor::setPin(int p) {
	pin = p;
}

void Sensor::converseAD() {
	adc = analogRead(pin);
}

int Sensor::getADC() const {
	return adc;
}

float Sensor::getMilliVolts() const {
	//return (adc / 1024.0) * 3300.0;
	return adc / 205.0;
}

float Sensor::getCelsius() const {
	return getMilliVolts() * 100 - 50;
}

float Sensor::getFahrenheit()const {
	return getCelsius() * 9.0 / 5.0 + 32.0;
}

