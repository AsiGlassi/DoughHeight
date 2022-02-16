#include <RTClib.h>
#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <SparkFun_VL6180X.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <Adafruit_PN532.h>

 
#include "CyrcularAvg.h"
#include "LedDough.h"
#include "BLEDoughHeight.h"
#include "DoughServcieStatus.h"


//Debug
// #define DEBUG_MAIN true

#define BUZZ_PIN 14

//RTC 
RTC_DS3231 rtc;
// RTC_DS1307 rtc;

//TOF - Distance measures
#define VL6180X_ADDRESS 0x29
VL6180xIdentification identification;
VL6180x disSensor(VL6180X_ADDRESS);

//Dough config
uint8_t currDoughDist = 0;
CyrcularAvg avgDistance(10);
uint8_t currDoughFermPercent = 0;
int distanseEpsilon = 2;
int minDoughHeight = 20;
int floorDist=0;

//Pixel
LedDough leds;

//Service Status
DoughServcieStatus doughServcieStatus;

//BLE
BLEDoughHeight xBleDoughHeight(&doughServcieStatus);

// Service Status files
const char *lastSettingsFileName = "/LastSettings.json";

//pn352
#define PN532_IRQ   (34)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
#ifdef ESP32
    uint32_t frequency = 100000;
#else
    uint32_t frequency = 1000000;
#endif
const int DELAY_BETWEEN_CARDS = 500;
long timeLastCardRead = 0;
boolean readerDisabled = false;
volatile bool cardReadWaiting = false;

//cup presence 
#define CUP_PRESENCE_IRQ (35) 
hw_timer_t * cupPresenceTimer = NULL; 

//interval
unsigned long sendInterval = 3000;
unsigned long lastSentTime = 0;
unsigned int fermentationAgingSpan = 8*60;//in minutes 


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


// Saves Dough Service Startus to a file
void saveStatus() {

  if (SPIFFS.exists(lastSettingsFileName)) {
    //File Exist
    SPIFFS.remove(lastSettingsFileName);
  }

  // Open file for writing
  File file = SPIFFS.open(lastSettingsFileName, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create Last Settings file."));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<128> doc;

  // Set the values in the document
  doc["DoughServcieStatus"] = doughServcieStatus.getDoughServcieStatusEnum();
  char strFormat[] = "MM-DD-YYYYThh:mm:ss";
  DateTime startTime = doughServcieStatus.getFermentationStart();
  doc["FermentationStart"] = startTime.toString(strFormat);
  doc["DoughInitDist"] = doughServcieStatus.getDoughInitDist();
  doc["CupBaseDis"] = doughServcieStatus.getCupBaseDist();

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write Last Settings file."));
  }

  // Close the file
  file.close();
}

//Read service status file and update the Class
void readStatus() {

  //read
  if (SPIFFS.exists(lastSettingsFileName)) {
    //File Exist
    File file2 = SPIFFS.open(lastSettingsFileName);
 
    if(!file2){
        Serial.println("Failed to open Last Settings file for reading.");
        return;
    }
 
    // Serial.println("File Content:"); 
    // while(file2.available()) {
      // Serial.write(file2.read());
    // }
    // Serial.println();

    StaticJsonDocument<256> doc;
    // JsonObject obj = doc.as<JsonObject>();
    DeserializationError error = deserializeJson(doc, file2);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("Deserialize Last Settings Json failed: "));
      Serial.println(error.f_str());
      return;
    }
      
    DoughServcieStatusEnum statusFromFile = doc["DoughServcieStatus"].as<DoughServcieStatusEnum>();
    Serial.printf("Status read from the file: %d\n", statusFromFile);
    if (statusFromFile == DoughServcieStatusEnum::Fermenting) { 

      //check fermentation start time
      const char* fileDateStr = doc["FermentationStart"].as<const char*>();
      DateTime fermStarted = DateTime(fileDateStr);
      TimeSpan startedBefore = (rtc.now() - fermStarted);
      Serial.printf("Last operation stopped during fermentation at %s - %d min ago.\n", 
                    fileDateStr, startedBefore.minutes());

      if (startedBefore.minutes() < fermentationAgingSpan) {
        //Continue fermentation
        Serial.printf("Continue fermentation.\n");
        doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::Fermenting);
        doughServcieStatus.setFermentationStart(fermStarted);
        doughServcieStatus.setDoughInitDist(doc["DoughInitDist"]);
        doughServcieStatus.setCupBaseDist(doc["CupBaseDis"]);
      }
    }
    file2.close();
  } 
}


