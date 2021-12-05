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

class BLEDoughHeight 
{
    bool deviceConnected = false;
    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pRxCharacteristic;
    BLECharacteristic* pTxCharacteristic;
    BLEAdvertising *pAdvertising;
    
    friend class TheServerCallBacks;

public:
    void initBLE();

    void setDeviceConnected(bool conn) {deviceConnected = conn;}
    bool isDeviceConnected();
    void sendData(int doughHeight);

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


class theCharacteristicCallbacks: public BLECharacteristicCallbacks {
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
#endif