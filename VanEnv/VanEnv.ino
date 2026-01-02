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

// Define rotary encoder pins
#define ENC_A 2
#define ENC_B 3
#define ENC_SW 8

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

//#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

long ONE_MINUTE=1000*60;

const float QNH_INIT=101300.00;
float QNH=101300.00;
const float QNH_MAX=104000.00;
const float QNH_MIN=80000.00;
int COUNTER=0;
float SAVED_PRESSURE=0;        

//BME280 mySensorA; //Uses default I2C address 0x77
BME280 mySensorB; //Uses I2C address 0x76 (jumper closed)

void setup()
{
  Serial.begin(9600);

  pinMode(ENC_A, INPUT);
  pinMode(ENC_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A), shaft_moved, LOW);
  attachInterrupt(digitalPinToInterrupt(ENC_B), shaft_moved, LOW);

  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  mySensorB.setI2CAddress(0x76); //Connect to a second sensor
  if(mySensorB.beginI2C() == false) Serial.println("Sensor B connect failed");

  //mySensorB.setReferencePressure(QNH);
  SAVED_PRESSURE=mySensorB.readFloatPressure();
  String rorf="^";
  QNH=QNH_INIT;
}

void loop()
{
  COUNTER++;

  if (!digitalRead(ENC_SW)) {
    QNH=SAVED_PRESSURE;
  }

  if (COUNTER>59) {
    SAVED_PRESSURE=mySensorB.readFloatPressure();
    mySensorB.setReferencePressure(QNH);
    COUNTER=0;
  }

  Serial.print(String("COUNTER: "));
  Serial.println(COUNTER);

  Serial.print(String("SAVED_PRESSURE: "));
  Serial.println(SAVED_PRESSURE);

  Serial.print(String("QNH: "));
  Serial.println(QNH);

  Serial.print("HumidityA: ");
  //Serial.print(String(mySensorB.readFloatHumidity()));
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
    Serial.println(String("^Rising"));
    display.println("^");
  } else  {
    Serial.println(String("vFalling"));
    display.println("v");
  }

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.print(F("A:"));
  display.print(mySensorB.readFloatAltitudeMeters());
  display.println("m");

  display.display();

  delay(1000);
}

void shaft_moved() {
  static unsigned long lastInterruptTime=0;
  unsigned long interruptTime=millis();
  if (interruptTime - lastInterruptTime > 5) {
    if (digitalRead(ENC_A) == HIGH) {
      if (QNH<QNH_MAX) {
        QNH=QNH+100;
      }  
    }
    if (digitalRead(ENC_A) == LOW) {
      if (QNH>QNH_MIN) {
        QNH=QNH-100;
      }  
    }
    lastInterruptTime=interruptTime;
    COUNTER=60;
  }
}

// void btn_pushed() {
//   static unsigned long lastInterruptTime=0;
//   unsigned long interruptTime=millis();
//   if (interruptTime - lastInterruptTime > 5) {
//     if (digitalRead(BTN)==1) {
//       QNH=SAVED_PRESSURE;
//     }
//     lastInterruptTime=interruptTime;
//     COUNTER=60;
//   }
// }

