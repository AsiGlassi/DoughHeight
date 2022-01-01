#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_VL6180X.h>
 
#include "BLEDoughHeight.h"

#define VL6180X_ADDRESS 0x29

//Debug
#define debugMode false

//TOF 
VL6180xIdentification identification;
VL6180x disSensor(VL6180X_ADDRESS);

//BLE
BLEDoughHeight xBleDoughHeight;

//interval
unsigned long sendInterval = 3000;
unsigned long lastSentTime = 0;

void printIdentification(struct VL6180xIdentification *temp) {
  Serial.print("Model ID = ");
  Serial.println(temp->idModel);

  Serial.print("Model Rev = ");
  Serial.print(temp->idModelRevMajor);
  Serial.print(".");
  Serial.println(temp->idModelRevMinor);

  Serial.print("Module Rev = ");
  Serial.print(temp->idModuleRevMajor);
  Serial.print(".");
  Serial.println(temp->idModuleRevMinor);

  Serial.print("Manufacture Date = ");
  Serial.print((temp->idDate >> 3) & 0x001F);
  Serial.print("/");
  Serial.print((temp->idDate >> 8) & 0x000F);
  Serial.print("/1");
  Serial.print((temp->idDate >> 12) & 0x000F);
  Serial.print(" Phase: ");
  Serial.println(temp->idDate & 0x0007);

  Serial.print("Manufacture Time (s)= ");
  Serial.println(temp->idTime * 2);
  Serial.println();
  Serial.println();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println("Starting up");
  
  pinMode(LED_BUILTIN, OUTPUT);

  Wire.begin();
  disSensor.getIdentification(&identification); // Retrieve manufacture info from device memory
  printIdentification(&identification);      // Helper function to print all the Module information

  if (disSensor.VL6180xInit() != 0) {
    Serial.println("Failed to initialize. Freezing..."); // Initialize device and check for errors
    while (1)
      ;
  }

  disSensor.VL6180xDefautSettings(); // Load default settings to get started.

  //init BLE
  xBleDoughHeight.initBLE();

  delay(1000); 
}

void loop() {
  
  if (xBleDoughHeight.isDeviceConnected()) {
    
    unsigned long now = millis();
    if ((now - lastSentTime) > sendInterval) {

      lastSentTime = now;
      uint8_t dis = disSensor.getDistance();
      if (debugMode) {
        // Get Ambient Light level and report in LUX
        Serial.print("Ambient Light Level (Lux) = ");
        // Input GAIN for light levels,
        //  GAIN_20     // Actual ALS Gain of 20
        //  GAIN_10     // Actual ALS Gain of 10.32
        //  GAIN_5      // Actual ALS Gain of 5.21
        //  GAIN_2_5    // Actual ALS Gain of 2.60
        //  GAIN_1_67   // Actual ALS Gain of 1.72
        //  GAIN_1_25   // Actual ALS Gain of 1.28
        //  GAIN_1      // Actual ALS Gain of 1.01
        //  GAIN_40     // Actual ALS Gain of 40
        Serial.println(disSensor.getAmbientLight(GAIN_1));

        // Get Distance and report in mm
        Serial.print("Distance measured (mm) = ");
        Serial.println(dis);
      }
      xBleDoughHeight.sendHeightData(dis);
    }
  }
  delay(500);
}

