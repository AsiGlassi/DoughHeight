#include "BLEDoughHeight.h"
#include "BLEDoughHeightCharacteristic.h"


//Characteristic Call Backs Implementation
void heightCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {

    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
        Serial.printf("BLE Received Value: %s\n", rxValue.c_str());
    }
}

void heightCharacteristicCB::onRead(BLECharacteristic *pCharacteristic) {
    std::string txValue = std::string("N/A");//ToDo - Is this correct why not the real value?
    pCharacteristic->setValue(txValue.c_str());
}


void FermPercentageCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
        Serial.printf("BLE Write Fermentation Percentage Charachtiristic NOT IMPL: %s\n", rxValue.c_str());
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


void SessionStatusCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
        Serial.printf("BLE write Session Status Charachtiristic NOT IMPL: %s\n", rxValue.c_str());
    }}

void SessionStatusCharacteristicCB::onRead(BLECharacteristic *pCharacteristic) {
    Serial.printf("BLE read Session Status Charachtiristic NOT IMPL.\n");
}


void ConfigurationCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
        Serial.printf("BLE write Configuration Charachtiristic NOT IMPL: %s\n", rxValue.c_str());
    }
}

void ConfigurationCharacteristicCB::onRead(BLECharacteristic *pCharacteristic) {

    Serial.printf("BLE write Configuration Charachtiristic NOT IMPL.\n");

}


void CommandCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
    if (rxValue.length() == 1) {
        if (rxValue[0] == 0x0) { //Stop
            Serial.println(F("BLE Command Charachtiristic received Stop Service Command."));
            pBleDoughHeight->StopFermentation();
        } else if (rxValue[0] == 0x1) { //Start
            Serial.println(F("BLE Command Charachtiristic received Start Service Command."));
            pBleDoughHeight->StartFermentation();
        } else if (rxValue[0] == 0x2) { //Calibrate base
            Serial.println(F("BLE Command Charachtiristic received Start Service Command."));
            pBleDoughHeight->GeneralAction();
        } else {
            Serial.printf("BLE Command Charachtiristic Received Value: %s\n", rxValue.c_str());
        }
    }
    }
}

void CommandCharacteristicCB::onRead(BLECharacteristic *pCharacteristic) {
    Serial.printf("BLE Command Charachtiristic INVALID read request.\n");
}


void StatusCharacteristicCB::onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();

    if (rxValue.length() > 0) {
        Serial.printf("BLE Command Charachtiristic NOT IMPL: %s\n", rxValue.c_str());
    }
}

void StatusCharacteristicCB::onRead(BLECharacteristic *pCharacteristic) {
#ifdef DEBUG_BLE
    Serial.printf("BLE Status Charachtiristic read Status req: %d '%s'.\n", 
        pBleDoughHeight->getBleDoughServcieStatus().getDoughServcieStatusEnum(),
        pBleDoughHeight->getBleDoughServcieStatus().getDoughServcieStatusMessage().c_str());
#endif
    char statusCTemp[128];
    snprintf(statusCTemp, sizeof(statusCTemp),
        "{\"Status\": %d, \"Message\": \"%s\"}", 
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