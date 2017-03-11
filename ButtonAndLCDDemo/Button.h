// Button.h

#ifndef _BUTTON_h
#define _BUTTON_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
class Button {
private:
	boolean previousState = HIGH;
	boolean currentState = HIGH;
	byte pin;
public:
	//Constructor
	Button(byte p=0);
	boolean de
};

#endif

