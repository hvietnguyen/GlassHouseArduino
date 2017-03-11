/*
 Name:		ButtonAndLCDDemo.ino
 Created:	4/19/2016 1:07:22 PM
 Author:	Nguyen, Hoai Viet
*/
#include <Servo.h>
#include <SoftwareSerial.h>
#include "Time.h"
#include <LiquidCrystal.h>

#define FAN 10
#define WATER 11
#define LINEAR 12
#define BT_FUNCTION 2
#define BT_SETTING 3
//                RS E  D4 D5 D6 D7
LiquidCrystal lcd(4, 5, 6, 7, 8, 9);
//Servo
Servo servo;
// Software serial
SoftwareSerial esp(1, 0); //TX,RX

boolean lastBtnState = LOW;
boolean currentBtnState = LOW;
unsigned long previousTimeDisplay = 0;
byte count = 0;
float temperature = 0.0f;
volatile int hour = 0;
volatile int minute = 0;
volatile byte tempSet = 10;
volatile boolean fanAuto = true;
volatile boolean waterAuto = true;
volatile boolean linearActAuto = true;
volatile byte trigger = 0;
String ssid = "vodafone7F59";
String pass = "8XBEA64FSR";
String server = "glasshouse-hhtml.rhcloud.com";
String line1 = "";
String line2 = "";
Time timer = Time(0,0,0);
String currentTime = "";
enum Mode
{
	Display,
	Temp_Setting,
	Fan_Setting,
	Water_Setting,
	LinearActuator_Setting,
	Time_Setting
};
Mode mode;
enum Display_Mode {
	Temperature,
	Fan,
	Water,
	Linear_Actuator,
	Timer
};
Display_Mode display_mode;
enum Time_Mode {
	Hour,
	Minute,
	Default
};
Time_Mode time_mode;
enum Status
{
	OFF = 0,
	ON = 1,
	FULL = 2
};
Status fanStatus, waterStatus, linearStatus;

// the setup function runs once when you press reset or power the board
void setup() {
	//Serial.begin(115200);
	esp.begin(115200);
	temperature = getTempInCelsius(0); // sensor attach to pin A0
	mode = Display;
	display_mode = Temperature;
	time_mode = Hour;
	fanStatus = OFF;
	waterStatus = OFF;
	linearStatus = OFF;
	trigger = 1;
	//Serial.println(current);
	line1 = "Display Info";
	line2 += temperature;
	line2 += "/";
	line2 += tempSet;
	//Setup comparison register for Timer0 to generate another interrupt
	//Timer0 is 8-bit that counts from 0-255
	OCR0A = 0xFF;  // 0xFF == 255
	// Set n-th bit of mem_add 0x6E
	TIMSK0 |= _BV(OCIE0A); // _BV(bit) (1<<(bit)) where OCIE0A is defined as 1
	// LCD start
	lcd.begin(16, 2);
	display();
	//Motor set pin
	pinMode(FAN, OUTPUT);
	pinMode(WATER, OUTPUT);
	//pinMode(LINEAR, OUTPUT);
	servo.attach(LINEAR);
	//buttons set pins
	pinMode(BT_FUNCTION, INPUT);
	pinMode(BT_SETTING, INPUT);
	// attach interrupts
	attachInterrupt(digitalPinToInterrupt(BT_FUNCTION), listenBtnFunction, CHANGE);
	attachInterrupt(digitalPinToInterrupt(BT_SETTING), listenBtnSetting, CHANGE); 
	connectWiFi();
}

void connectWiFi() {
	esp.println("AT+CWMODE=3"); // configure esp module as AP+station
	delay(500);
	esp.println("AT+CWQAP"); // Quit previous wifi connection
	//esp.println("AT+RST"); // reset
	delay(500);
	String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + pass + "\"";
	esp.println(cmd); // connect wifi
	delay(7000);
	esp.println("AT+CIFSR"); // Get IP address
	delay(500);
	//esp.println("At+CIPMUX=1");
	//delay(1000);
	//esp.println("AT+CIPSERVER=1,80"); // configure esp module as server, port 80
	//delay(1000);
}