void StartFermentation() {
    
    if (currDoughDist == 0) {
      Serial.println("Cant start process, Distance = 0.");
      doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::Error);
    } else if (abs(currDoughDist - doughServcieStatus.getCupBaseDist()) < minDoughHeight) {
      Serial.println("Cant start process, Dough level is too low.");
      doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::Error);
    } else {
      DateTime currTime = rtc.now();
      char strFormat[] = "MM-DD-YYYYThh:mm:ss";
      Serial.print("Start Fermentation Process: ");Serial.print(currTime.toString(strFormat));
      Serial.print("\tInitialize Dough Distance: "); Serial.println(currDoughDist);
      
      //set init distance 
      doughServcieStatus.setDoughInitDist(currDoughDist);

      //set status
      doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::Fermenting);
      doughServcieStatus.setFermentationStart(currTime);

      //Set Light status
      leds.Fermenting();

      //update BLE device status changed
      xBleDoughHeight.sendStatustData(doughServcieStatus.getDoughServcieStatusEnum());

      //save service status iin case the system restart
      saveStatus();
    }
}

void ContFermenting() {
      // Serial.println("Continue Fermentation Process.");

      //set status
      doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::Fermenting);

      //Set Light status
      leds.Fermenting();

      //update BLE device status changed
      xBleDoughHeight.sendStatustData(doughServcieStatus.getDoughServcieStatusEnum());
}

void StopFermentation() {
    Serial.println("Stop Fermentation Process.");

    //set status
    doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::idle);

    //Set Light status
    leds.idle();

    //update BLE device status changed
    xBleDoughHeight.sendStatustData(doughServcieStatus.getDoughServcieStatusEnum());
}

void ReachedDesiredFermentation() {
     Serial.println("Reached Desired Fermentation.");

    //set status
    doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::ReachedDesiredFerm);

    //Set Light status
    leds.ReachedDesiredFerm();

    //update BLE device status changed
    xBleDoughHeight.sendStatustData(doughServcieStatus.getDoughServcieStatusEnum());

    //Make some sounds for
    digitalWrite(BUZZ_PIN, HIGH);
    delay(1250);
    digitalWrite(BUZZ_PIN, LOW);
}

void OverFermentation() {
     Serial.println("Dough Over Fermentating.");

    //set status
    doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::OverFerm);

    //Set Light status
    leds.OverFermentation();

    //update BLE device status changed
    xBleDoughHeight.sendStatustData(doughServcieStatus.getDoughServcieStatusEnum());
}



class DoughServiceBLECallback: public DoughServiceBLECallbacks {
public:
  void onStart() {
    StartFermentation();
  }

  void onStop() {
    StopFermentation();
  }
};

volatile unsigned long timeDiff;
void IRAM_ATTR CupStatusChangedInt() {
  detachInterrupt(CUP_PRESENCE_IRQ);
  bool cupPresence = digitalRead(CUP_PRESENCE_IRQ);//need about 4-1 milli Sec to stable
  timeDiff = micros();
  Serial.printf("\nCup Presence Status Changed to %d ... %lu\n", cupPresence, micros() );
  timerAlarmEnable(cupPresenceTimer);  
}


void IRAM_ATTR onCupPresenceTimerTimer() {
  bool cupPresence = digitalRead(CUP_PRESENCE_IRQ);//need about 4-1 milli Sec to stable
  unsigned long diff = micros()-timeDiff;
  Serial.printf("Cup Presence Timer, Status - %d Took %lu\t%lu\n", cupPresence, micros(), diff);

  attachInterrupt(CUP_PRESENCE_IRQ, CupStatusChangedInt, CHANGE); 
}


void IRAM_ATTR detectsNFCCard() {
  Serial.println("IRQ - ISO14443A Card ...");
  detachInterrupt(PN532_IRQ); 
  cardReadWaiting = true;
}

void startListeningToNFC() {
  
  Serial.println("Waiting for an ISO14443A Card ...");
  nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
  attachInterrupt(PN532_IRQ, detectsNFCCard, FALLING); //Enable interrupt after starting NFC
}

void handleCardDetected() {
    uint8_t success = false;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
    uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

    // read the NFC tag's info
    success = nfc.readDetectedPassiveTargetID(uid, &uidLength);
    Serial.println(success ? "Read successful" : "Read failed (not a card?)");

    if (success) {
      // Display some basic information about the card
      Serial.println("Found an ISO14443A card");
      Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
      Serial.print("  UID Value: ");
      nfc.PrintHex(uid, uidLength);
      
      if (uidLength == 4)
      {
        // We probably have a Mifare Classic card ... 
        uint32_t cardid = uid[0];
        cardid <<= 8;
        cardid |= uid[1];
        cardid <<= 8;
        cardid |= uid[2];  
        cardid <<= 8;
        cardid |= uid[3]; 
        Serial.print("Seems to be a Mifare Classic card #");
        Serial.println(cardid);
      }
      Serial.println("");

      timeLastCardRead = millis();
    }

    // The reader will be enabled again after DELAY_BETWEEN_CARDS ms will pass.
    readerDisabled = true;
    cardReadWaiting = false;
}


