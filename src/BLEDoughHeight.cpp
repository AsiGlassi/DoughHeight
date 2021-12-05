#include "BLEDoughHeight.h"

void BLEDoughHeight::initBLE() {
    Serial.println("BLE Init");
    BLEDevice::init("Asi Dough Height");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new TheServerCallBacks(this));
    pService = pServer->createService(DOUGH_HEIGHT_SERVICE_UUID);
    pTxCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_HEIGHT_UUID,
										BLECharacteristic::PROPERTY_NOTIFY |
                                        BLECharacteristic::PROPERTY_READ
									);
    pTxCharacteristic->setCallbacks(new theCharacteristicCallbacks());

    BLEDescriptor doughHeightDescriptor(BLEUUID((uint16_t)0x2902));
    doughHeightDescriptor.setValue("Dough Height");
    pTxCharacteristic->addDescriptor(&doughHeightDescriptor);
    
    pTxCharacteristic->setValue("N/A");

    // // pRxCharacteristic = pService->createCharacteristic(
	// // 									CHARACTERISTIC_UUID_RX,
	// // 									BLECharacteristic::PROPERTY_WRITE
	// // 									);

    // pRxCharacteristic->setCallbacks(new theCharacteristicCallbacks());
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

void BLEDoughHeight::sendData(int doughHeight) {
    Serial.printf("BLE Push dought height '%d'\n", doughHeight);
    static char temperatureCTemp[6];
    dtostrf(doughHeight, 6, 0, temperatureCTemp);
    pTxCharacteristic->setValue(temperatureCTemp);
    pTxCharacteristic->notify();
}
