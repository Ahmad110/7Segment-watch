#include "Wire.h"
#define DS1307_I2C_ADDRESS 0x68
// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
  return ( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
  return ( (val/16*10) + (val%16) );
}
// 1) Sets the date and time on the ds1307
// 2) Starts the clock
// 3) Sets hour mode to 24 hour clock
// Assumes you're passing in valid numbers
void setDateDs1307(byte second,        // 0-59
byte minute,        // 0-59
byte hour,          // 1-23
byte dayOfWeek,     // 1-7
byte dayOfMonth,    // 1-28/29/30/31
byte month,         // 1-12
byte year)          // 0-99
{
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.write(decToBcd(second));    // 0 to bit 7 starts the clock
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.write(0x10);// sends 0x10 (hex) 00010000 (binary) to control register - turns on square wave
  Wire.endTransmission();
}
// Gets the date and time from the ds1307
void getDateDs1307(byte *second,
  byte *minute,
  byte *hour,
  byte *dayOfWeek,
  byte *dayOfMonth,
  byte *month,
  byte *year)
{
  // Reset the register pointer
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS1307_I2C_ADDRESS, 7);
  // A few of these need masks because certain bits are control bits
  *second     = bcdToDec(Wire.read() & 0x7f);
  *minute     = bcdToDec(Wire.read());
  *hour       = bcdToDec(Wire.read() & 0x3f);  // Need to change this if 12 hour am/pm
  *dayOfWeek  = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month      = bcdToDec(Wire.read());
  *year       = bcdToDec(Wire.read());
}

// Define the bit-patterns for the 7-segment displays 
const byte SevenSeg[16] = 
{
     // Hex digits
     B11111100, B01100000, B11011010, B11110010,   // 0123
     B01100110, B10110110, B10111110, B11100000,   // 4567
     B11111110, B11110110, B11101110, B00111110,   // 89AB
     B00011010, B01111010, B10011110, B10001110,   // CDEF
  
};
const byte carma[4] =
{
  B10000000, B01000000, B00100000, B00010000,
};
// Pin connected to latch pin (RCLK,12) of 74HC595
const int latchPin = 2;
// Pin connected to clock pin (SRCLK,11) of 74HC595
const int clockPin = 1;
// Pin connected to Data in (SER,14) of 74HC595
const int dataPin  = 3;
// Pin connected to cathodes
const int laPin = 6;
const int clPin = 5;
const int daPin = 7;
const int in1 = 9;
const int in2 = 10;
const int in3 = 11;
unsigned long previousMillis = 0;
const long interval = 200;

void setup() 
{
  Wire.begin();
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin,  OUTPUT);
  pinMode(laPin, OUTPUT);
  pinMode(daPin, OUTPUT);
  pinMode(clPin, OUTPUT);
  pinMode(in1, INPUT);
  pinMode(in2, INPUT);
  pinMode(in3, INPUT);
}  
//
void loop() 
{
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  
  getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  
  SevenSegDisplay(hour, minute, second);

  if (digitalRead(in1) == HIGH || digitalRead(in2) == HIGH || digitalRead(in3)== HIGH){
  
    sTime();
  
  }

}

void DisplayADigit(byte dispno, byte digit2disp)
{
  
  // Turn off the shift register pins
  // while you're shifting bits:
  digitalWrite(latchPin, LOW);
  
  AllDispOff();  // Turn off all cathode drivers.
  
  // shift the bits out:
  shiftOut(dataPin, clockPin, LSBFIRST, digit2disp);
  
  digitalWrite(latchPin, HIGH);  // Set latch high to set segments.

  digitalWrite(laPin, LOW);

  shiftOut(daPin, clPin, LSBFIRST, dispno);

  digitalWrite(laPin, HIGH);
  
  delay(3);  // Wait a bit for POV

}

void AllDispOff()
{
  // Turn all cathode drivers off
  digitalWrite(laPin, LOW);

  shiftOut(daPin, clPin, LSBFIRST, 0);

  digitalWrite(laPin, HIGH);

  delay(1);
}

byte yekan(byte ye)
{
  if(ye<10){
    return ye;
  }else{
    ye = ye%10;
    return ye;
  }
}

byte dahgan(byte da)
{
  if(da<10){
    return 0;
  }else{
    da = da/10;
    return da;
  }
}

void SevenSegDisplay(byte ho, byte mi, byte se)
{
  byte d1,d2,d3,d4,d5,d6; 
  
  d1 = dahgan(ho);    
  d2 = yekan(ho);     
  d3 = dahgan(mi);      
  d4 = yekan(mi);       
  d5 = dahgan(se);
  d6 = yekan(se);
  
  DisplayADigit(byte(carma[0]), byte(SevenSeg[d1]));  
  DisplayADigit(byte(carma[1]), byte(SevenSeg[d2]));  
  DisplayADigit(byte(carma[2]), byte(SevenSeg[d3]));  
  DisplayADigit(byte(carma[3]), byte(SevenSeg[d4]));  

}

void sTime(){
  if (digitalRead(in1) == HIGH && digitalRead(in2) == LOW){
    
      byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
      
      getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);

      unsigned long currentMillis = millis();

      if (currentMillis - previousMillis >= interval) {
    
         previousMillis = currentMillis;
  
          minute ++;
    
          if (minute > 59){
    
              minute = 0;
            
          }
          
          setDateDs1307(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
          
        }
      
    }

  if (digitalRead(in2) == HIGH && digitalRead(in1) == LOW){

      byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
      
      getDateDs1307(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
      
       unsigned long currentMillis = millis();

      if (currentMillis - previousMillis >= interval) {
    
         previousMillis = currentMillis;
  
          hour ++;
    
          if (hour > 23){
    
              hour = 0;
            
          }
          
          setDateDs1307(second, minute, hour, dayOfWeek, dayOfMonth, month, year);
          
        }
      
    }

}