void sendData() { 
	esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80");
	delay(1000);
	// build data
	String data = String(trigger)+"||"+String(temperature) + "||" + String(tempSet);
	data += "||" + String(fanAuto) + "||" + String(fanStatus);
	data += "||" + String(waterAuto) + "||" + String(waterStatus);
	data += "||" + String(linearActAuto) + "||" + String(linearStatus);
	String getRequest = "GET /test?param=" + data + " HTTP/1.1\r\nHost:glasshouse-hhtml.rhcloud.com\r\n\r\n";
	//String postRequest = "POST /testHTTP/1.1\r\nHost: glasshouse-hhtml.rhcloud.com\r\nAccept:*/*\r\n";
	//postRequest += "Content-Length: " + String(data.length()) + "\r\n";
	//postRequest += "Content-Type: application/x-www-form-urlencoded\r\n\r\n" + data;
	String cmd = "AT+CIPSEND=";
	cmd += String(getRequest.length());
	esp.println(cmd);
	delay(1000);
	if (esp.find(">")) {
		//esp.println(getRequest);
		delay(1000);
	}
	esp.println(getRequest);
	delay(1000);
	while (esp.available()) {
		if (esp.find("SEND OK")) {
			trigger = 0;
		}
	}
	esp.println("AT+CIPCLOSE");
	//trigger = 0;
}

/* 
	Define an interrupt handler for timer interrupt vector TIMER0_COMPA_vect
	Interrupt is called once a millisecond 
*/
SIGNAL(TIMER0_COMPA_vect) {
	unsigned long currentMillis = millis();
	if ((currentMillis - previousTimeDisplay) > 1000) {
		++count;
		previousTimeDisplay = millis();
		currentTime = timer.nextSecond();
		if (mode == Display && display_mode == Timer) {
			line2 = currentTime;
		}
		display();
	}
	
	if (count >= 4/*(currentMillis - previousTimeDisplay) > 4000*/) {
		count = 0;
		previousTimeDisplay = millis();
		temperature = getTempInCelsius(0);
		if (mode == Display && display_mode == Temperature) {
			line2 = "";
			line2 += temperature;
			line2 += "/";
			line2 += tempSet;
		}
		display();
	}
	fanOperate();
	waterOperate();
	linearOperate();
}

void fanOperate() {
	// Fan operation
	/*if (!fanAuto) {
		digitalWrite(FAN, HIGH);
		fanStatus = ON;
	}
	else if (fanAuto && (temperature - tempSet) > 5) {
		digitalWrite(FAN, HIGH);
		fanStatus = ON;
	}
	else {
		digitalWrite(FAN, LOW);
		fanStatus = OFF;
	}*/
	if (fanAuto) {
		if ((temperature - tempSet) > 5) {
			digitalWrite(FAN, HIGH);
			fanStatus = ON;
		}
		else {
			digitalWrite(FAN, LOW);
			fanStatus = OFF;
		}
	}
	else {
		if (fanStatus == ON) {
			digitalWrite(FAN, HIGH);
		}
		else {
			digitalWrite(FAN, LOW);
		}
	}
}

void waterOperate() {
	/*if (!waterAuto) {
		digitalWrite(WATER, HIGH);
		waterStatus = ON;
	}
	else if(timer.getHour()==6 || timer.getHour()==12)
	{
		digitalWrite(WATER, HIGH);
		waterStatus = ON;
	}
	else {
		digitalWrite(WATER, LOW);
		waterStatus = OFF;
	}*/
	if (waterAuto) {
		if (timer.getHour() == 6 || timer.getHour() == 12) {
			digitalWrite(WATER, HIGH);
			waterStatus = ON;
		}
		else {
			digitalWrite(WATER, LOW);
			waterStatus = OFF;
		}
	}
	else {
		if (waterStatus == ON) {
			digitalWrite(WATER, HIGH);
		}
		else {
			digitalWrite(WATER, LOW);
		}
	}
}

