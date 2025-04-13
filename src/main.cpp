#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include <esp_log.h>

#include <Arduino.h>
#include <RTClib.h>
// #include <SPI.h>
#include <Wire.h>

#include <Adafruit_VL53L0X.h>

#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <Adafruit_PN532.h>
#include <map>
 
#include "CircularAvg.h"
#include "CircularAvg.cpp"
#include "Circular.cpp"

#include "NfcId.h"
#include "DoughCup.h"
#include "LedDough.h"
#include "BLEDoughHeight.h"
#include "DoughServcieStatus.h"


//Debug
// #define DEBUG_MAIN true

#define BUZZ_PIN 14

//RTC 
RTC_DS3231 rtc;

//TOF - Distance measures
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

//I2C 
//0x24 - NFC - pn352 (Need Batt)
//0x29 - Dist - VL53L0X
//0x57 -  
//0x68 - Time - DS3231

//Dough config
uint8_t currDoughDist = 0;
uint8_t defaultDist = 0;
CircularAvg<int> avgDistance(4, 0);
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

// Cup List
const char *cupsListFileName = "/Cups.json";
std::map<std::string, DoughCup> cupsMap;

//pn352 0x24
#define PN532_IRQ   (15)
#define PN532_RESET (4)  // Not connected by default on the NFC Shield
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

//general 
const int DELAY_BETWEEN_CARDS = 1000;//500 caused issue 
long timeLastCardRead = 0;
volatile bool connected = false;
boolean readerDisabled = true;
volatile bool cardReadWaiting = false;

//cup presence 
#define CUP_PRESENCE_IRQ (27) 
hw_timer_t * cupPresenceTimer = NULL; 
volatile bool cupPresence = false;
bool cupPresenceLast = cupPresence;
bool encounteredCupError = false;

//interval
unsigned long sendIntervalLow = 2250; //5000
unsigned long sendInterval = sendIntervalLow;
unsigned long sendIntervalHigh = sendIntervalLow/4;
unsigned long lastSentTime = 0;
unsigned int fermentationAgingSpan = (4*60);//in minutes 


//Parse Date standard ISO 8601-like format
DateTime ParseDateTime(const char* timestampStr) {
  int year, month, day, hour, minute, second;

  // Extract date and time components from the string
  if (sscanf(timestampStr, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
      return DateTime(year, month, day, hour, minute, second);
  } else {
      Serial.println("Failed to parse timestamp!");
      return DateTime(); // Return an invalid DateTime
  }
}

bool isReadingStable() {
  float deviation = avgDistance.Stdev();
  // Serial.printf("Reading deviation: %2f%\n", deviation);
  return (deviation <= 1.25);
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
  char strFormat[] = "YYYY-MM-DD hh:mm:ss";
  DateTime startTime = doughServcieStatus.getFermentationStart();
  doc["FermentationStart"] = startTime.toString(strFormat);
  doc["DoughInitDist"] = doughServcieStatus.getDoughInitDist();
  doc["CupBaseDis"] = doughServcieStatus.getCupBaseDist();

  // Serialize JSON to file
  if (serializeJson(doc, file) == 0) {
    Serial.println(F("Failed to write last Status file."));
  } else {
    Serial.println(F("Saved last Status file."));

  }

  // Close the file
  file.close();
}

File openFile(const char* filePath) {
  //read
  File statusFile;
  if (SPIFFS.exists(filePath)) {
    //File Exist
    statusFile = SPIFFS.open(filePath);
 
    if(!statusFile) {
        Serial.println("Failed to open Last Settings file.");
    }
  }
    return statusFile;
}

//Read service status file and update the Class
void readStatus() {

  //read
  File statusFile = openFile(lastSettingsFileName);
  if (statusFile != 0) {
 
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, statusFile);

    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("Deserialize Last Settings Json failed: "));
      Serial.println(error.f_str());
      return;
    }
      
    DoughServcieStatusEnum statusFromFile = doc["DoughServcieStatus"].as<DoughServcieStatusEnum>();
    // Serial.printf("Status read from the file: %d\n", statusFromFile);
    if (statusFromFile == DoughServcieStatusEnum::Fermenting) { 

      //check fermentation start time
      const char* fileDateStr = doc["FermentationStart"].as<const char*>();
      DateTime fermStarted = ParseDateTime(fileDateStr);
      TimeSpan startedBefore = (rtc.now() - fermStarted);
      unsigned int diffInMin = startedBefore.totalseconds()/60;
      Serial.printf("Last operation stopped during fermentation at %s - %d min ago.\n", 
                    fileDateStr, diffInMin);

      if (diffInMin < fermentationAgingSpan) {
        //Continue fermentation
        Serial.printf("Continue fermentation.\n");
        doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::Fermenting, "Start Fermentation");
        doughServcieStatus.setFermentationStart(fermStarted);
        doughServcieStatus.setDoughInitDist(doc["DoughInitDist"]);
        doughServcieStatus.setCupBaseDist(doc["CupBaseDis"]);
      } else {
        Serial.printf("Ignoring Saved status - Past a long time since last run.\n");    
      }
    }
    statusFile.close();
  } else {
    Serial.println(F("Failed to open Last Settings for reading."));
  }
}

