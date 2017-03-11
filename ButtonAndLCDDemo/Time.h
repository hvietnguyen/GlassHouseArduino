// Time.h

#ifndef _TIME_h
#define _TIME_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
class Time {

private:
	int h, m, s;
	boolean isStart;
public:
	Time(int h = 0, int m = 0, int s = 0);
	boolean IsStart()const;
	void setStart(bool flag);
	int getHour()const;
	void setHour(int h);
	int getMinute()const;
	void setMinute(int m);
	int getSecond()const;
	void setSecond(int s);
	String getCurrent()const;
	void setTime(int h, int m, int s);
	String nextSecond();
	void start();
};

#endif

