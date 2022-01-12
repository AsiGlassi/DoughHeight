#ifndef __led_dough_h__
#define __led_dough_h__

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "DoughServcieStatus.h"


#define NUMPIXELS 3 
#define PIXELDATAPIN    12

class LedDough {

    //Pixel
    Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIXELDATAPIN, NEO_GRB + NEO_KHZ800);

public:
    void initLed();

    void BleConnected();

    void BleDisconnected();

    void idle();

    void Fermenting();

    void ReachedDesiredFerm();

    void OverFermentation();

    void Error();

};
#endif