#include <SigFox.h>
#include <ArduinoLowPower.h>
#include <HX711_ADC.h>
#include "def.h"   // Librairie des structures


const int HX711_dout = 6; //mcu > HX711 dout pin
const int HX711_sck = 7; //mcu > HX711 sck pin
const int battery_sck = A0; //input to compute battery level
const int tension_card = 3.3;

const int msg_cycle = 1000 * 3600 * 12; // sends message every 12h

//for 1D97F5
const float calibrationValue = 19.64;

const String softVersion = "1.0";
const bool isDebug = true;

HX711_ADC LoadCell(HX711_dout, HX711_sck);

long t;

void setup() {
  Serial.begin(57600);
  // wait for 3 sec for console display
  delay(3000);
  Serial.println("On initialise notre ruche connectee!");

  // shutdown built in led 
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);

  //initialisation à 3,3V
  analogReference(AR_DEFAULT);
  // attach dummy event for deep slepp
  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, alarmEvent0, CHANGE);


  long stabilizingtime = 10000; // tare precicion can be improved by adding a few seconds of stabilizing time
  LoadCell.begin();

  LoadCell.start(stabilizingtime);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration factor (float)
    Serial.println("Software version is " + softVersion);
    Serial.println("initiating with calibration value=" + String(calibrationValue));
    Serial.println("Startup is complete - you can put the load on ");
  }
  
  if (!isDebug) {
    Serial.println("on attend 3 min avant l'envoi du premier message le temps de poser la ruche");
    LowPower.deepSleep(1000 * 60 * 3);
  }
  else {
    Serial.println("on attend juste 1 min");
    LowPower.deepSleep(1000 * 60);
  }

}

// During execution we send weight to sigfox bus
void loop() {
  
  int weight = 0;
  
  // get weight from the cell
  weight = readWeightValue();

  struct SigFoxMessage sigFoxMessage;
  sigFoxMessage.weight = weight;

  if (!isDebug) {
    Serial.println("debug mode disabled - sending data to sigfox network");
    sendToSigfox (&sigFoxMessage);
    Serial.println("sending done");

  Serial.println("We go for sleep for the next 12h");
  //
  LoadCell.powerDown();
  LowPower.deepSleep(msg_cycle);
  }
  else {
      Serial.println("debug mode enabled - no data will be sent");
      Serial.print("weight=");
      Serial.print(sigFoxMessage.weight);
      Serial.println(" g");

    //wait for 1 min
    Serial.println("We wait for 1 min for the next capture");
    LoadCell.powerDown();
    LowPower.deepSleep(1000*60);  }
}

// send value deca-grams to not transfert useless data
int readWeightValue() {
  static boolean newDataReady = 0;
  const int interval = 7000; //time in ms to perform measure
  float w = -1.0;

  long t0 = millis();
  long t = t0;

  while (t < t0 + interval) {
    // check for new data/start next conversion:
    if (LoadCell.update()) newDataReady = true;

    // get smoothed value from the dataset:
    if (newDataReady) {
      w = LoadCell.getData();
      newDataReady = 0;
    }
    t = millis();

  }

  return w;
}

// return is 0 if OK and greater than 0 if problem
int sendToSigfox(SigFoxMessage* sigFoxMessage) {

  Serial.println("Init du module");

  // Start the module
  SigFox.begin();
  // Enable debug led and disable automatic deep sleep
  if (isDebug) {
    SigFox.debug();
  }
  // Wait at least 30mS after first configuration (100mS before)
  delay(100);
  // Clears all pending interrupts
  //SigFox.status();
  //delay(1);

  Serial.println("On commence le packetage");
  if (!isDebug) {
    SigFox.beginPacket();

    Serial.println("On envoie la sauce");

    SigFox.write(sigFoxMessage->temp);
    SigFox.write(sigFoxMessage->humidity);
    SigFox.write(sigFoxMessage->battery);
    SigFox.write(sigFoxMessage->weight);

    int ret = 0;
    ret = SigFox.endPacket(false); // send buffer to SIGFOX network
    Serial.println("Tentative d'envoi terminee");

    if (ret > 0) {
      Serial.println("No transmission");
    } else {
      Serial.println("Transmission ok");
    }
  }
  SigFox.end();
}

// dummy function to ensure proper wake up of deep sleep mode
void alarmEvent0() {
  volatile int ttt = 0;
}