void saveCups() {

  if (SPIFFS.exists(cupsListFileName)) {
    //File Exist
    SPIFFS.remove(cupsListFileName);
  }

  // Open file for writing
  File file = SPIFFS.open(cupsListFileName, FILE_WRITE);
  if (!file) {
    Serial.println(F("Error: Failed to create Cups List file."));
    return;
  }

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use https://arduinojson.org/assistant to compute the capacity.
  const int capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + cupsMap.size()*94;
 
   // StaticJsonDocument<capacity> doc;
  DynamicJsonDocument doc(capacity);

  Serial.print(PROGMEM "Cup size save: "); Serial.println(cupsMap.size());
  // Set the values in the document
  JsonObject root = doc.to<JsonObject>();
  JsonArray arr = root.createNestedArray("CupList");
  
  int i=0;
  for (std::pair<std::string, DoughCup> element : cupsMap) {
    DoughCup item = element.second;
    Serial.println(item.str().c_str());
    JsonObject arrObj = arr.createNestedObject();
    arrObj["CupId"] = item.getCupId();
    arrObj["CupBaseDis"] = item.getCupHeight();
    i++;
  }  
  size_t actualCap = doc.capacity();// overflowed();
  if (actualCap < capacity) {
    //Json allocation Error
    Serial.print("Error: Calc Capacity: "); Serial.print(capacity);
    Serial.print(" Actual Cappacity "); Serial.println(actualCap);
  }

  // Serialize JSON to file
  if (serializeJsonPretty(doc, file) == 0) {
    Serial.println(F("Failed to write Last Settings file."));
  }
  // char buffer[1024];
  // serializeJsonPretty(doc, buffer);
  // Serial.println(buffer);

  // Close the file
  file.close();
}

void readCups() {

  //read
  File cupFile = openFile(cupsListFileName);
  if (cupFile != 0) {
 
    Serial.println("Read Cups File"); 
    // while(cupFile.available()) {
    //   Serial.write(cupFile.read());
    // }
    // Serial.println("\n");
    // int fileSize = cupFile.size();
    // Serial.print("\nFile Size: ");Serial.println(fileSize);

    //Parse Json
    StaticJsonDocument<1024> parsedDoc;
    DeserializationError error = deserializeJson(parsedDoc, cupFile);
    // Test if parsing succeeds.
    if (error) {
      Serial.print(F("Deserialize Cups Json failed: "));
      Serial.println(error.f_str());
      return;
    }
    
    JsonArray cupArr = parsedDoc["CupList"].as<JsonArray>();
    // int arraySize = cupArr.size();   
    // Serial.print("\Cup size read: "); Serial.println(arraySize);
 
    for (JsonObject cup : cupArr) {
      const char* cupId = cup["CupId"].as<const char*>();
      // Serial.print("CupId: "); Serial.print(cupId);
      byte cupHeight = cup["CupBaseDis"].as<byte>();
      // Serial.print("\tCupHeight: "); Serial.println(cupHeight);
      DoughCup item(cupId, cupHeight);
      cupsMap.insert(std::make_pair(item.getCupId(), item));
    }
 
    cupFile.close();
  } else {
    Serial.println("Cups file not found.");
  }
}

void listDir() {
 
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
 
  while(file) {
 
      Serial.print("FILE: ");
      Serial.println(file.name());
 
      file = root.openNextFile();
  } 
}


void ErrorHandeling(std::string errorMsg) {
    Serial.printf("Dough Service Error %s.\n", errorMsg.c_str());

    //set status
    doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::Error, errorMsg);

    //Set Light status
    leds.Error();

    //update BLE device status changed
    xBleDoughHeight.sendStatustData(doughServcieStatus.getDoughServcieStatusEnum(), errorMsg);
}

