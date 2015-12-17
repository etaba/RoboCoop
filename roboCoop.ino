#include <Time.h>
#include <TimeLib.h>
#include "roboCoop.h"
#include <LiquidCrystal.h>

//inputs
int switch_p = 2; //D2
int doorStop_p = 3; //D3
int setButton_p = 7; //D7
int selectButton_p = A0; //A0

//outputs
int solenoidEn_p = 4;
int motorDirection_p = 5; //0 -> counter-clockwise, 1 -> clockwise
int motorEn_p = 6;

//LCD
int d4_p = 8;
int d5_p = 9;
int d6_p = 10;
int d7_p = 11;
int rs_p = 12;
int en_p = 13;
LiquidCrystal lcd(rs_p, en_p, d4_p, d5_p, d6_p, d7_p); 
String currScreen;

//state variables
int doorState; //0 -> closed, 1 -> open
int switchState; //0 -> switch off, 1 -> switch on

TimeElements time;
TimeElements openAlarm;
TimeElements closeAlarm;
time_t timeOut;

machine_state_t machineState;
set_time_state_t setState;


void setup()
{
  pinMode(motorDirection_p,OUTPUT);
  pinMode(motorEn_p, OUTPUT);
  pinMode(switch_p, INPUT);
  pinMode(solenoidEn_p,OUTPUT);
  pinMode(setButton_p,INPUT);
  pinMode(selectButton_p,INPUT);
  digitalWrite(motorEn_p, LOW);
  digitalWrite(motorDirection_p,LOW);
  digitalWrite(solenoidEn_p,LOW);
  
  doorState = LOW;
  machineState = READY;
  
  //DEFAULT time and alarms?
  time.Second = 0;
  time.Minute = 34;
  time.Hour = 5;
  time.Day = 1;
  time.Month = 1;
  time.Year = 46;
  setTime(makeTime(time));
  openAlarm.Hour = 8;
  openAlarm.Minute = 0;
  closeAlarm.Hour = 18;
  closeAlarm.Minute = 30;
}


void loop()
{
  switch(machineState)
  {
    case READY:
      switchState = digitalRead(switch_p);
      breakTime(now(),time);
      lcdShowTime("Time: ",time, "");
      if (switchState != doorState)
      {
        operateDoor(switchState);
        doorState = switchState;
        machineState = (switchState==1) ? OPEN : CLOSE;
      }
      if(doorState==0 && checkAlarm(openAlarm))
      {
        machineState = OPEN;
      }
      if (doorState==1 && checkAlarm(closeAlarm))
      {
        machineState = CLOSE;
      }
      if (digitalRead(setButton_p) == HIGH)
      {
         machineState = SET;
         setState = OPEN_HOUR;
         timeOut = now();
         delay(500);
      }
      break;
    case SET:
      if (now() - timeOut > 60)
      {
        machineState = READY;
      }
      switch(setState)
      {
        case OPEN_HOUR:
          lcdShowTime("Open at: ",openAlarm, "SETTING HOUR");
          if (digitalRead(selectButton_p)==HIGH)
          {
            openAlarm.Hour += 1;
            delay(250);
          }
          if (digitalRead(setButton_p))
          {
            setState = OPEN_MINUTE;
            timeOut = now();
            delay(500);
          }
          break;
          
        case OPEN_MINUTE: 
          lcdShowTime("Open at: ",openAlarm,"SETTING MINUTE");
          if (digitalRead(selectButton_p)==HIGH)
          {
            openAlarm.Minute += 1;
            delay(250);
          }
          if (digitalRead(setButton_p))
          {
            setState = CLOSE_HOUR;
            timeOut = now();
            delay(500); 
          }
          break;
          
        case CLOSE_HOUR:
          lcdShowTime("Close at: ",closeAlarm,"SETTING HOUR");
          if (digitalRead(selectButton_p)==HIGH)
          {
            closeAlarm.Hour += 1;
            delay(250);
          }
          if (digitalRead(setButton_p))
          {
            setState = CLOSE_MINUTE;
            timeOut = now();
            delay(500); 
          }
          break;
          
        case CLOSE_MINUTE: 
          lcdShowTime("Close at: ",closeAlarm,"SETTING MINUTE");
          if (digitalRead(selectButton_p)==HIGH)
          {
            closeAlarm.Minute += 1;
            delay(250);
          }
          if (digitalRead(setButton_p))
          {
            setState = TIME_HOUR;
            timeOut = now();
            delay(500); 
          }
          break;

        case TIME_HOUR:
          lcdShowTime("Time: ",closeAlarm,"SETTING HOUR");
          if (digitalRead(selectButton_p)==HIGH)
          {
            time.Minute += 1;
            delay(250);
          }
          if (digitalRead(setButton_p))
          {
            setState = TIME_MINUTE;
            timeOut = now();
            delay(500); 
          }
          break;

        case TIME_MINUTE:
          lcdShowTime("Time: ",time,"SETTING MINUTE");
          if (digitalRead(selectButton_p)==HIGH)
          {
            closeAlarm.Minute += 1;
            delay(250);
          }
          if (digitalRead(setButton_p))
          {
            setTime(makeTime(time));
            machineState = READY;
            delay(500); 
          }
          break;
      }
      break;
    case OPEN: //activate door
      operateDoor(1);
      doorState = 1;
      machineState = READY;
      break;
    case CLOSE:
      operateDoor(0);
      doorState = 0;
      machineState = READY;
      break;
  }
}

void lcdShowTime(String prefix, TimeElements t, String details)
{
  String formattedTime = formatTime(t.Hour,t.Minute); 
  // Print a message to the LCD.
  if (currScreen != formattedTime + details)
  {
    lcd.clear();
    lcd.begin(16, 2);
    lcd.print(prefix + formattedTime);
    currScreen = formattedTime + details;
    if (details != "")
    {
      lcd.setCursor(1,2);
      lcd.print(details);
    }
  }
}

void lcdShowAlarm(alarm_s alarm)
{
  lcd.begin(16, 2);
  lcd.print("Open at: " + String(alarm.hour) + ":" + String(alarm.minute));
}

String formatTime(int hour, int minute)
{
  String dispHour;
  String dispMinute;
  dispHour = hour < 10 ? "0" + String(hour) : String(hour);
  dispMinute = minute < 10 ? "0" + String(minute) : String(minute);
  return dispHour + ":" + dispMinute;
}

void operateDoor(int state) //assuming door open and close takes same amount of time
{
  digitalWrite(motorDirection_p, !state);
  if (state) //opening door
  {
    digitalWrite(solenoidEn_p, HIGH);
    delay(500); //needs calibration
    //Serial.print("activating motor...");
    digitalWrite(motorEn_p,HIGH);
    delay (4500); //needs calibration
    digitalWrite(solenoidEn_p, LOW);
    //delay (2500); //needs calibration
    digitalWrite(motorEn_p,LOW);
  }
  else //closing door
  {
    digitalWrite(motorEn_p, HIGH);
    digitalWrite(solenoidEn_p,HIGH);
    while (!digitalRead(doorStop_p));
    digitalWrite(motorEn_p, LOW);
    delay(300);
    digitalWrite(solenoidEn_p,LOW);
  }
}



bool checkAlarm(TimeElements alarm)
{
 return (alarm.Hour == hour() && alarm.Minute == minute()) ? true : false; 
}

void pulse13()
{
  digitalWrite(13,HIGH);
  delay(1000);
  digitalWrite(13,LOW);
}
