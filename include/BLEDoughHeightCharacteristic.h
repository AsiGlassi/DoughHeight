#ifndef __ble_dough_height_Characteristic_h__
#define __ble_dough_height_Characteristic_h__

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>


class BLEDoughHeight;// Forward declaration

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