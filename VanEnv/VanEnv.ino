/*
  Van External Environment Display
  with adjustable QNH 

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

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

long ONE_SECOND=1000;

const float QNH_INIT=101300.00;
float QNH=101300.00;
const float QNH_MAX=104000.00;
const float QNH_MIN=96000.00;
int COUNTER=0;
int MINUTE=0;
int BTN_COUNTER=0;
int QNH_COUNTER=0;
float SAVED_PRESSURE=0;
bool STILL_UPDATING = true;        

BME280 bmeSensor; //Uses I2C address 0x76 (jumper closed)

void setup()
{
  Serial.begin(9600);

  pinMode(ENC_A, INPUT);
  pinMode(ENC_SW, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A), encoder_moved, LOW);
  attachInterrupt(digitalPinToInterrupt(ENC_B), encoder_moved, LOW);

  Wire.begin();
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  bmeSensor.setI2CAddress(0x76); //Connect to a second sensor
  if(bmeSensor.beginI2C() == false) {
    Serial.println(F("Sensor B connect failed"));
    for(;;);
  }  

  SAVED_PRESSURE=bmeSensor.readFloatPressure();
  String rorf="^";
  QNH=QNH_INIT;
}

/* Main Loop */
void loop()
{
  COUNTER++;

  if (!digitalRead(ENC_SW)) {
    BTN_COUNTER++;
    if (BTN_COUNTER>3) {
      update_qnh();
    }
  } else {
    BTN_COUNTER=0;
  }

  if (COUNTER>59) {
    update_env();
    MINUTE++;
    if (MINUTE>29) {
      SAVED_PRESSURE=bmeSensor.readFloatPressure();
      MINUTE=0;
    }  
    COUNTER=0;
  }

  // Serial.print(String("COUNTER: "));
  // Serial.println(COUNTER);

  // Serial.print(String("BTN_COUNTER: "));
  // Serial.println(BTN_COUNTER);

  // Serial.print(String("SAVED_PRESSURE: "));
  // Serial.println(SAVED_PRESSURE);

  // Serial.print(String("QNH: "));
  // Serial.println(QNH);

  // Serial.print("HumidityA: ");
  // Serial.print(bmeSensor.readFloatHumidity());

  // Serial.print(" PressureA: ");
  // Serial.print(bmeSensor.readFloatPressure()/100, 0);

  // Serial.print(" TempA: ");
  // Serial.println(bmeSensor.readTempC(), 2);

  delay(ONE_SECOND);
}

/* The encoder was turned - adjust QNH */
void encoder_moved() {
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
    bmeSensor.setReferencePressure(QNH);
    COUNTER=60;
  }
}

/* Update environmental display */
void update_env() {
  display.clearDisplay();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("T: "));
  display.print(bmeSensor.readTempC());
  display.println(F("c"));

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.print(F("H: "));
  display.print(bmeSensor.readFloatHumidity());
  display.println(F("%"));

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.print(F("P:"));
  display.print(bmeSensor.readFloatPressure()/100);
  if (bmeSensor.readFloatPressure()>SAVED_PRESSURE) {
    display.println("^");
  } else  {
    display.println("v");
  }

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.print(F("A:"));
  display.print(bmeSensor.readFloatAltitudeMeters());
  display.println("m");

  display.display();
  return;
}

/** display the QNH to show the value during adjustment */
void update_qnh() {

  QNH_COUNTER=0;
  STILL_UPDATING=true;
  while (STILL_UPDATING) {

    if (!digitalRead(ENC_SW)) {
      QNH_COUNTER++;
      if (QNH_COUNTER>2) {
        STILL_UPDATING=false;
      }
    } else {
      QNH_COUNTER=0;
    }

    display.clearDisplay();
    display.setTextSize(2);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println(F("QNH: "));
    display.println(QNH);

    display.display();
    delay(100);

  }
  BTN_COUNTER=0;
  COUNTER=60; 
  return;
}
