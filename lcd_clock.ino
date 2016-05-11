#include <LiquidCrystal.h>
#include "LCDKeypad.h"
#include "Wire.h"

//DS3231 address bus:
#define DS3231_I2C_ADDRESS 0x68

//Date:
#define YEAR 0
#define MONTH 1
#define DAYS 2
#define WEEKDAY 3

//weekday state:
#define SUN 0
#define MON 1
#define TUE 2
#define WED 3
#define THU 4
#define FRI 5
#define SAT 6

//Time:
#define HOURS 0
#define MINUTES 1
#define SECONDS 2

//menu list:
#define IDLEMENU 0
#define SETDATE 1
#define SETTIME 2
#define DONE 3

// The LCD screen
LCDKeypad lcd;

// The time model
byte year = 0;
byte month = 1;
byte days = 1;
byte hours = 0;
byte minutes = 0;
byte seconds = 0;
byte weekday = 0;
byte datesetting = 0;
byte timesetting = 0;
byte menustate = 0;

void setup() {
  //power up ZS-042
  pinMode(53, OUTPUT);
  digitalWrite(53, HIGH);
  //initialize I2C and lcd: 
  lcd.begin(16,2);
  Wire.begin();
  // Set the cursor at the begining of the first row
  lcd.setCursor(0,0);

  // Print a text in the first row
  lcd.print("PRESS SELECT    ");
  lcd.setCursor(0,1);
  lcd.print("TO SET DATETIME ");
  //setDS3231time(30,06,17,3,11,5,16);
  //delay(1000);
  readDS3231time(&seconds, &minutes, &hours, &weekday, &days, &month, &year);
}

void loop() {
  // Increase the time model by one second
  incTime();
  //readDS3231time(&seconds, &minutes, &hours, &weekday, &days, &month, &year);
  // Print the time on the LCD
  printTime();

  // Listen for buttons for 1 second
  buttonListen();
}

void buttonListen() {
  // Read the buttons five times in a second
  for (int i = 0; i < 5; i++) {

    // Read the buttons value
    int button = lcd.button();
    switch (menustate){
       case SETDATE:
          switch (button) {
            // Right button was pushed
            case KEYPAD_RIGHT:
              datesetting++;
              break;
        
            // Left button was pushed
            case KEYPAD_LEFT:
              datesetting--;
              if (datesetting == -1) datesetting = 3;
              break;
        
            // Up button was pushed
            case KEYPAD_UP:
              switch (datesetting) {
              case YEAR:
                year++;
                break;
              case MONTH:
                month++;
                break;
              case DAYS:
                days++;
                break;
              case WEEKDAY:
                weekday++;
              }     
              break;
        
            // Down button was pushed
            case KEYPAD_DOWN:
              switch (datesetting) {
              case YEAR:
                year--;
                if (year == -1) year = 99;
                break;
              case MONTH:
                month--;
                if (month == 0) month = 12;
                break;
              case DAYS:
                days--;
                if (days == 0) days = 31;
                break;
              case WEEKDAY:
                weekday--;
                if (weekday == 0) days = 6;
              }
            }
       break;
       case SETTIME:
            switch (button) {
            // Right button was pushed
            case KEYPAD_RIGHT:
              timesetting++;
              break;
        
            // Left button was pushed
            case KEYPAD_LEFT:
              timesetting--;
              if (timesetting == -1) timesetting = 2;
              break;
        
            // Up button was pushed
            case KEYPAD_UP:
              switch (timesetting) {
                case HOURS:
                  hours++;
                  break;
                case MINUTES:
                  minutes++;
                  break;
                case SECONDS:
                  seconds++;
                }     
              break;
        
            // Down button was pushed
            case KEYPAD_DOWN:
              switch (timesetting) {
                case HOURS:
                  hours--;
                  if (hours == -1) hours = 23;
                  break;
                case MINUTES:
                  minutes--;
                  if (minutes == -1) minutes = 59;
                  break;
                case SECONDS:
                  seconds--;
                  if (seconds == -1) seconds = 59;
                }
            }
          break;
        case DONE:
           // DS3231 seconds, minutes, hours, day, date, month, year
           if (button== KEYPAD_SELECT) {
              setDS3231time(seconds,minutes,hours,weekday,days,month,year);
           }
    }
    if (button== KEYPAD_SELECT) {
      menustate++;
    }
    datesetting %= 4;
    timesetting %= 3;
    menustate %=4;
    printSetting();

    year %= 100;
    month %= 13;
    days %= 32;
    hours %= 24;
    minutes %= 60;
    seconds %= 60;
    weekday %=7;
    printTime();

    // Wait one fifth of a second to complete
    while(millis() % 200 != 0);
  }
}

