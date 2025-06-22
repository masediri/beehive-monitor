

#include <SigFox.h>
#include <ArduinoLowPower.h>
#include <HX711_ADC.h>



const int HX711_dout = 6; //mcu > HX711 dout pin
const int HX711_sck = 7; //mcu > HX711 sck pin
const int battery_sck = A0; //input to compute battery level
const int tension_card = 3.3;

const int msg_cycle = 3600 * 12; // sends message every 12h
// si on fait 2h la batterie tient 2 mois

//for 1D97F5
const float calibrationValue = 19.64;

const String softVersion = "1.5";
const bool isDebug = false;

HX711_ADC LoadCell(HX711_dout, HX711_sck);

long t;

void setup() {
  Serial.begin(57600);
  delay(3000);
  Serial.println("On initialise notre ruche connectee!");

  // shutdown built in led 
  pinMode(6, OUTPUT);
  digitalWrite(6, LOW);

  //initialisation à 3,3V
  analogReference(AR_DEFAULT);
  // attach dummy event for deep slepp
  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, alarmEvent0, CHANGE);


  long stabilizingtime = 10000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  LoadCell.begin();

  LoadCell.start(stabilizingtime);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration factor (float)
    Serial.println("Software version is " + softVersion);
    Serial.println("initiating with calibration value=" + String(calibrationValue));
    Serial.println("Startup is complete - we wait 1 min");
  }
  
  if (!isDebug) {
    Serial.println("on attend 1 min avant l'envoi du premier message");
        LowPower.deepSleep(1000 * 60);
  }

}

void loop() {
  
  int weight = 0;
  
  // poids
  weight = readWeightValue();

  Serial.print("Poids : ");
  Serial.print(weight);
  Serial.println(" g ");

  delay(3000);

  //sigFoxMessage.weight = weight;

  //Serial.println("transmission");

  //sendToSigfox (&sigFoxMessage);
  //Serial.println("Tentative de transmission effectuée");



  //Serial.println("On passe en veille profonde pour 12h");
  //LowPower.deepSleep(msg_cycle);
  
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
      Serial.print("Load_cell output val: ");
      Serial.println(w);
      newDataReady = 0;
    }
    t = millis();

  }

  return w;
}


// dummy function to ensure proper wake up of deep sleep mode
void alarmEvent0() {
  volatile int ttt = 0;
}
