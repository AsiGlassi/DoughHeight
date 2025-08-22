#ifndef __led_dough_h__
#define __led_dough_h__

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "DoughServcieStatus.h"



class LedDough {

    //Pixel
    Adafruit_NeoPixel strip;
    uint32_t colors[6]= {0xFFFFFF, 0xFFFF88, 0x33FF33, 0xFF7533, 0xEE3333};
    uint32_t clientConnectedcolor = 0xAC6199;
    volatile uint32_t selectedColor = colors[0];

    //Task Parameters
    TaskHandle_t BlinkLedTaskHandle;
    volatile bool FadeLedTaskRunning = false;

public:
    // Constructor that receives the data pin
    LedDough(uint8_t pixelDataPin, int numPixels)
        : strip(numPixels, pixelDataPin, NEO_GRB + NEO_KHZ800) {}

private:
    void StartFadeLedTask(uint32_t color);
    static void LedBlinkingTask(void *pvParameters);
    void SetFillColor(uint32_t color);

public:
    void initLed();

    void BleConnected();

    void BleDisConnected();

    void idle();

    void Fermenting();

    void ReachedDesiredFerm();

    void OverFermentation();

    void Error();

};
#endif