void ClientIdle() {

  //set status
  doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::idle);

  //Set Light status
  if (xBleDoughHeight.isClientDeviceConnected()) {
    leds.BleConnected();
  } else {
    leds.idle();
  }

  //update BLE device status changed
  xBleDoughHeight.sendStatustData(doughServcieStatus.getDoughServcieStatusEnum());
}

void ClientConnected() {

  //Set Light status
  DoughServcieStatusEnum currStatus = doughServcieStatus.getDoughServcieStatusEnum();
  if (currStatus == DoughServcieStatusEnum::idle) {
    leds.BleConnected();
  }
}

void ClientDisConnected() {

  //Set Light status
  DoughServcieStatusEnum currStatus = doughServcieStatus.getDoughServcieStatusEnum();
  if (currStatus == DoughServcieStatusEnum::idle) {
    leds.idle();
  }
}

void StartFermentation() {
  if (!cupPresence) {
    ErrorHandeling("Cant start process, Cup Not Pressent");
  } else if (currDoughDist == 0) {
    ErrorHandeling("Cant start process, Distance = 0.");
  } else if (abs(currDoughDist - doughServcieStatus.getCupBaseDist()) < minDoughHeight) {
    ErrorHandeling("Cant start process, Dough level is too low.");
  } else {
    //check that reading is stable
    bool stable = false;
    while (!stable) {
      stable = isReadingStable();
      delay(sendIntervalHigh);
    }

    DateTime currTime = rtc.now();
    char strFormat[] = "YYYY-MM-DD hh:mm:ss";
    Serial.printf("Start Fermentation Process: %s \tInitialize Dough Distance: %d\n", 
      currTime.toString(strFormat), currDoughDist);
    
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
  Serial.println(F("Continue Fermentation Process."));

  //set status
  doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::Fermenting);

  //Set Light status
  leds.Fermenting();

  //update BLE device status changed
  xBleDoughHeight.sendStatustData(doughServcieStatus.getDoughServcieStatusEnum());
}

void StopFermentation() {
  Serial.println(F("Stop Fermentation Process."));

  //set status
  doughServcieStatus.setDoughServcieStatusEnum(DoughServcieStatusEnum::idle, "Stop Fermentation Process");

  //Set Light status
  if (xBleDoughHeight.isClientDeviceConnected()) {
    leds.BleConnected();
  } else {
    leds.idle();
  }

  //update BLE device status changed
  xBleDoughHeight.sendStatustData(doughServcieStatus.getDoughServcieStatusEnum());
}

void ReachedDesiredFermentation() {
    Serial.println(F("Reached Desired Fermentation."));

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

  void onConnect() {
    ClientConnected();
  }

  void onDisConnect() {
    ClientDisConnected();
  }
};


NfcId handleCardDetected() {
  
  NfcId retUId;
  bool success;

  Serial.println("Handelling");

  // Buffer to store the UID
  uint8_t uid[7] = { 0, 0, 0, 0, 0, 0, 0 };
  // UID size (4 or 7 bytes depending on card type)
  uint8_t uidLength;

  // read the NFC tag's info
  success = nfc.readDetectedPassiveTargetID(uid, &uidLength);
  if (success) {

    retUId.setIds(uid);
    // If the card is detected, print the UID
    Serial.print("Size of UID: "); Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("UID: ");
    Serial.print(retUId.str().c_str());
    Serial.println("\n\n");
  } else {
      Serial.println("Read failed (not a card?)");
  }

  // The reader will be enabled again after DELAY_BETWEEN_CARDS ms will pass.
  readerDisabled = true;
  cardReadWaiting = false;
  timeLastCardRead = millis();
  return retUId;
}

void IRAM_ATTR detectsNFCCard() {
  Serial.printf("\nNFC Card detected Interupt ... %lu\n", micros() );
  detachInterrupt(PN532_IRQ); 
  cardReadWaiting = true;
}

void startListeningToNFC() {
  Serial.println("StartListeningToNFC - Waiting for card (ISO14443A Mifare)...");

  //Enable interrupt after starting NFC
  nfc.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A);
  attachInterrupt(PN532_IRQ, detectsNFCCard, FALLING); 
}

