// This program assumes RTC was previously set to correct time and time format

#include <Arduino.h>
#include <DS3231.h>
#include <Wire.h>

// identifies pin # for interrupt and sets up interrupt signalling byte
// connect SQW pin (RTC) to Pin 2 (Arduino)
#define CLINT 2
volatile byte water = 1;

// Setup clock and common variables
 DS3231 myRTC;

 byte alarmDay;
 byte alarmHour;
 byte alarmMinute;
 byte alarmSecond;
 byte alarmBits;
 bool alarmDayIsDay;
 bool alarmH12;
 bool alarmPM;

// identify output pin to relay
 int relay = 7;

// setup time for millis() and watering run time
 int upTime;
 int startMillis;
 int waterRuntime = 10000; // 10 seconds (1000 * time(in seconds))
 
void water_time() {
// interrupt calls this loop
 water = 1;
}

void setup() {
// begin I2C and Serial
 Wire.begin();
 Serial.begin(9600);

// assign Alarm 1 parameters
// set up to never trigger alarm
 alarmDay = myRTC.getDate();
 alarmHour = myRTC.getHour(alarmH12, alarmPM);
 alarmMinute = myRTC.getMinute(); //sets trigger minutes to 255 (impossible to match)
 alarmSecond = 0xFF; //sets trigger seconds to 255 (impossible to match)
 alarmBits = 0b00001110; // Alarm 1 seconds match 255, i.e. never
 alarmDayIsDay = false; // using date of month

 myRTC.setA1Time(alarmDay, alarmHour, alarmMinute, alarmSecond, alarmBits, alarmDayIsDay, false, false);

 myRTC.turnOffAlarm(1);

// assign Alarm 2 parameters
// triggered to go off EVERYDAY at 0800hr
 alarmHour = 0x08;
 alarmMinute = 0x00;
 alarmBits = 0b00001000;

 myRTC.setA2Time(alarmDay, alarmHour, alarmMinute, alarmBits, alarmDayIsDay, false, false);

 myRTC.turnOffAlarm(2);

// setup clock interrupt
 pinMode(CLINT, INPUT_PULLUP);
 attachInterrupt(digitalPinToInterrupt(CLINT), water_time, FALLING);

// setup relay
 pinMode(relay, OUTPUT);
}

void loop() {
  // do when alarm interrupt is recieved
 if(water) {
  startMillis = millis(); //records current time per millis()
  
  digitalWrite(relay, HIGH);//triggers pump relay
  
  while(upTime < waterRuntime){
    upTime = millis() - startMillis; // records relay uptime per millis()
    delay(500);
  }

  digitalWrite(relay, LOW); // turns off pump relay
  water = 0; // resets water time ISR
  myRTC.turnOffAlarm(2); // resets Alarm 2
 }
}