void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(33, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);
  pinMode(PN532_IRQ, INPUT_PULLUP);
  pinMode(CUP_PRESENCE_IRQ, INPUT_PULLUP);


  Serial.begin(115200);
  Serial.println("\n\n --- Starting Dough Fermentation Service ---");

  //initiate value
  floorDist = 255;
  doughServcieStatus.setCupBaseDist(floorDist-15);

  // RTC 
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    Serial.flush();
    delay(1000);
    // abort();
  }
  if (rtc.lostPower()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  DateTime currTime = rtc.now();
  
  char strFormat[25];
  String cTimeStr = currTime.timestamp(DateTime::timestampOpt::TIMESTAMP_FULL);
  snprintf(strFormat, sizeof(strFormat), "%s", cTimeStr.c_str());
  Serial.printf(" -- %s -- \n\n", strFormat);
  
  //Start TOF Sensor
  Wire.begin();
  disSensor.getIdentification(&identification); // Retrieve manufacture info from device memory
  //printIdentification(&identification);      

  if (disSensor.VL6180xInit() != 0) {
    Serial.println("Failed to initialize. Freezing..."); // Initialize device and check for errors
    Serial.flush();
    delay(1000);
    abort();
  }
  disSensor.VL6180xDefautSettings(); // Load default settings to get started.

  //Start NFC
  nfc.begin();
    
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.println("\nDidn't find PN53x board !!!\n");
    Serial.flush();
    delay(1000);
    // abort();
  } else {
    // Got ok data, print it out!
    Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
    // Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
    // Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
    
    // configure board to read RFID tags
    nfc.SAMConfig();
  	startListeningToNFC();
}

  //Start Pixel light
  leds.initLed();
  
  //init BLE
  xBleDoughHeight.initBLE();
  xBleDoughHeight.regDoughServiceBLECallback(new DoughServiceBLECallback());

  //cup interupt
  int SamplingRate = 1000;          //Read 1000 values in one second.
  attachInterrupt(CUP_PRESENCE_IRQ, CupStatusChangedInt, CHANGE); 
  cupPresenceTimer = timerBegin(0, 80, true);               //Begin timer with 1 MHz frequency - 11 tick take 1/(80MHZ/80) = 1us
  timerAttachInterrupt(cupPresenceTimer, &onCupPresenceTimerTimer, true);   
  unsigned int timerFactor = 1000000/SamplingRate;          //Calculate the time interval between two readings, or more accurately, the number of cycles between two readings
  timerAlarmWrite(cupPresenceTimer, timerFactor, false);     //Initialize the timer (one time)

  //Start SPIFF
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    Serial.flush();
    delay(1000);
    abort();
  }

  //Read Service status 
  readStatus();

  delay(750);
}

void loop() {

  //check if there is NFC Card Detected.
  if (cardReadWaiting) {     
    handleCardDetected();
    delay(150);
    startListeningToNFC();
  }
  
  if (xBleDoughHeight.isDeviceConnected()) {
    
    unsigned long now = millis();
    if ((now - lastSentTime) > sendInterval) {

      lastSentTime = now;
      // Get Distance and report in mm
      uint8_t tmpDist = disSensor.getDistance();
      avgDistance.Insert(tmpDist);
      currDoughDist = (uint8_t)avgDistance.Avg();

#ifdef DEBUG_MAIN
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
#endif
      
      if ((doughServcieStatus.getDoughServcieStatusEnum() == DoughServcieStatusEnum::Fermenting) ||
        (doughServcieStatus.getDoughServcieStatusEnum() == DoughServcieStatusEnum::ReachedDesiredFerm)|| 
        (doughServcieStatus.getDoughServcieStatusEnum() == DoughServcieStatusEnum::OverFerm)) {

        //broadcast heightv & Percentage
        int initDist = doughServcieStatus.getDoughInitDist();
        int baseDist = doughServcieStatus.getCupBaseDist();
        
        float fermPercent = (initDist - currDoughDist)/(float)(baseDist - initDist);
        // Serial.printf("Dough Fermentation BaseDist:%d InitDist:%d currDist:%d = %f2%%\n", baseDist, initDist, currDoughDist, fermPercent*100);
        Serial.printf("Dough Fermentation Base Height:%d Init Dough Height:%d Current Ferm Height:%d = %2f%%\n", 
                      floorDist - baseDist, baseDist - initDist, initDist - currDoughDist, fermPercent*100);

        doughServcieStatus.setDoughHeight(currDoughDist);
        xBleDoughHeight.sendHeightData(currDoughDist);
        doughServcieStatus.setFermPercentage(fermPercent);
        xBleDoughHeight.sendDoughFermPercentData(fermPercent);


        //check if reached desired fermentation status
        float desiredPercentage = doughServcieStatus.getDesiredFermPercentage();
        if (fermPercent > (desiredPercentage + doughServcieStatus.getOverFermPercentage())) {
          OverFermentation();
        } else if (fermPercent > desiredPercentage) {
          ReachedDesiredFermentation();
        } else {
          ContFermenting();
        }
      } else {
        Serial.printf("Distance measured = %2d (%d) mm.\t Height = %2d\n", currDoughDist, tmpDist, floorDist - currDoughDist);
        // avgDistance.printDebug();
      }
    }
  }
  delay(500);
}




