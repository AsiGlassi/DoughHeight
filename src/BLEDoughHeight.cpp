#include "BLEDoughHeight.h"

void BLEDoughHeight::initBLE() {
    Serial.println("\nBLE Init");
    BLEDevice::init(DOUGH_DEVICE_NAME);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new TheServerCallBacks(this));
    //numHandles = (# of Characteristics)*2 + (# of Services) + (# of Characteristics with BLE2902)
    uint32_t numHandles = 5*2 + 1 + 2;
    pService = pServer->createService(BLEUUID(DOUGH_HEIGHT_SERVICE_UUID), 35, 0);


    //Height Measure
    pHeightCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_HEIGHT_UUID,
										BLECharacteristic::PROPERTY_NOTIFY |
                                        BLECharacteristic::PROPERTY_READ
									);
    pHeightCharacteristic->setCallbacks(new heightCharacteristicCB(this));

    BLE2902* doughHeightDescriptor = new BLE2902();
    doughHeightDescriptor->setNotifications(true);
    pHeightCharacteristic->addDescriptor(doughHeightDescriptor);

    BLEDescriptor heightRWDescriptor(BLEUUID((uint16_t)0x2901));//read write
    heightRWDescriptor.setValue("Dough Height in mm");
    pHeightCharacteristic->addDescriptor(&heightRWDescriptor);

    pHeightCharacteristic->setValue("N/A");


    //Fermentation Percentage 
    pFermPercentageCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_FERMENTATION_UUID,
                                        BLECharacteristic::PROPERTY_NOTIFY |
                                        BLECharacteristic::PROPERTY_READ
									);
    pFermPercentageCharacteristic->setCallbacks(new FermPercentageCharacteristicCB(this));

    BLE2902* fermDescriptor= new BLE2902();
    fermDescriptor->setNotifications(true);
    pFermPercentageCharacteristic->addDescriptor(fermDescriptor);

    BLEDescriptor fermRWDescriptor(BLEUUID((uint16_t)0x2901));//read write
    fermRWDescriptor.setValue("Fermentation Percentage");
    pFermPercentageCharacteristic->addDescriptor(&fermRWDescriptor);

    pFermPercentageCharacteristic->setValue("N/A");


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
                                BLECharacteristic::PROPERTY_READ |
                                BLECharacteristic::PROPERTY_NOTIFY
                            );
    pStatusCharacteristic->setCallbacks(new StatusCharacteristicCB(this));
        
    BLE2902* statusDescriptor= new BLE2902();
    statusDescriptor->setNotifications(true);
    pStatusCharacteristic->addDescriptor(statusDescriptor);

    BLEDescriptor statusRWDescriptor(BLEUUID((uint16_t)0x2901));//read write
    statusRWDescriptor.setValue("Service Status");
    pStatusCharacteristic->addDescriptor(&statusRWDescriptor);

    pStatusCharacteristic->setValue("{\"Status\": 0x0, \"Message\": \"N/A\"}");


    //Desired Fermentation Percentage 
    pDesiredFermPercentCharacteristic = pService->createCharacteristic(
										CHARACTERISTIC_DESIRED_FERMENTATION_UUID,
                                        BLECharacteristic::PROPERTY_READ |
                                        BLECharacteristic::PROPERTY_WRITE
									);
    pDesiredFermPercentCharacteristic->setCallbacks(new DesiredFermPercentageCharacteristicCB(this));

    BLEDescriptor DFPDescriptor(BLEUUID((uint16_t)0x2901));
    DFPDescriptor.setValue("Desired Fermentation Percentage");
    pDesiredFermPercentCharacteristic->addDescriptor(&DFPDescriptor);

    pDesiredFermPercentCharacteristic->setValue("N/A");



    pService->start();

    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE Dough Height Server Started\n");                              
}

    void BLEDoughHeight::setDeviceConnected(bool conn) {
        deviceConnected = conn;
        if (bleDoughHeightCallback != NULL) {
            if(deviceConnected) {
                bleDoughHeightCallback->onConnect(); 
            } else {
                bleDoughHeightCallback->onDisConnect();
            }
        }
    }


bool BLEDoughHeight::isDeviceConnected() {
       return deviceConnected;
}

