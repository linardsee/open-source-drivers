/*
    TCS32000prk.cpp - TCS3200 color sensor library
    Created by Linards Smeiksts, 5th August 2020, Preilu Robotics Club
    Released 6th August 2020
    Version 1.0
*/

#include "Arduino.h"
#include "TCS3200prk.h"

// Uncomment for debug mode
//#define DEBUG

TCS3200prk::TCS3200prk(int s2Pin, int s3Pin, int outPin)
{
  // testbench mode - frequency divider bits connected to Arduino GPIO
  // on the real hardware these bits should be pulled up and pull down using resistor accordingly
  // comment if testbench mode, uncomment if hardware
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);
  
  pinMode(s2Pin, OUTPUT);
  pinMode(s3Pin, OUTPUT);
  pinMode(outPin, OUTPUT);
  _s2Pin = s2Pin;
  _s3Pin = s3Pin;
  _outPin = outPin;

  Serial.begin(115200);
  Serial.begin(115200);
}

int TCS3200prk::detectColor(int timeout)
{
  unsigned long timer;
  unsigned int freqRed, freqGreen, freqBlue;
  int color;
  
  for(int i = 0; i <= 3; i++)
  {
    if(i = 1)
    {
      // Set Red filter
      digitalWrite(_s2Pin, LOW);
      digitalWrite(_s3Pin, LOW); 
      timer = millis();
      
      while( (millis() - timer) < timeout )
      {
        countOnesZeros(1);
      }

      freqRed = getFrequency();
    }
    if(i = 2)
    {
      // Set Green filter
      digitalWrite(_s2Pin, HIGH);
      digitalWrite(_s3Pin, HIGH); 
      timer = millis();
      
      while( (millis() - timer) < timeout )
      {
        countOnesZeros(1);
      }

      freqGreen = getFrequency();
    }
    if(i = 3)
    {
      // Set Red filter
      digitalWrite(_s2Pin, LOW);
      digitalWrite(_s3Pin, HIGH); 
      timer = millis();
      
      while( (millis() - timer) < timeout )
      {
        countOnesZeros(1);
      }

      freqBlue = getFrequency();
    }
  }

  // Check if reflection is not present
  if( (freqRed > 200) && (freqGreen > 200) && (freqBlue > 200))
  {
    #ifdef DEBUG
      Serial.println("UNDETECTED");
    #endif
    
    color = 0;
  }
  else
  {
    if(freqRed < freqGreen)
    {
      if(freqRed < freqBlue)
      {
        #ifdef DEBUG
          Serial.println("RED");
        #endif
        
        color = 1;
      }
      else
      {
        #ifdef DEBUG
          Serial.println("BLUE");
        #endif
        
        color = 3;
      }
    }
    else
    {
      if(freqGreen < freqBlue)
      {
        #ifdef DEBUG
          Serial.println("GREEN");
        #endif
        
        color = 2;
      }
      else
      {
        #ifdef DEBUG
          Serial.println("BLUE");
        #endif
        
        color = 3;
      }
    }
  }

  return color;
}

void TCS3200prk::countOnesZeros(int cutoff)
{
  if(digitalRead(_outPin))
  { 
    if(_zeros > cutoff)
    {
      _zerosCollect += _zeros;
      _zerosCycle++;
    }
    _zeros = 0;
    _ones++;
  }
  else
  {
    if(_ones > cutoff)
    {
      _onesCollect += _ones;
      _onesCycle++;
    }
    _ones = 0;
    _zeros++;
  }
}

unsigned int TCS3200prk::getFrequency(void)
{
  unsigned int freq;
  
  _onesAvg = _onesCollect/_onesCycle;
  _zerosAvg = _zerosCollect/_zerosCycle;
  freq = (_onesAvg + _zerosAvg) / 2;

  #ifdef DEBUG
    Serial.print("Ones total: ");
    Serial.println(_onesCollect);
    Serial.print("Zeros total: ");
    Serial.println(_zerosCollect);
    Serial.print("Ones pulse count: ");
    Serial.println(_onesCycle);
    Serial.print("Zeros pulse count: ");
    Serial.println(_zerosCycle);
    Serial.print("Average pulse width (ones): ");
    Serial.println(_onesAvg);
    Serial.print("Average pulse width (zeros): ");
    Serial.println(_zerosAvg);
    Serial.print("Calculated frequency: ");
    Serial.println(freq);
  #endif

  
  _onesAvg = 0;
  _onesCollect = 0;
  _onesCycle = 0;
  _zerosAvg = 0;
  _zerosCollect = 0;
  _zerosCycle = 0;

  return freq;
}
