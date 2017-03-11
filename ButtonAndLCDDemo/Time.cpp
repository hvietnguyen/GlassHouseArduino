// 
// 
// 

#include "Time.h"
//Implement the Timer class
Time::Time(int h, int m, int s) {
	this->h = h;
	this->m = m;
	this->s = s;
	isStart = false;
}
boolean Time::IsStart()const{
	return this->isStart;
}
void Time::setStart(boolean flag) {
	this->isStart = flag;
}
int Time::getHour()const {
	return this->h;
}
void Time::setHour(int h) {
	this->h = h;
}
int Time::getMinute()const {
	return this->m;
}
void Time::setMinute(int m) {
	this->m = m;
}
int Time::getSecond()const {
	return this->s;
}
void Time::setSecond(int s) {
	this->s = s;
}
String Time::getCurrent()const {
	String current = "";
	current += getHour();
	current += ":";
	current += getMinute();
	current += ":";
	current += getSecond();
	return current;
}
void Time::setTime(int h, int m, int s) {
	this->h = h;
	this->m = m;
	this->s = s;
}
String Time::nextSecond() {
	if (IsStart()) {
		String current = "";
		++s;
		if (s >= 60) {
			s = 0;
			++m;
		}
		if (m >= 60) {
			m = 0;
			++h;
		}
		if (h >= 24) {
			h = 0;
		}
	}
	return getCurrent();
}
void Time::start() {
	this->setStart(true);
}