void BLEDoughHeight::sendHeightData(uint8_t doughHeight) {
#ifdef DEBUG_BLE
    Serial.printf("BLE Push dought height '%d'\n", doughHeight);
#endif
    static char doughHeightCTemp[6];
    dtostrf(doughHeight, 6, 0, doughHeightCTemp);
    pHeightCharacteristic->setValue(doughHeightCTemp);
    pHeightCharacteristic->notify();
}

void BLEDoughHeight::sendDoughFermPercentData(float doughFermenPercent) {
#ifdef DEBUG_BLE
    Serial.printf("BLE Push dought Fermentation Percentage '%f'\n", doughFermenPercent);
#endif
    static char FermPercentageCTemp[10];
    snprintf(FermPercentageCTemp, sizeof(FermPercentageCTemp), "%f", doughFermenPercent);
    pFermPercentageCharacteristic->setValue(FermPercentageCTemp);
    pFermPercentageCharacteristic->notify();
}

void BLEDoughHeight::sendStatustData(DoughServcieStatusEnum status) {
#ifdef DEBUG_BLE
    Serial.printf("BLE Push Service Status '%d' ;'%s'\n", status, msg.c_str());
#endif
    static char statusCTemp[128];
    snprintf(statusCTemp, sizeof(statusCTemp), "{\"Status\": 0x%X}", status);
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



void heightCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {

    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
        Serial.printf("BLE Received Value: %s\n", rxValue.c_str());
    }
}

void heightCharacteristicCB::onRead(BLECharacteristic *pCharacteristic) {
    std::string txValue = std::string("N/A");
    pCharacteristic->setValue(txValue.c_str());
}


void FermPercentageCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
        Serial.printf("BLE Fermentation Percentage Charachtiristic NOT IMPL: %s\n", rxValue.c_str());
    }
}

void FermPercentageCharacteristicCB::onRead(BLECharacteristic *pCharacteristic) {
    
    float percnt = pBleDoughHeight->getDoughFermentationPercent();
#ifdef DEBUG_BLE
    Serial.printf("BLE Fermentation Percentage Charachtiristic Read Value req: '%f'.\n", percnt);
#endif
    char statusCTemp[10];
    snprintf(statusCTemp, sizeof(statusCTemp), "%f", percnt);
    pCharacteristic->setValue(statusCTemp);
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
    Serial.printf("BLE StartStop Charachtiristic INVALID read request.\n");
}


void StatusCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
        Serial.printf("BLE Status Charachtiristic NOT IMPL: %s\n", rxValue.c_str());
    }
}

void StatusCharacteristicCB::onRead(BLECharacteristic *pCharacteristic) {
#ifdef DEBUG_BLE
    Serial.printf("BLE Status Charachtiristic read Status req: '%d'.\n", pBleDoughHeight->getBleDoughServcieStatus().getDoughServcieStatusEnum());
#endif
    char statusCTemp[128];
    snprintf(statusCTemp, sizeof(statusCTemp),
        "{\"Status\": 0x%X, \"Message\": \"%s\"}", 
        pBleDoughHeight->getBleDoughServcieStatus().getDoughServcieStatusEnum(), 
        pBleDoughHeight->getBleDoughServcieStatus().getDoughServcieStatusMessage().c_str());

    pCharacteristic->setValue(statusCTemp);
}


void DesiredFermPercentageCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if ((rxValue.length() > 0) && (rxValue.length() < 7)) {
        float precent  = atof(rxValue.c_str());
        pBleDoughHeight->getBleDoughServcieStatus().setDesiredFermPercentage(precent);
#ifdef DEBUG_BLE
        Serial.printf("BLE Desired Fermentation Percentage Charachtiristic Received Value: %f\n", precent);
#endif
    } else {
        Serial.printf("BLE Desired Fermentation Percentage Charachtiristic Received INVALID Value: %s\n", rxValue.c_str());
    }

}

void DesiredFermPercentageCharacteristicCB::onRead(BLECharacteristic *pCharacteristic) {
    
#ifdef DEBUG_BLE
    Serial.printf("BLE Desired Fermentation Percentage Charachtiristic Read Value req: '%f'.\n", 
        pBleDoughHeight->getBleDoughServcieStatus().getDesiredFermPercentage());
#endif
    char statusCTemp[6];
    sprintf(statusCTemp, "%f", pBleDoughHeight->getBleDoughServcieStatus().getDesiredFermPercentage());
    pCharacteristic->setValue(statusCTemp);
}