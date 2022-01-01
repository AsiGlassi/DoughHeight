#ifndef __ble_dough_height_h__
#define __ble_dough_height_h__

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// #define UART_SERVICE_UUID      "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
// #define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
// #define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"


#define DOUGH_HEIGHT_SERVICE_UUID   "3ee2ffbe-e236-41f2-9c40-d44563ddc614"
#define CHARACTERISTIC_HEIGHT_UUID  "7daf9c2b-715c-4a1c-b889-1ccd50817186"
#define CHARACTERISTIC_START_UUID  "fc70539e-2e17-4cf8-b7e2-4375fc7ded5a"
#define CHARACTERISTIC_STATUS_UUID  "a1990b88-249f-45b2-a0b2-ba0f1f90ca0a"


enum DoughServcieStatusEnum {
  idle,
  Fermenting, 
  ReachedDesiredFerm, 
  OverFerm
};


class BLEDoughHeight 
{
    bool deviceConnected = false;
    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pHeightCharacteristic;
    BLECharacteristic* pStartCharacteristic;
    BLECharacteristic* pStatusCharacteristic;
    BLEAdvertising *pAdvertising;

 public:
   DoughServcieStatusEnum DoughServcieStatus = DoughServcieStatusEnum::idle;

 private:   
    friend class TheServerCallBacks;

public:
    void initBLE();

    void setDeviceConnected(bool conn) {deviceConnected = conn;}
    bool isDeviceConnected();
    void sendHeightData(int doughHeight);
    void sendStatustData(DoughServcieStatusEnum status);

    void StartFermentation();
    void StopFermentation();

};


class TheServerCallBacks: public BLEServerCallbacks { 

    BLEDoughHeight* pBleDoughHeight;

public:
    TheServerCallBacks(BLEDoughHeight* pinDoughHeight) {
        pBleDoughHeight = pinDoughHeight;
    }


    void onConnect(BLEServer* pServer, esp_ble_gatts_cb_param_t *param) {

        Serial.println("Device Connected"); 
        pBleDoughHeight->setDeviceConnected(true);
		
        char remoteAddress[18];

		sprintf(
			remoteAddress,
			"%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
			param->connect.remote_bda[0],
			param->connect.remote_bda[1],
			param->connect.remote_bda[2],
			param->connect.remote_bda[3],
			param->connect.remote_bda[4],
			param->connect.remote_bda[5]
		);

		Serial.printf("myServerCallback onConnect, MAC: %s\n", remoteAddress);
		ESP_LOGI(LOG_TAG, "myServerCallback onConnect, MAC: %s", remoteAddress);
	}


    void onDisconnect (BLEServer* pServer) {
        pBleDoughHeight->setDeviceConnected(false);
        Serial.println("Device Disconnected");
        pServer->startAdvertising(); 
    };
};


class heightCharacteristicCallbacks: public BLECharacteristicCallbacks {
    
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.printf("BLE Received Value: %s\n", rxValue.c_str());

      }
    }

    void onRead(BLECharacteristic *pCharacteristic) {
        std::string txValue = std::string("N/A");
        pCharacteristic->setValue(txValue.c_str());
    }
};


class startCharacteristicCallbacks: public BLECharacteristicCallbacks {
    
    BLEDoughHeight* pBleDoughHeight;

public:
    startCharacteristicCallbacks(BLEDoughHeight* pinDoughHeight) {
        pBleDoughHeight = pinDoughHeight;
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        if (rxValue.length() == 1) {
            if (rxValue[0] == 0x0) {
                Serial.println("BLE StartStop Charachtiristic received Stop Service Command.");
                pBleDoughHeight->StopFermentation();
            } else if (rxValue[0] == 0x1) {
                Serial.println("BLE StartStop Charachtiristic received Start Service Command.");
                pBleDoughHeight->StartFermentation();
            } else {
                Serial.printf("BLE StartStop Charachtiristic Received Value: %s\n", rxValue.c_str());
            }
        }
      }
    }

    void onRead(BLECharacteristic *pCharacteristic) {
        Serial.printf("BLE StartStop Charachtiristic invalid read request.\n");
    }
};


class statusCharacteristicCallbacks: public BLECharacteristicCallbacks {

    BLEDoughHeight* pBleDoughHeight;

public:
    statusCharacteristicCallbacks(BLEDoughHeight* pinDoughHeight) {
        pBleDoughHeight = pinDoughHeight;
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.printf("BLE Status Charachtiristic Received INVALID Value: %s\n", rxValue.c_str());
      }
    }

    void onRead(BLECharacteristic *pCharacteristic) {
        
        Serial.printf("BLE Status Charachtiristic read status: '%d'.\n", pBleDoughHeight->DoughServcieStatus);

        char statusCTemp[6];
        sprintf(statusCTemp, "%d", pBleDoughHeight->DoughServcieStatus);
        pCharacteristic->setValue(statusCTemp);
    }
};

#endif