void linearOperate() {
	/*if (!linearActAuto && linearStatus==ON) {
		servo.write(90);
	}
	else if (!linearActAuto && linearStatus == FULL) {
		servo.write(180);
	}
	else if (linearActAuto && (temperature - tempSet) > 5) {
		servo.write(180);
		linearStatus = FULL;
	}
	else {
		servo.write(0);
		linearStatus = OFF;
	}*/
	if (linearActAuto) {
		if ((temperature - tempSet) > 5) {
			servo.write(90);
			linearStatus = ON;
		}
		else {
			servo.write(0);
			linearStatus = OFF;
		}
	}
	else {
		if(linearStatus==ON)servo.write(90);
		else if(linearStatus==FULL)servo.write(180);
		else servo.write(0);
	}
}

// the loop function runs over and over again until power down or reset
void loop() {
	//if(!esp.available())
	sendData();
	delay(5000);
}

void display() {
	// LCD
	lcd.clear();
	lcd.setCursor(0, 0);
	lcd.print(line1);
	lcd.setCursor(0, 1);
	lcd.print(line2);
}

/* Detect tempture from LM35 sensor IC*/
float getTempInCelsius(int p) {
	float miliVolt, cel;
	int value = analogRead(p);
	miliVolt = value / 1024.0 * 5000;
	return cel = miliVolt / 10;
}

/*
	Debounce button - return current state of button.
*/
boolean deBounce(byte btnPin, boolean last) {
	boolean current = digitalRead(btnPin);
	if (current != last) {
		delay(5);
		current = digitalRead(btnPin);
	}
	return current;
}
/*
Button Function
Interrupt Service Routine (ISR)
*/
void listenBtnFunction() {
	currentBtnState = deBounce(BT_FUNCTION, lastBtnState);
	if (lastBtnState == LOW && currentBtnState == HIGH) {
		switch (mode)
		{
		case Display:
			mode = Temp_Setting;
			line1 = "Temp Setting";
			line2 = "";
			line2 += tempSet;
			break;
		case Temp_Setting:
			mode = Fan_Setting;
			line1 = "Fan Setting";
			//line2 = (fanAuto) ? "Auto" : "Active";
			if (fanAuto) line2 = "Auto";
			else line2= (fanStatus == ON) ? "ON":"OFF";
			break;
		case Fan_Setting:
			mode = Water_Setting;
			line1 = "Water Setting";
			//line2 = (waterAuto) ? "Auto" : "Active";
			if (waterAuto) line2 = "Auto";
			else line2= (waterStatus == ON) ? "ON":"OFF";
			break;
		case Water_Setting:
			mode = LinearActuator_Setting;
			line1 = "Linear Setting";
			//line2 = (linearActAuto) ? "Auto" : "Active";
			if (linearActAuto) line2 = "Auto";
			else {
				if (linearStatus == ON)line2 = "OPEN HALF";
				else if (linearStatus == FULL)line2 = "OPEN FULL";
				else line2 = "OFF";
			}
			break;
		case LinearActuator_Setting:
			mode = Time_Setting;
			line1 = "Time Setting";
			line2 = currentTime;
			break;
		case Time_Setting:
			if (time_mode == Hour)
				time_mode = Minute;
			else if(time_mode == Minute)
			{
				time_mode = Hour;
				display_mode = Temperature;
				mode = Display;
				line1 = "Display Info";
				line2 = "";
				line2 += temperature;
				line2 += "/";
				line2 += tempSet;
			}
			break;
		default:
			break;
		}
	}
	lastBtnState = currentBtnState;
	display();
}

