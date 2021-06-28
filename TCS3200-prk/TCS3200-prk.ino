
/*
 * Created by Linards Smeiksts, Preilu Robotikas klubs
 * Released 6th August 2020
 * 
 * Description:
 * This is example of how to use TCS3200 sensor.
 * Define S2, S3 and OUT pins as digital output pins 
 * Use colorReader.detectColor(timeout) to detect between Red, Green and Blue colors
 * timeout variable sets the timeout for each filter measurement (R, G, B)
 * timeout = 1000 means 1s measurement in Red filter mode, 1s - Green, 1s - blue, total = 3 seconds
 * choose timeout value not less than 50 otherwise false measurements could be made
 * 
 * colorReader.detectColor(timeout) returns 4 different numbers depending on color detected
 * 0 - Undetected color (unknown frequency)
 * 1 - RED
 * 2 - GREEN
 * 3 - BLUE
 */


#include "TCS3200prk.h"

#define S2_PIN    5
#define S3_PIN    4
#define OUT_PIN   1

TCS3200prk colorReader(S2_PIN, S3_PIN, OUT_PIN);

void setup() 
{
}

int i;

void loop() 
{
  i = colorReader.detectColor(50);
  Serial.println(i);
}
