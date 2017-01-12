//v2.0

#include <Time.h>
#include <TimeLib.h>
#include "roboCoop.h"
#include <LiquidCrystal.h>
#include "Wire.h"
#define DS3231_I2C_ADDRESS 0x68


//inputs (switches)
int switch_p = A3; //D2
int doorStop_p = A7; //D3
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
bool alarmState; //True -> alarm on
bool flipFlag; //To determine if the switch logic need to be reversed
int prevSwitchState; //To detect switch has been flipped
int currSwitchState;

TimeElements openAlarm;
TimeElements closeAlarm;
time_t timeOut;

machine_state_t machineState;
set_time_state_t setState;


void setup()
{
  Wire.begin();
  Serial.begin(9600);

  pinMode(13,OUTPUT);
  
  pinMode(motorDirection_p,OUTPUT);
  pinMode(motorEn_p, OUTPUT);
  pinMode(solenoidEn_p,OUTPUT);
  pinMode(rw_p,OUTPUT);
  
  pinMode(switch_p, INPUT);
  pinMode(setButton_p,INPUT);
  pinMode(selectButton_p,INPUT);
  pinMode(doorStop_p,INPUT);

  digitalWrite(motorEn_p, LOW);
  digitalWrite(motorDirection_p,LOW);
  digitalWrite(solenoidEn_p,LOW);
  digitalWrite(rw_p,LOW);
  
  //DEFAULT time and alarms   
  setCurrTime(12,0);
  openAlarm.Hour = 6;
  openAlarm.Minute = 30;
  closeAlarm.Hour = 20;
  closeAlarm.Minute = 0;

  alarmState = true;
  flipFlag = digitalRead(switch_p) == digitalRead2(doorStop_p);
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
  //for testing:
  if(false){
    while(digitalRead2(doorStop_p))
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
      doorState = !digitalRead2(doorStop_p); //LOW->closed, HIGH->open
      currSwitchState = digitalRead(switch_p);
      switchState =  currSwitchState ^ flipFlag; //HIGH->on, LOW->off
      lcdShowTime("Time: ", getCurrTime(), (alarmState ? "Alarm On" : "Alarm Off"));
      if (currSwitchState != prevSwitchState &&
          switchState != doorState) //door must be opened or closed
      {
        machineState = (switchState==HIGH) ? OPENING : CLOSING;
      }
      prevSwitchState = currSwitchState;
      if(alarmState && doorState==LOW && checkAlarm(openAlarm))
      {
        machineState = OPENING;
        flipFlag = !flipFlag;
      }
      else if (alarmState && doorState==HIGH && checkAlarm(closeAlarm))
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
      if (digitalRead(selectButton_p) == HIGH) //toggle alarm
      {
        alarmState = !alarmState;
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
          {
            TimeElements currTime = getCurrTime();
            lcdShowTime("Time: ",currTime,"SETTING HOUR");
            if (digitalRead(selectButton_p)==HIGH)
            {
              if (currTime.Hour == 23)
                currTime.Hour = 0;
              else
                currTime.Hour += 1;
              setCurrTime(currTime.Hour, currTime.Minute);
              delay(250);
            }
            if (digitalRead(setButton_p))
            {
              setState = TIME_MINUTE;
              timeOut = now();
              delay(500); 
            }
          }
          break;

        case TIME_MINUTE:
        {
          TimeElements t = getCurrTime();
          lcdShowTime("Time: ",t,"SETTING MINUTE");
          if (digitalRead(selectButton_p)==HIGH)
          {
            if (t.Minute == 59)
              t.Minute = 0;
            else
              t.Minute += 1;
            setCurrTime(t.Hour, t.Minute);
            delay(250);
          }
          if (digitalRead(setButton_p))
          {
            currSwitchState = digitalRead(switch_p);
            machineState = READY;
            delay(500); 
          }
        }
        break;
      }
      break;
    case OPENING: //activate door
      if (!operateDoor(HIGH))
      {
        machineState = ERROR_STATE;
      }
      else
      {
        machineState = READY;
      }
      break;
    case CLOSING:
      if (!operateDoor(LOW))
      {
        machineState = ERROR_STATE;
      }
      else
      {
        machineState = READY;
      }
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
        flipFlag = digitalRead(switch_p) == digitalRead2(doorStop_p);
        machineState = READY;
        delay(500); 
      }
      break;
    case ERROR_STATE: //error, to prompt user to reset system
      lcdPrint("     ERROR!","RESET MANUALLY");
      delay (10000);
      break;
  }
  }
}

