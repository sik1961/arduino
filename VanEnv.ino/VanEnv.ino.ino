/*
  Communicate with BME280s with different I2C addresses
  Nathan Seidle @ SparkFun Electronics
  March 23, 2015

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/14348 - Qwiic Combo Board
  https://www.sparkfun.com/products/13676 - BME280 Breakout Board

  This example shows how to connect two sensors on the same I2C bus.

  The BME280 has two I2C addresses: 0x77 (jumper open) or 0x76 (jumper closed)

  Hardware connections:
  BME280 -> Arduino
  GND -> GND
  3.3 -> 3.3
  SDA -> A4
  SCL -> A5
*/

#include <Wire.h>

#include "SparkFunBME280.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

BME280_SensorMeasurements *measurements;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

#define ONE_MINUTE    1000*60

#define QNH           1019.00
#define COUNTER       0
#define SAVED_PRESSURE 0        

BME280 mySensorA; //Uses default I2C address 0x77
BME280 mySensorB; //Uses I2C address 0x76 (jumper closed)

void setup()
{
  Serial.begin(9600);

  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  

  //mySensorA.beginI2C(TwoWire &wirePort = Wire)
  //mySensorA.setI2CAddress(0x76); //The default for the SparkFun Environmental Combo board is 0x77 (jumper open).
  //If you close the jumper it is 0x76
  //The I2C address must be set before .begin() otherwise the cal values will fail to load.

  //if(mySensorA.beginI2C() == false) Serial.println("Sensor A connect failed");

  mySensorB.setI2CAddress(0x76); //Connect to a second sensor
  if(mySensorB.beginI2C() == false) Serial.println("Sensor B connect failed");

  mySensorB.setReferencePressure(QNH * 100);
  float savedPressure = mySensorB.readFloatPressure();
  String rorf="^";
}

void loop()
{
  COUNTER==COUNTER+1;

  if (COUNTER>59) {
    SAVED_PRESSURE==mySensorB.readFloatPressure();
    COUNTER==0;
  }



  Serial.print("HumidityA: ");
  Serial.print(String(mySensorB.readFloatHumidity()));
  Serial.print(mySensorB.readFloatHumidity());

  Serial.print(" PressureA: ");
  Serial.print(mySensorB.readFloatPressure()/100, 0);

  Serial.print(" TempA: ");
  Serial.print(mySensorB.readTempC(), 2);

  //Serial.print(" DewPointA: ");
  //Serial.print(mySensorB.readTempC(), 2);
  //Serial.print(mySensorB.readTempF(), 2);
  //Serial.print(mySensorB.readAllMeasurements(measurements));

  display.clearDisplay();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("T: "));
  display.print(mySensorB.readTempC());
  display.println(F("c"));

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  //display.setCursor(0,0);             // Start at top-left corner
  display.print(F("H: "));
  display.print(mySensorB.readFloatHumidity());
  display.println(F("%"));

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.print(F("P:"));
  display.print(mySensorB.readFloatPressure()/100);
  if (mySensorB.readFloatPressure()>SAVED_PRESSURE) {
    display.println("^");
  } else  {
    display.println("v");
  }

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.print(F("A:"));
  display.print(mySensorB.readFloatAltitudeMeters());
  display.println("m");

  display.display();

  delay(1000*10);
}