// Print the current setting
void printSetting() {
    switch (menustate){
        case SETDATE:
          lcd.setCursor(0,0);
          lcd.print("SETTING:        ");
          lcd.setCursor(9,0);
          switch (datesetting) {
            case YEAR:
            lcd.print("YEAR   ");
            break;  
            case MONTH:
            lcd.print("MONTH  ");
            break;  
          case DAYS:
            lcd.print("Days   ");
            break;
          case WEEKDAY:
            lcd.print("WeekDay");
          }
        break;
        case SETTIME:
          lcd.setCursor(0,0);
          lcd.print("SETTING:        ");
          lcd.setCursor(9,0);
          switch (timesetting) {  
            case HOURS:
              lcd.print("Hours  ");
              break;
            case MINUTES:
              lcd.print("Minutes");
              break;
            case SECONDS:
              lcd.print("Seconds");
          }
          break; 
        case IDLEMENU:
          lcd.setCursor(0,0);
          lcd.print("PRESS SELECT BTN");
          lcd.setCursor(0,1);
          lcd.print("TO SET DATETIME ");
          break;
        case DONE:
          lcd.setCursor(0,0);
          lcd.print("DONE            ");
          lcd.setCursor(0,1);
          lcd.print("................");
      }
}

// Increase the time model by one second
void incTime() {
  // Increase seconds
  seconds++;

  if (seconds == 60) {
    // Reset seconds
    seconds = 0;

    // Increase minutes
    minutes++;

    if (minutes == 60) {
      // Reset minutes
      minutes = 0;

      // Increase hours
      hours++;

      if (hours == 24) {
        // Reset hours
        hours = 0;

        // Increase days
        days++;
        weekday++;
      }
    }
  }
}

// Print the time on the LCD
void printTime() {
  char time[17];
  //check which state right now:
  switch(menustate){
    case SETDATE:
      lcd.setCursor(0,1);      
      switch(weekday){
        case SUN:
          sprintf(time, "%02i/%02i/%02i     SUN", year, month, days);
          break;
        case MON:
          sprintf(time, "%02i/%02i/%02i     MON", year, month, days);
          break;
        case TUE:
          sprintf(time, "%02i/%02i/%02i     TUE", year, month, days);
          break;
        case WED:
          sprintf(time, "%02i/%02i/%02i     WED", year, month, days);
          break;
        case THU:
          sprintf(time, "%02i/%02i/%02i     THU", year, month, days);
          break;
        case FRI:
          sprintf(time, "%02i/%02i/%02i     FRI", year, month, days);
          break;
        case SAT:
          sprintf(time, "%02i/%02i/%02i     SAT", year, month, days);
      }
      lcd.print(time);
      break;
    case SETTIME:
      lcd.setCursor(0,1);
      sprintf(time, "%02i:%02i:%02i        ", hours, minutes, seconds);
      lcd.print(time);
    }
  // Set the cursor at the begining of the second row
  
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

void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte dayOfMonth, byte month, byte year)
{
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (0=Sunday, 6=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}
void readDS3231time(byte *second,byte *minute,byte *hour,byte *dayOfWeek,byte *dayOfMonth,byte *month,byte *year)
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}
