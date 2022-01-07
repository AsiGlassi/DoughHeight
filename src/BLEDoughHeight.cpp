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
    pHeightCharacteristic->setCallbacks(new heightCharacteristicCB());

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
    pStartCharacteristic->setCallbacks(new StartCharacteristicCB(this));

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
    pStatusCharacteristic->setCallbacks(new StatusCharacteristicCB(this));

    BLEDescriptor statusDescriptor(BLEUUID((uint16_t)0x2902));
    statusDescriptor.setValue("Service Status");
    pStatusCharacteristic->addDescriptor(&statusDescriptor);

    BLEDescriptor statusRWDescriptor(BLEUUID((uint16_t)0x2901));//read write
    statusDescriptor.setValue("Service Status");
    pStatusCharacteristic->addDescriptor(&statusDescriptor);

    pStatusCharacteristic->setValue("N/A");


    //Desired Fermentation Percentage 
    pDesiredFermPercentageCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_DESIRED_FERMENTATION_UUID,
                                        BLECharacteristic::PROPERTY_READ |
                                        BLECharacteristic::PROPERTY_WRITE
									);
    pDesiredFermPercentageCharacteristic->setCallbacks(new DesiredFermPercentageCharacteristicCB(this));

    // BLEDescriptor desiredFermDescriptor(BLEUUID((uint16_t)0x2902));
    // desiredFermDescriptor.setValue("Desired Fermentation Percentage Notification");
    // pDesiredFermPercentageCharacteristic->addDescriptor(&desiredFermDescriptor);

    BLEDescriptor desiredFermRWDescriptor(BLEUUID((uint16_t)0x2901));//read write
    desiredFermRWDescriptor.setValue("Desired Fermentation Percentage");
    pDesiredFermPercentageCharacteristic->addDescriptor(&desiredFermRWDescriptor);

    pDesiredFermPercentageCharacteristic->setValue("N/A");



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

void BLEDoughHeight::sendHeightData(uint8_t doughHeight) {
    Serial.printf("BLE Push dought height '%d'\n", doughHeight);
    static char doughHeightCTemp[6];
    dtostrf(doughHeight, 6, 0, doughHeightCTemp);
    pHeightCharacteristic->setValue(doughHeightCTemp);
    pHeightCharacteristic->notify();
}

void BLEDoughHeight::sendStatustData(DoughServcieStatusEnum status) {
    Serial.printf("BLE Push Service Status '%d'\n", status);
    static char statusCTemp[6];
    dtostrf(status, 1, 0, statusCTemp);
    pStatusCharacteristic->setValue(statusCTemp);
    pStatusCharacteristic->notify();
}

void BLEDoughHeight::StartFermentation() {
    if (bleDoughHeightCallback != NULL) {
        bleDoughHeightCallback->onStart();
    }
}

void BLEDoughHeight::StopFermentation() {
    if (bleDoughHeightCallback != NULL) {
        bleDoughHeightCallback->onStop();
    }
}

void StartCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {
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

void StartCharacteristicCB::onRead(BLECharacteristic *pCharacteristic) {
    Serial.printf("BLE StartStop Charachtiristic invalid read request.\n");
}


void StatusCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
    Serial.printf("BLE Status Charachtiristic NOT IMPL: %s\n", rxValue.c_str());
    }
}

void StatusCharacteristicCB::onRead(BLECharacteristic *pCharacteristic) {
    
    Serial.printf("BLE Status Charachtiristic read status: '%d'.\n", pBleDoughHeight->getBleDoughServcieStatus().getDoughServcieStatusEnum());

    char statusCTemp[6];
    sprintf(statusCTemp, "%d", pBleDoughHeight->getBleDoughServcieStatus().getDoughServcieStatusEnum());
    pCharacteristic->setValue(statusCTemp);
}


void DesiredFermPercentageCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if ((rxValue.length() > 0) && (rxValue.length() < 7)) {
        float precent  = atof(rxValue.c_str());
        Serial.printf("BLE Desired Fermentation Percentage Charachtiristic Received Value: %f\n", precent);
    } else {
        Serial.printf("BLE Desired Fermentation Percentage Charachtiristic Received INVALID Value: %s\n", rxValue.c_str());
    }
}

void DesiredFermPercentageCharacteristicCB::onRead(BLECharacteristic *pCharacteristic) {
    
    Serial.printf("BLE Desired Fermentation Percentage  Charachtiristic Read Value: '%f'.\n", 
        pBleDoughHeight->getBleDoughServcieStatus().getDesiredFermPercentage());

    char statusCTemp[6];
    sprintf(statusCTemp, "%f", pBleDoughHeight->getBleDoughServcieStatus().getDesiredFermPercentage());
    pCharacteristic->setValue(statusCTemp);
}