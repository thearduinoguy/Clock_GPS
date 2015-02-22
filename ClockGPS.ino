//We always have to include the library
#include "LedControl.h"
#include <Wire.h>
#include <Time.h>
#include <DS1307RTC.h>
#include <TinyGPS.h>
#include <SoftwareSerial.h>

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 4800;
int counter = 0;

tmElements_t tm;

// The TinyGPS++ object
TinyGPS gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin 12: DataIn.   pin 11: CLK.   pin 10: LOAD.   Number of MAX72XX.
 */
LedControl lc=LedControl(12,11,10,1);

/* we always wait a bit between updates of the display */
unsigned long delayTime=100;

void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);

  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,4);
  /* and clear the display */
  lc.clearDisplay(0);
  Serial.println("GPS Clock by Mike McRoberts");
  pinMode(6, OUTPUT); // RED led
  pinMode(7, OUTPUT); // GREEN led
  digitalWrite(6, HIGH); // Red on
  digitalWrite(7, LOW); // Green off
}

void showTime() {

    if (RTC.read(tm)) {
      int hour = tm.Hour;
      int minute = tm.Minute;
      lc.setDigit(0,0,hour/10,false);
      lc.setDigit(0,1,hour%10,true);
      lc.setDigit(0,2,minute/10,false);
      lc.setDigit(0,3,minute%10,false);
    }
  delay(delayTime);
  counter+=1;
}

static void printDateTime()
{
  smartDelay(0);
  int year;
  byte day, month, hour, minute, second, hundredths;
  unsigned long fix_age;
 
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &fix_age);
  
  char timeDateStr[64];
  sprintf(timeDateStr, "%02d:%02d:%02d    ",hour, minute, second);
  Serial.print(timeDateStr);
  
  if (fix_age == TinyGPS::GPS_INVALID_AGE) {
      Serial.print("   No fix detected");
      Serial.print("     Fix Age:"); Serial.print(fix_age);
      digitalWrite(6, HIGH); // Red on
      digitalWrite(7, LOW); // Green off 
    }
  else if (fix_age > 5000)  {
      Serial.print("   Warning: possible stale data!");
      Serial.print("     Fix Age:"); Serial.print(fix_age);
      digitalWrite(6, HIGH); // Red on
      digitalWrite(7, HIGH); // Green on 
   }
  else {
      Serial.print("   Data is current.");
      Serial.print("     Fix Age:"); Serial.print(fix_age);
      //setTime(hour, minute, second,month, day, year);
      tm.Hour = hour;
      tm.Minute = minute;
      tm.Second = second;
      //tm.Day = day;
      //tm.Month = month;
      //tm.Year = year;
      RTC.write(tm);
      Serial.print("    Time Synced.");
      digitalWrite(6, LOW); // Red off
      digitalWrite(7, HIGH); // Green on
      showTime();
  }
  smartDelay(0);
  Serial.println();
  counter=0;
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

void loop() { 
    if(counter>100) {
      printDateTime();
    }
  showTime(); 
}
