#include "BLEDoughHeight.h"

void BLEDoughHeight::initBLE() {
    Serial.println("BLE Init");
    BLEDevice::init("Asi Dough Height");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new TheServerCallBacks(this));
    pService = pServer->createService(DOUGH_HEIGHT_SERVICE_UUID);


    //Height Measure
    pHeightCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_HEIGHT_UUID,
										BLECharacteristic::PROPERTY_NOTIFY |
                                        BLECharacteristic::PROPERTY_READ
									);
    pHeightCharacteristic->setCallbacks(new heightCharacteristicCallbacks());

    BLEDescriptor doughHeightDescriptor(BLEUUID((uint16_t)0x2902));
    doughHeightDescriptor.setValue("Dough Height");
    pHeightCharacteristic->addDescriptor(&doughHeightDescriptor);

    BLEDescriptor heightRWDescriptor(BLEUUID((uint16_t)0x2901));//read write
    heightRWDescriptor.setValue("Dough Height in mm");
    pHeightCharacteristic->addDescriptor(&heightRWDescriptor);

    pHeightCharacteristic->setValue("N/A");


    //Start/Stop
    pStartCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_START_UUID,
                                        BLECharacteristic::PROPERTY_WRITE
									);
    pStartCharacteristic->setCallbacks(new startCharacteristicCallbacks(this));

    BLEDescriptor startDescriptor(BLEUUID((uint16_t)0x2901));
    startDescriptor.setValue("Start/Stop Service");
    pStartCharacteristic->addDescriptor(&startDescriptor);

    pStartCharacteristic->setValue("N/A");


    //Satus
    pStatusCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_STATUS_UUID,
										BLECharacteristic::PROPERTY_NOTIFY |
                                        BLECharacteristic::PROPERTY_READ
									);
    pStatusCharacteristic->setCallbacks(new statusCharacteristicCallbacks(this));

    BLEDescriptor statusDescriptor(BLEUUID((uint16_t)0x2902));
    statusDescriptor.setValue("Service Status");
    pStatusCharacteristic->addDescriptor(&statusDescriptor);

    BLEDescriptor statusRWDescriptor(BLEUUID((uint16_t)0x2901));//read write
    statusDescriptor.setValue("Service Status");
    pStatusCharacteristic->addDescriptor(&statusDescriptor);

    pStatusCharacteristic->setValue("N/A");



    pService->start();

    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE Dough Height Server Started");                              
}

bool BLEDoughHeight::isDeviceConnected() {
       return deviceConnected;
}

void BLEDoughHeight::sendHeightData(int doughHeight) {
    Serial.printf("BLE Push dought height '%d'\n", doughHeight);
    static char doughHeightCTemp[6];
    dtostrf(doughHeight, 6, 0, doughHeightCTemp);
    pHeightCharacteristic->setValue(doughHeightCTemp);
    pHeightCharacteristic->notify();
}

void BLEDoughHeight::sendStatustData(DoughServcieStatusEnum status) {
    Serial.printf("BLE Push Service Status '%d'\n", status);
    static char statusCTemp[6];
    dtostrf(status, 6, 0, statusCTemp);
    pStatusCharacteristic->setValue(statusCTemp);
    pStatusCharacteristic->notify();
}

void BLEDoughHeight::StartFermentation() {
    if (BLEDoughHeightCallback != NULL) {
        BLEDoughHeightCallback->onStart();
    }
}
void BLEDoughHeight::StopFermentation() {
    if (BLEDoughHeightCallback != NULL) {
        BLEDoughHeightCallback->onStop();
    }
}
