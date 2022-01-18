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
    uint32_t colors[5]= {0xAC6199, 0xFFFF88, 0x33FF33, 0xFF7533, 0xEE3333};
    volatile uint32_t selectedColor = colors[0];

    //Task Parameters
    TaskHandle_t BlinkLedTaskHandle;
    volatile bool FadeLedTaskRunning = false;

private:
    void StartFadeLedTask(uint32_t color);
    static void LedBlinkingTask(void *pvParameters);
    void SetFillColor(uint32_t color);

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