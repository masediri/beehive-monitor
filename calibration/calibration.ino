//
//    FILE: HX_kitchen_scale.ino
//  AUTHOR: Matthieu SEDIRI
// VERSION: 0.1.0
// PURPOSE: HX711 calibrate 

//
// HISTORY:
// 0.1.0    2020-06-16 initial version
//

// to be tested 

#include "HX711.h"

HX711 loadCell;

uint8_t dataPin = 6;
uint8_t clockPin = 7;

float w1, w2, previous = 0;

void setup()
{
  Serial.begin(9600); delay(5000);
  Serial.println();
  Serial.println("Starting...");
  
  Serial.println(__FILE__);
  Serial.print("LIBRARY VERSION: ");
  Serial.println(HX711_LIB_VERSION);
  Serial.println();

  loadCell.begin(dataPin, clockPin);


calibrate();
}

void loop()
{
      Serial.println(loadCell.get_units(10));

    // receive command from serial terminal
  if (Serial.available() > 0) {
    float i;
    char inByte = Serial.read();
    if (inByte == 't') loadCell.tare(); //tare
    else if (inByte == 'r') calibrate(); //calibrate
  }


  delay (1000);
  
}

void calibrate() {
  Serial.println("***");
  Serial.println("Start calibration:");
  Serial.println("Place the load cell an a level stable surface.");
  Serial.println("Remove any load applied to the load cell.");
  Serial.println("Send 't' from serial monitor to set the tare offset.");


  boolean _resume = false;
  while (_resume == false) {

    if (Serial.available() > 0) {
            Serial.println("***");

        float i;
        char inByte = Serial.read();
        if (inByte == 't') {
          loadCell.tare();
            Serial.println("tare done:");
          _resume = true;
        }
    }
  }

  Serial.println("Now, place your known mass on the loadcell.");
  Serial.println("Then send the weight of this mass (i.e. 100.0) from serial monitor.");

  float known_mass = 0;
  _resume = false;
  while (_resume == false) {
    if (Serial.available() > 0) {
      known_mass = Serial.parseFloat();
      if (known_mass != 0) {
        Serial.print("Known mass is: ");
        Serial.println(known_mass);
        _resume = true;
      }
    }
  }

  loadCell.calibrate_scale(known_mass,100);
  delay(100);
  float newCalibrationValue = loadCell.get_scale(); //get the new calibration value


  Serial.print("New calibration value has been set to: ");
  Serial.print(newCalibrationValue);

  Serial.println(", use this as calibration value (calFactor) in your project sketch.");

  Serial.println("End calibration");
  Serial.println("***");
  Serial.println("To re-calibrate, send 'r' from serial monitor.");
  Serial.println("For manual edit of the calibration value, send 'c' from serial monitor.");
  Serial.println("***");
}

// END OF FILE