bool nfcConnect() {
  
  nfc.begin();

  // Connected, show version
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.println("\nDidn't find PN53x board !!!\n");
    return false;
  }

  //port
  Serial.print("Found chip PN5"); Serial.print((versiondata >> 24) & 0xFF, HEX);
  Serial.print(", Firmware version: "); Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

  // configure board to read RFID tags
  nfc.SAMConfig();

  return true;
}

void IRAM_ATTR CupStatusChangedInt() {
  // Object presence changed, Start timer and check
  // Serial.printf("Cup INT %d\n",!digitalRead(CUP_PRESENCE_IRQ) );

  //Enable timer, to take measure after stabilizing.
  timerAlarmDisable(cupPresenceTimer); 
  timerRestart(cupPresenceTimer);
  timerWrite(cupPresenceTimer, 0);
  timerAlarmEnable(cupPresenceTimer);  
}

void IRAM_ATTR onCupPresenceTimerTimer() {
  //Disable timer
  timerAlarmDisable(cupPresenceTimer); 

  //Check if cup presence
  cupPresence = !digitalRead(CUP_PRESENCE_IRQ);//need about 4-1 milli Sec to stable
  // Serial.printf("Cup Presence Timer, Status - %d\n", cupPresence); 
}


void Setup_VL53L0X() {
  Serial.println("Adafruit VL53L0X Init\n");
  if (!lox.begin()) {
    ErrorHandeling("Failed to boot VL53L0X\n");
    while(3000);
    //abort();
  }
  lox.configSensor(Adafruit_VL53L0X::VL53L0X_SENSE_HIGH_ACCURACY);
  // lox.startRangeContinuous(125);
}

int VL53L0X_Dist() {
  VL53L0X_RangingMeasurementData_t measure;
    
  // Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    // Serial.print("Distance (mm): "); 
    // Serial.println(measure.RangeMilliMeter);
  } else {
    Serial.println("Distance out of range");
  }
  return measure.RangeMilliMeter;
}


void setup() {

  esp_log_level_set("*", ESP_LOG_WARN);        // set all components to ERROR level
  esp_log_level_set("BLEDevice", ESP_LOG_ERROR);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);
  pinMode(PN532_IRQ, INPUT_PULLUP);
  pinMode(CUP_PRESENCE_IRQ, INPUT);

  Serial.begin(115200);
  Serial.println("\n\n --- Starting Dough Fermentation Service ---");

  //initiate value
  floorDist = 255;
  doughServcieStatus.setCupBaseDist(floorDist-15);

  //Start Pixel light
  leds.initLed();
  
    //init BLE
  xBleDoughHeight.initBLE();
  xBleDoughHeight.regDoughServiceBLECallback(new DoughServiceBLECallback());

  // RTC 
  if (!rtc.begin()) {
    ErrorHandeling("Couldn't find RTC!");
    Serial.flush();
    delay(3000);
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
  Setup_VL53L0X();
  uint8_t tmpDist = VL53L0X_Dist();
  avgDistance.SetAll(tmpDist);

  //Start NFC
  if (!nfcConnect()) {
    ErrorHandeling("Failed to initialize NFC tag reader."); 
    delay(3000);
    // abort();
  }  

  //cup interupt
  cupPresence = !digitalRead(CUP_PRESENCE_IRQ);
  cupPresenceLast = cupPresence;
  attachInterrupt(digitalPinToInterrupt(CUP_PRESENCE_IRQ), CupStatusChangedInt, CHANGE); 
  //Begin timer with 1 MHz frequency - 11 tick take 1/(80MHZ/80) = 1us
  cupPresenceTimer = timerBegin(0, (getApbFrequency()/1000000), true);
  timerAttachInterrupt(cupPresenceTimer, &onCupPresenceTimerTimer, true);   
  //Initialize the timer (one time).
  //Todo - One time event doesnt work well for me, i set it to continues and i stop the timer at the end of timer event.
  timerAlarmWrite(cupPresenceTimer, 1000000*0.25, true);//1000000
  // Serial.print(" >> CPU Freq:   "); Serial.println(getCpuFrequencyMhz());
  // Serial.print(" >> XTL Freq:   "); Serial.println(getXtalFrequencyMhz());
  // Serial.print(" >> APB Freq:   "); Serial.println(getApbFrequency());

  //Start SPIFF
  Serial.println("\nStarting SPIFFS");

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    Serial.flush();
    delay(3000);
    abort();
  }

  //Read Service status 
  readStatus(); 

  //read cup files
  readCups();
  Serial.printf("Cup size: %d\n", cupsMap.size());
  // for (std::pair<std::string, DoughCup> element : cupsMap) {
  //     Serial.print(element.first.c_str());
  //     Serial.print(": ");
  //     Serial.print(element.second.c_str());
  //     Serial.println();
  // }
 
  // startListeningToNFC();// For  testing

  delay(750);
}


