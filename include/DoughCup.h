#pragma once

#include <Arduino.h>
#include <iostream>
#include <sstream>

class DoughCup {
private:

    std::string cupId;
    int height = 0;
  
public:
    DoughCup() {height = 0;}
    DoughCup(int lHeight) {height = lHeight;}
    DoughCup(std::string lCupId, int lHeight) {cupId = lCupId, height = lHeight;}

    std::string getCupId(){return cupId;}
    int getCupHeight(){return height;}

    std::string str() {
        std::stringstream ss;
        ss << "{\"" << cupId << "\":" << height << "}";
        // ss << "{\"" << cupId << "\":" << height << "}";
        std::string ret = ss.str();
        return ret;
    }

    const char * c_str() {
        return str().c_str();
    }
};
