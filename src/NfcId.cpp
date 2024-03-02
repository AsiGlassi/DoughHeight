#include "NfcId.h"

void NfcId::setIds(uint8_t nfcIds[7]) {
    for (uint8_t i = 0; i < 7; i++)
    {
      ids[i] = nfcIds[i];
    //   Serial.print(" 0x"); Serial.print(retUId.uid[i], HEX);
    }
}


bool NfcId::isEmpty() {
    bool empty = true;
    for (uint8_t i = 0; i < 7; i++)
    {
      if (ids[i] != 0) {
        empty = false;
        break;
      }  
    }
    return empty;    
}


std::string NfcId::str() {
    std::stringstream ss;
    for (uint8_t i = 0; i < 6; i++) {
        ss << "0x";
        ss.fill('0');
	    ss.width(2);
        ss << std::noshowbase << std::uppercase << std::hex << (int)ids[i] << ":";
    }
    ss << "0x";
    ss.fill('0');
	ss.width(2);
    ss << std::hex << (int)ids[6];
    std::string ret = ss.str();
    return ret;
}