void loop() {

  //check if there is NFC Card Detected.
  bool pastDelayTime = true;
  if (cardReadWaiting) { 
    //NFC found, read card
    NfcId nfcId = handleCardDetected();
    if (nfcId.isEmpty()) {
      //Error
    }

    // check against current status, get cup details
    DoughCup cupDetected = cupsMap[nfcId.str()];
    int newCupH = cupDetected.getCupHeight();
    int currCupH = doughServcieStatus.getCupBaseDist();
    if (newCupH != currCupH) {
      //Cup Changed...

    }

  } else if (readerDisabled) {
    if (millis() - timeLastCardRead > DELAY_BETWEEN_CARDS) {
      readerDisabled = false;
      pastDelayTime = true;
    } else {
      pastDelayTime = false;
    }
  } 
  if (pastDelayTime) {
    if(cupPresence != cupPresenceLast)	{
      Serial.printf("Cup Presence Status changed - %d \n", cupPresence);
      cupPresenceLast = cupPresence;
      if (cupPresence) {
        //cup presence - try to read NFC
        startListeningToNFC();// doesnt work within int cup
      }
    }
  }
  
  if (xBleDoughHeight.isClientDeviceConnected()) {
    
    //check if stable, if not increase freq
    bool stable = isReadingStable();
    sendInterval = stable ? sendIntervalLow : sendIntervalHigh;
    
    unsigned long now = millis();
    if ((now - lastSentTime) > sendInterval) {

      lastSentTime = now;

      DoughServcieStatusEnum statusBeforeCupError;
	    if (!cupPresence) {
        encounteredCupError = true;
        statusBeforeCupError = doughServcieStatus.getDoughServcieStatusEnum();
        Serial.printf("Cup Error, Save current status %d\n", statusBeforeCupError);
	      ErrorHandeling("Cup Not Pressent");
	    } else {
        if (encounteredCupError) {
          encounteredCupError = false;
          //cup restored, restore latest status
          Serial.printf("Recovered from Cup Error restore status %d\n", statusBeforeCupError);
          switch (statusBeforeCupError) {
          case DoughServcieStatusEnum::idle:
            ClientIdle();
            break;
          case DoughServcieStatusEnum::Fermenting: 
            ContFermenting(); 
            break;
          case DoughServcieStatusEnum::OverFerm:
            OverFermentation();
            break;
          case DoughServcieStatusEnum::ReachedDesiredFerm: 
            ReachedDesiredFermentation();
            break;
          };
        }
      

        // Get Distance and report in mm
        uint8_t tmpDist = VL53L0X_Dist();
        avgDistance.Insert(tmpDist);
        currDoughDist = (uint8_t)avgDistance.Avg();
             
        if ((doughServcieStatus.getDoughServcieStatusEnum() == DoughServcieStatusEnum::Fermenting) ||
          (doughServcieStatus.getDoughServcieStatusEnum() == DoughServcieStatusEnum::ReachedDesiredFerm)|| 
          (doughServcieStatus.getDoughServcieStatusEnum() == DoughServcieStatusEnum::OverFerm)) {

          //broadcast height & Percentage
          int initDist = doughServcieStatus.getDoughInitDist();
          int baseDist = doughServcieStatus.getCupBaseDist();
          
          float fermPercent = (initDist - currDoughDist)/(float)(baseDist - initDist);
          // Serial.printf("Dough Fermentation BaseDist:%d InitDist:%d currDist:%d = %f2%%\n", baseDist, initDist, currDoughDist, fermPercent*100);
          Serial.printf("Dough Fermentation Base Height:%d \tInit Dough Height:%d \tCurrent Ferm Height:%d = %2f%%\n", 
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
          Serial.printf("Distance measured = %2d (%d) mm.\t Height = %2d (%d)\n", currDoughDist, tmpDist, floorDist - currDoughDist, floorDist - tmpDist);
          xBleDoughHeight.sendHeightData(currDoughDist);

          // avgDistance.printDebug();
        }
      }
    }
  }
  // delay(500); //causing issues with BLE & NFC
}

