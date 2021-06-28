/*
    TCS32000prk.h - TCS3200 color sensor library
    Created by Linards Smeiksts, 5th August 2020, Preilu Robotics Club    
    Released 6th August 2020
    Version 1.0
*/
#ifndef TCS3200prk_h
#define TCS3200prk_h
  #include "Arduino.h"

  class TCS3200prk
  {
    public:
      TCS3200prk(int s2Pin, int s3Pin, int outPin);
      int detectColor(int timeout);
    private: 
      int _s2Pin, _s3Pin, _outPin;
      void countOnesZeros(int cutoff);
      unsigned int _zeros, _ones, _onesAvg, _onesCollect, _onesCycle, _zerosAvg, _zerosCollect, _zerosCycle, _freqAvgRed, _freqAvgGreen, _freqAvgBlue; 
      unsigned int getFrequency(void);
  };
  
#endif
