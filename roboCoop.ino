//v2.0

#include <Time.h>
#include <TimeLib.h>
#include "roboCoop.h"
#include <LiquidCrystal.h>

//inputs (switches)
int switch_p = A3; //D2
int doorStop_p = A4; //D3
int setButton_p = 2;
int selectButton_p = 3;

//outputs (to actuators)
int solenoidEn_p = A2;
int motorDirection_p = 5; //0 -> counter-clockwise, 1 -> clockwise
int motorEn_p = 4;

//LCD pins
int d4_p = 11;
int d5_p = 6;
int d6_p = 12;
int d7_p = 7;
int rs_p = 8;
int en_p = 10;
int rw_p = 9;

LiquidCrystal lcd(rs_p, en_p, d4_p, d5_p, d6_p, d7_p); 
String currScreen;

//state variables
int doorState; //0 -> closed, 1 -> open
int switchState; //0 -> switch off, 1 -> switch on
bool flipFlag; //To determine if the switch logic need to be reversed
int prevSwitchState; //To detect switch has been flipped
int currSwitchState;

TimeElements time;
TimeElements openAlarm;
TimeElements closeAlarm;
time_t timeOut;

machine_state_t machineState;
set_time_state_t setState;


void setup()
{
  pinMode(13,OUTPUT);
  
  pinMode(motorDirection_p,OUTPUT);
  pinMode(motorEn_p, OUTPUT);
  pinMode(solenoidEn_p,OUTPUT);
  pinMode(rw_p,OUTPUT);
  
  pinMode(switch_p, INPUT);
  pinMode(setButton_p,INPUT);
  pinMode(selectButton_p,INPUT);

  digitalWrite(motorEn_p, LOW);
  digitalWrite(motorDirection_p,LOW);
  digitalWrite(solenoidEn_p,LOW);
  digitalWrite(rw_p,LOW);
  
  //DEFAULT time and alarms
  time.Second = 0;
  time.Minute = 0;
  time.Hour = 12;
  time.Day = 1;
  time.Month = 1;
  time.Year = 46;
  setTime(makeTime(time));
  
  openAlarm.Hour = 8;
  openAlarm.Minute = 0;
  closeAlarm.Hour = 18;
  closeAlarm.Minute = 30;

  flipFlag = digitalRead(switch_p) == digitalRead(doorStop_p);
  prevSwitchState = digitalRead(switch_p);
  machineState = READY;
}

void mockDoor(bool openDoor)
{
  if (openDoor)
  for(int i = 0; i < 5; i++)
  {
     digitalWrite(13,i%2);
     delay(500);
  }
  else
  for(int i = 0; i < 50; i++)
  {
     digitalWrite(13,i%2);
     delay(100);
  }
}

void loop()
{
  if(false){
    while(digitalRead(switch_p))
    {
      digitalWrite(13,HIGH);
      digitalWrite(solenoidEn_p,HIGH);
    }
    digitalWrite(13,LOW);
      digitalWrite(solenoidEn_p,LOW);
  }
  else{
  switch(machineState)
  {
    case READY:
      doorState = !digitalRead(doorStop_p); //LOW->closed, HIGH->open
      currSwitchState = digitalRead(switch_p);
      switchState =  currSwitchState ^ flipFlag; //HIGH->on, LOW->off
      breakTime(now(),time);
      lcdShowTime("Time: ",time, "");
      if (currSwitchState != prevSwitchState &&
          switchState != doorState) //door must be opened or closed
      {
        machineState = (switchState==HIGH) ? OPENING : CLOSING;
      }
      prevSwitchState = currSwitchState;
      if(doorState==LOW && checkAlarm(openAlarm))
      {
        machineState = OPENING;
        flipFlag = !flipFlag;
      }
      else if (doorState==HIGH && checkAlarm(closeAlarm))
      {
        machineState = CLOSING;
        flipFlag = !flipFlag;
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
            if (openAlarm.Hour == 23)
              openAlarm.Hour = 0;
            else
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
            if (openAlarm.Minute == 59)
              openAlarm.Minute = 0;
            else
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
            if (closeAlarm.Hour == 23)
              closeAlarm.Hour = 0;
            else
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
            if (closeAlarm.Minute == 59)
              closeAlarm.Minute = 0;
            else
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
          lcdShowTime("Time: ",time,"SETTING HOUR");
          if (digitalRead(selectButton_p)==HIGH)
          {
            if (time.Hour == 23)
              time.Hour = 0;
            else
              time.Hour += 1;
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
            if (time.Minute == 59)
              time.Minute = 0;
            else
              time.Minute += 1;
            delay(250);
          }
          if (digitalRead(setButton_p))
          {
            setTime(makeTime(time));
            currSwitchState = digitalRead(switch_p);
            lcdPrint("MANUAL MODE");
            machineState = MANUAL;
            delay(500); 
          }
          break;
      }
      break;
    case OPENING: //activate door
      operateDoor(HIGH);
      machineState = READY;
      break;
    case CLOSING:
      operateDoor(LOW);
      machineState = READY;
      break;
    case MANUAL: //manual operation of door
      while(currSwitchState != digitalRead(switch_p))
      {
        digitalWrite(motorDirection_p,HIGH);
        digitalWrite(motorEn_p,HIGH);
        digitalWrite(solenoidEn_p,HIGH);
      }
      digitalWrite(solenoidEn_p,LOW);
      digitalWrite(motorEn_p,LOW);
      while(digitalRead(selectButton_p))
      {
        digitalWrite(motorDirection_p,LOW);
        digitalWrite(motorEn_p,HIGH);
        digitalWrite(solenoidEn_p,HIGH);
      }
      digitalWrite(solenoidEn_p,LOW);
      digitalWrite(motorEn_p,LOW);
      if (digitalRead(setButton_p))
      {
        flipFlag = (digitalRead(switch_p) == digitalRead(doorStop_p)) ? true : false;
        machineState = READY;
        delay(500); 
      }
      break;
  }
  }
}

void lcdPrint(String message)
{
  lcd.clear();
    lcd.begin(16, 2);
    lcd.print(message);
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

void operateDoor(bool openDoor) 
{
  //mockDoor(openDoor);
  //return;
  digitalWrite(13,HIGH);
  digitalWrite(motorDirection_p, !openDoor);
  if (openDoor) //opening door
  {
    digitalWrite(solenoidEn_p, HIGH);
    delay(250); //TODO: calibrate
    digitalWrite(motorEn_p,HIGH);
    delay(1000); //TODO: calibrate
    digitalWrite(solenoidEn_p, LOW);
    delay (1600); // TODO: calibrate
    digitalWrite(motorEn_p,LOW);
  }
  else //closing door
  {
    time_t start = now();
    digitalWrite(motorEn_p, HIGH);
    digitalWrite(solenoidEn_p,HIGH);
    while (!digitalRead(doorStop_p) || now() - start > 2);
    digitalWrite(motorEn_p, LOW);
    digitalWrite(solenoidEn_p,LOW);
  }
  digitalWrite(13,LOW);
}



bool checkAlarm(TimeElements alarm)
{
  return (alarm.Hour == hour() && alarm.Minute == minute()) ? true : false; 
}


