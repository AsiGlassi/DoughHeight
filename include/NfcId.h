#pragma once

#include <Arduino.h>
#include <iostream>
#include <sstream>


class NfcId {
    
    private:
    uint8_t ids[7] = {0, 0, 0, 0, 0, 0, 0};

    public:
    NfcId(){};
    NfcId(uint8_t nfcIds[7]){setIds(nfcIds);};

    void setIds(uint8_t nfcIds[7]);
    std::string str();
};