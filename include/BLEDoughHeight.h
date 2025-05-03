#ifndef __ble_dough_height_h__
#define __ble_dough_height_h__

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "DoughServcieStatus.h"


#define DEBUG_BLE false

//BLE Dough Height service call back type
typedef void (*BleDoughServiceCallbackFunction)(void);

#define DOUGH_DEVICE_NAME "Asi Dough Height"
#define DOUGH_DOUGH_FERMENTATION_SERVICE_UUID   "3ee2ffbe-e236-41f2-9c40-d44563ddc614"

#define CHARACTERISTIC_HEIGHT_UUID  "7daf9c2b-715c-4a1c-b889-1ccd50817186"
#define CHARACTERISTIC_FERMENTATION_UUID  "8d0ca8ce-5c66-4e31-ba1a-48868601ec25"
#define CHARACTERISTIC_STATUS_UUID  "a1990b88-249f-45b2-a0b2-ba0f1f90ca0a"
#define CHARACTERISTIC_SESSION_STATUS_UUID  "90e0676a-eae2-4878-9a6f-61090aac8837"
#define CHARACTERISTIC_CONFIGURATION_UUID  "b164f891-400a-439f-95d2-659973c18df4"

#define CHARACTERISTIC_COMMAND_UUID  "fc70539e-2e17-4cf8-b7e2-4375fc7ded5a"


class DoughServiceBLECallbacks {
public:
	virtual ~DoughServiceBLECallbacks() {};
    virtual void onConnect();
	virtual void onDisConnect();
	virtual void onStart();
	virtual void onStop();
    virtual void onConfigurationChanged();
};


class BLEDoughHeight 
{
    BLEServer* pServer;
    BLEService* pService;
    
    BLECharacteristic* pHeightCharacteristic;
    BLECharacteristic* pFermPercentageCharacteristic;
    BLECharacteristic* pCommandCharacteristic;
    BLECharacteristic* pSessionCharacteristic;
    BLECharacteristic* pConfigurationCharacteristic;
    BLECharacteristic* pStatusCharacteristic;

    BLEAdvertising *pAdvertising;

    DoughServiceBLECallbacks* bleDoughHeightCallback = NULL;

    bool deviceConnected = false;
    DoughServcieStatus* bleDoughServcieStatus = NULL;
   

public:

    BLEDoughHeight(DoughServcieStatus* dServcieStatus) {bleDoughServcieStatus = dServcieStatus;}
    void initBLE();

    void setDeviceConnected(bool conn);
    bool isClientDeviceConnected();

    DoughServcieStatus getBleDoughServcieStatus() {return *bleDoughServcieStatus;}

    float getDoughFermentationPercent() {return bleDoughServcieStatus->getFermPercentage();}
    void sendHeightData(uint8_t doughHeight);
    void sendDoughFermPercentData(float doughFermenPercent);
    void sendStatustData(DoughServcieStatusEnum status);
    void sendStatustData(DoughServcieStatusEnum status, std::string);

    void StartFermentation();
    void StopFermentation();
    void GeneralAction();
    void UpdateConfigurationAction(); 
    void regDoughServiceBLECallback(DoughServiceBLECallbacks* pCallback) {bleDoughHeightCallback = pCallback;}
};



class DeviceSecurityCallbacks : public BLESecurityCallbacks {

    //BLEDoughHeight* pBleDoughHeight;

public:
    // DeviceSecurityCallbacks(BLEDoughHeight* pinDoughHeight) {
    //     pBleDoughHeight = pinDoughHeight;
    // }
    // DeviceSecurityCallbacks(){Serial.println(x);}

    virtual bool onConfirmPIN(uint32_t pin) {
        Serial.println("Confirming PIN: " + String(pin));
        return true;
    };

    virtual uint32_t onPassKeyRequest() {
        uint32_t passKey = 123456;
        Serial.printf("Passkey request, providing static passkey %d...", passKey);
        return passKey;  // Static passkey
    }

    virtual void onPassKeyNotify(uint32_t passkey) {
        Serial.println("Passkey: " + String(passkey));
    };

    virtual bool onSecurityRequest() {
        return true;
    };

    void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) {
        if (cmpl.success) {
            Serial.println("Bonding successful!");
        } else {
            Serial.println("Bonding failed!");
        }
    };
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

#ifdef DEBUG_BLE		
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
#endif
	}

    void onDisconnect (BLEServer* pServer) {
        pBleDoughHeight->setDeviceConnected(false);
        Serial.println("Device Disconnected");
        pServer->startAdvertising(); 
    };
};


class heightCharacteristicCB: public BLECharacteristicCallbacks {

    BLEDoughHeight* pBleDoughHeight;

public:
    heightCharacteristicCB(BLEDoughHeight* pinDoughHeight) {
        pBleDoughHeight = pinDoughHeight;
    }

    void onWrite(BLECharacteristic *pCharacteristic);
    void onRead(BLECharacteristic *pCharacteristic);
};


class FermPercentageCharacteristicCB: public BLECharacteristicCallbacks {
    
    BLEDoughHeight* pBleDoughHeight;

public:
    FermPercentageCharacteristicCB(BLEDoughHeight* pinDoughHeight) {
        pBleDoughHeight = pinDoughHeight;
    }

    void onWrite(BLECharacteristic *pCharacteristic);
    void onRead(BLECharacteristic *pCharacteristic);
};


class CommandCharacteristicCB: public BLECharacteristicCallbacks {
    
    BLEDoughHeight* pBleDoughHeight;

public:
    CommandCharacteristicCB(BLEDoughHeight* pinDoughHeight) {
        pBleDoughHeight = pinDoughHeight;
    }

    void onWrite(BLECharacteristic *pCharacteristic);
    void onRead(BLECharacteristic *pCharacteristic);
};


class SessionStatusCharacteristicCB: public BLECharacteristicCallbacks {

    BLEDoughHeight* pBleDoughHeight;

public:
    SessionStatusCharacteristicCB(BLEDoughHeight* pinDoughHeight) {
        pBleDoughHeight = pinDoughHeight;
    }

    void onWrite(BLECharacteristic *pCharacteristic);
    void onRead(BLECharacteristic *pCharacteristic);
};


class ConfigurationCharacteristicCB: public BLECharacteristicCallbacks {

    BLEDoughHeight* pBleDoughHeight;

public:
    ConfigurationCharacteristicCB(BLEDoughHeight* pinDoughHeight) {
        pBleDoughHeight = pinDoughHeight;
    }

    void onWrite(BLECharacteristic *pCharacteristic);
    void onRead(BLECharacteristic *pCharacteristic);
};


class StatusCharacteristicCB: public BLECharacteristicCallbacks {

    BLEDoughHeight* pBleDoughHeight;

public:
    StatusCharacteristicCB(BLEDoughHeight* pinDoughHeight) {
        pBleDoughHeight = pinDoughHeight;
    }

    void onWrite(BLECharacteristic *pCharacteristic);
    void onRead(BLECharacteristic *pCharacteristic);
};


class DesiredFermPercentageCharacteristicCB: public BLECharacteristicCallbacks {

    BLEDoughHeight* pBleDoughHeight;

public:
    DesiredFermPercentageCharacteristicCB(BLEDoughHeight* pinDoughHeight) {
        pBleDoughHeight = pinDoughHeight;
    }

    void onWrite(BLECharacteristic *pCharacteristic);
    void onRead(BLECharacteristic *pCharacteristic);
};

#endif