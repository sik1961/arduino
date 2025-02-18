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
#include <strings.h>
#include <Wire.h>

#include "SparkFunBME280.h"

#include "Arduino_LED_Matrix.h"   //Include the LED_Matrix library
// creates an array of two frames

#include <LiquidCrystal.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
const int bmup = byte(0), bmdn = byte(1);
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//BME280 mySensorA; //Uses default I2C address 0x77
BME280 mySensorB; //Uses I2C address 0x76 (jumper closed)
float oldPressure;
float newPressure;

ArduinoLEDMatrix ledMatrix;
byte increase[8][12] = {
  { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0 },
  { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 }
};

byte decrease[8][12] = {
  { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0 },
  { 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 }
};

byte upArrow[8] = {
  0b00100,
  0b01110,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100 
};
byte dnArrow[8] = { 
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b11111,
  0b01110, 
  0b00100
};

void setup()
{
  Serial.begin(115200);
  Serial.println("Example showing alternate I2C addresses");

  Wire.begin();

  //mySensorA.setI2CAddress(0x77); //The default for the SparkFun Environmental Combo board is 0x77 (jumper open).
  //If you close the jumper it is 0x76
  //The I2C address must be set before .begin() otherwise the cal values will fail to load.

  //if(mySensorA.beginI2C() == false) Serial.println("Sensor A connect failed");

  mySensorB.setI2CAddress(0x76); //Connect to a second sensor
  if(mySensorB.beginI2C() == false) Serial.println("Sensor B connect failed");

  ledMatrix.begin();

  oldPressure=0;
  newPressure=0;

  lcd.createChar(0,upArrow);
  lcd.createChar(1,dnArrow);

  lcd.begin(16, 2);
  // you can now interact with the LCD, e.g.:
  //lcd.print("Hello World!");
}

void loop()
{
  // Serial.print("HumidityA: ");
  // Serial.print(mySensorA.readFloatHumidity(), 0);

  // Serial.print(" PressureA: ");
  // Serial.print(mySensorA.readFloatPressure(), 0);

  // Serial.print(" TempA: ");
  // //Serial.print(mySensorA.readTempC(), 2);
  // Serial.print(mySensorA.readTempF(), 2);
  bool rising=true;
  newPressure = mySensorB.readFloatPressure()/100;

  if (newPressure > oldPressure) {
    Serial.print(" Pressure increasing...");
    ledMatrix.renderBitmap(increase, 8, 12);
    rising=true;
  } else {
    Serial.print(" Pressure decreasing...");
    ledMatrix.renderBitmap(decrease, 8, 12);
    rising=false;
  }

  Serial.print(" Humidity: ");
  Serial.print(mySensorB.readFloatHumidity(), 0);

  Serial.print(" Pressure: ");
  Serial.print(newPressure, 0);

  Serial.print(" Temp: ");
  Serial.print(mySensorB.readTempC(), 2);
  //Serial.print(mySensorB.readTempF(), 2);

  Serial.println();

  oldPressure = newPressure;

  for (int i=0; i<60; i++) {
    lcd.clear();
    lcd.print(String(mySensorB.readTempC()) + char(0xDF) + "C");
    lcd.print(" H:" + String(mySensorB.readFloatHumidity()) + "%");
    lcd.setCursor(0, 1);
    lcd.print("P:" + String(newPressure) + " hPa");
    if (rising) {
      lcd.write(bmup);
    } else {
      lcd.write(bmdn);
    }
    delay(1000 * 60);
  }

  //delay(1000 * 60 * 60);
}

