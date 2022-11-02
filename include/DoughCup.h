#pragma once

#include <Arduino.h>
#include <iostream>
#include <sstream>

class DoughCup {
private:

    int cupId = 0;
    byte height = 0;
  
public:
    DoughCup() {height = 0;}
    DoughCup(byte lHeight) {height = lHeight;}
    DoughCup(int lCupId, byte lHeight) {cupId = lCupId, height = lHeight;}

    std::string str() {
        std::stringstream ss;
        ss << "{\"" << cupId << "\":" << (int)height << "}";
        std::string ret = ss.str();
        return ret;
    }

    const char * c_str() {
        return str().c_str();
    }
};