/*
Button Setting
Interrupt Service Routine (ISR)
*/
void listenBtnSetting() {
	currentBtnState = deBounce(BT_SETTING, lastBtnState);
	if (lastBtnState == LOW && currentBtnState == HIGH) {
		if (mode == Display) {
			switch (display_mode)
			{
			case Temperature:
				display_mode = Fan;
				line2 = "Fan:";
				line2 += (fanAuto) ? "Auto/" : "Active/";
				line2 += (fanStatus == ON) ? "ON" : "OFF";
				break;
			case Fan:
				display_mode = Water;
				line2 = "Water:";
				line2 += (waterAuto) ? "Auto/" : "Active/";
				line2 += (waterStatus == ON) ? "ON" : "OFF";
				break;
			case Water:
				display_mode = Linear_Actuator;
				line2 = "Roof:";
				line2 += (linearActAuto) ? "Auto/" : "Active/";
				//line2 += (linearStatus == ON) ? "ON" : "OFF";
				if (linearStatus == ON) line2 += "HALF";
				else if (linearStatus == FULL)line2 += "FULL";
				else line2 += "OFF";
				break;
			case Linear_Actuator:
				display_mode = Timer;
				line2 = currentTime;
				break;
			case Timer:
				display_mode = Temperature;
				line2 = "";
				line2 += temperature;
				line2 += "/";
				line2 += tempSet;
				break;
			default:
				break;
			}
		}
		else if(mode == Temp_Setting) {
			
			tempSet = (++tempSet>26) ? 10 : tempSet;
			line2 = String(tempSet);
			trigger = 1;
		}
		else if (mode == Fan_Setting) {
			//fanAuto = !fanAuto;
			//line2 = (fanAuto) ? "Auto" : "Active";
			if (fanAuto) {
				fanAuto = false;
				fanStatus = ON;
			}
			else {
				if (fanStatus == ON)fanStatus = OFF;
				else fanAuto = true;
			}

			if (fanAuto) line2 = "Auto";
			else {
				if (fanStatus == ON) line2 = "ON";
				else line2 = "OFF";
			}
			trigger = 1;
		}
		else if (mode == Water_Setting) {
			//waterAuto = !waterAuto;
			//line2 = (waterAuto) ? "Auto" : "Active";
			if (waterAuto) {
				waterAuto = false;
				waterStatus = ON;
			}
			else {
				if (waterStatus == ON)waterStatus = OFF;
				else waterAuto = true;
			}

			if (waterAuto) line2 = "Auto";
			else {
				if (waterStatus == ON) line2 = "ON";
				else line2 = "OFF";
			}
			trigger = 1;
		}
		else if (mode == LinearActuator_Setting) {
			//linearActAuto = !linearActAuto;
			//line2 = (linearActAuto) ? "Auto" : "Active";
			if (linearActAuto) {
				linearActAuto = false;
				linearStatus = ON;
			}
			else {
				if (linearStatus == ON)linearStatus = FULL;
				else if (linearStatus == FULL)linearStatus = OFF;
				else linearActAuto = true;
			}
			if (linearActAuto) line2 = "Auto";
			else {
				if (linearStatus == ON)line2 = "OPEN HALF";
				else if (linearStatus == FULL)line2 = "OPEN FULL";
				else line2 = "OFF";
			}
			trigger = 1;
		}
		else if (mode == Time_Setting) {
			timer.start();
			if (time_mode == Hour) {
				hour = timer.getHour();
				hour = (++hour >= 24) ? 0: hour;
			}
			else if(time_mode == Minute) 
			{
				minute = timer.getMinute();
				minute = (++minute >= 60) ? 0 : minute;
			}
			timer.setHour(hour);
			timer.setMinute(minute);
			currentTime = timer.getCurrent();
			Serial.println(currentTime);
			line2 = "";
			line2 += currentTime;
		}
	}
	lastBtnState = currentBtnState;
	display();
}