void lcdPrint(String header, String sub)
{
  lcd.clear();
  lcd.begin(16, 2);
  lcd.print(header);
  lcd.setCursor(1,2);
  lcd.print(sub);
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

String formatTime(int t_hour, int t_minute)
{
  String dispHour;
  String dispMinute;
  dispHour = t_hour < 10 ? "0" + String(t_hour) : String(t_hour);
  dispMinute = t_minute < 10 ? "0" + String(t_minute) : String(t_minute);
  return dispHour + ":" + dispMinute;
}

bool operateDoor(bool openDoor) 
{
  //mockDoor(openDoor);
  //return;
  //digitalWrite(13,HIGH);
  time_t start = now();
  digitalWrite(motorDirection_p, !openDoor);
  if (openDoor && digitalRead2(doorStop_p)) //opening door
  {
    digitalWrite(solenoidEn_p, HIGH);
    delay(250); //TODO: calibrate
    digitalWrite(motorEn_p,HIGH);
    delay(1000);
    if (digitalRead2(doorStop_p)) //error if microswitch still closed after opening door for 500ms
    {
      digitalWrite(motorEn_p, LOW);
      digitalWrite(solenoidEn_p,LOW);
      return false;
    }
    delay (1750 );
    digitalWrite(motorEn_p,LOW);
    digitalWrite(solenoidEn_p, LOW);
  }
  else //closing door
  {
    digitalWrite(motorEn_p, HIGH);
    digitalWrite(solenoidEn_p,HIGH);
    while (!digitalRead2(doorStop_p))
    {
        if (now() - start > 3) //something is blocking the door or messing with the microswitch
        {
          digitalWrite(motorEn_p, LOW);
          digitalWrite(solenoidEn_p,LOW);
          return false;
        }
        delay(1);
    }
    digitalWrite(motorEn_p, LOW);
    digitalWrite(solenoidEn_p,LOW);
  }
  digitalWrite(13,LOW);
  delay(1000);
  return true;
}

bool checkAlarm(TimeElements alarm)
{
  byte t_second, t_minute, t_hour, dayOfWeek, dayOfMonth, t_month, t_year;
  readDS3231time(&t_second, &t_minute, &t_hour, &dayOfWeek, &dayOfMonth, &t_month, &t_year);
  return (alarm.Hour == int(t_hour) && alarm.Minute == int(t_minute) && int(t_second) == 0) ? true : false; 
}

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

void setDS3231time(byte t_second, byte t_minute, byte t_hour, byte dayOfWeek, byte
dayOfMonth, byte t_month, byte t_year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(t_second)); // set seconds
  Wire.write(decToBcd(t_minute)); // set minutes
  Wire.write(decToBcd(t_hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(t_month)); // set t_month
  Wire.write(decToBcd(t_year)); // set t_year (0 to 99)
  Wire.endTransmission();
}
void readDS3231time(byte *t_second,
byte *t_minute,
byte *t_hour,
byte *dayOfWeek,
byte *dayOfMonth,
byte *t_month,
byte *t_year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *t_second = bcdToDec(Wire.read() & 0x7f);
  *t_minute = bcdToDec(Wire.read());
  *t_hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *t_month = bcdToDec(Wire.read());
  *t_year = bcdToDec(Wire.read());

}
void setCurrTime(int h, int m)
{
  byte t_second, t_minute, t_hour, dayOfWeek, dayOfMonth, t_month, t_year;
  t_second = 0;
  t_minute = m;
  t_hour = h;
  dayOfWeek = 1;
  dayOfMonth = 1;
  t_month = 1;
  t_year = 46;
  setDS3231time(t_second, t_minute, t_hour, dayOfWeek, dayOfMonth, t_month, t_year);
}
TimeElements getCurrTime()
{
  byte t_second, t_minute, t_hour, dayOfWeek, dayOfMonth, t_month, t_year;
  readDS3231time(&t_second, &t_minute, &t_hour, &dayOfWeek, &dayOfMonth, &t_month, &t_year);
  
  TimeElements currTime;
  currTime.Hour = (int)t_hour;
  currTime.Minute = (int)t_minute;
  return currTime;
}

//Below is ugliest hack ive written. 
//I'm ashamed of it and it keeps me up at night, but i was too lazy to rewire
//the reed switch to a digital pin. 
//My deepest apologies to anyone that has the misfortune of seeing this
bool digitalRead2(int pin)
{
  int analogV = analogRead(pin);
  return analogV > 900 ? true : false;
}
/*
void displayTime()
{
  byte t_second, t_minute, t_hour, dayOfWeek, dayOfMonth, t_month, t_year;
  // retrieve data from DS3231
  readDS3231time(&t_second, &t_minute, &t_hour, &dayOfWeek, &dayOfMonth, &t_month,
  &t_year);
  // send it to the serial monitor
  Serial.print(t_hour, DEC);
  // convert the byte variable to a decimal number when displayed
  Serial.print(":");
  if (t_minute<10)
  {
    Serial.print("0");
  }
  Serial.print(t_minute, DEC);
  Serial.print(":");
  if (t_second<10)
  {
    Serial.print("0");
  }
  Serial.print(t_second, DEC);
  Serial.print(" ");
  Serial.print(dayOfMonth, DEC);
  Serial.print("/");
  Serial.print(t_month, DEC);
  Serial.print("/");
  Serial.print(t_year, DEC);
  Serial.print(" Day of week: ");
  switch(dayOfWeek){
  case 1:
    Serial.println("Sunday");
    break;
  case 2:
    Serial.println("Monday");
    break;
  case 3:
    Serial.println("Tuesday");
    break;
  case 4:
    Serial.println("Wednesday");
    break;
  case 5:
    Serial.println("Thursday");
    break;
  case 6:
    Serial.println("Friday");
    break;
  case 7:
    Serial.println("Saturday");
    break;
  }
}
*/
