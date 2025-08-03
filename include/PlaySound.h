#pragma once
#include <Arduino.h>
#include <SPIFFS.h>
#include "Audio.h"

class PlaySound {
public:
    PlaySound(uint8_t bclk, uint8_t lrc, uint8_t dout);
    void playSound(const char* filename);
    void setVolume(uint8_t volume);

    bool isRunning();

private:
    Audio audio;
    TaskHandle_t soundTaskHandle;
    volatile bool soundTaskRunning = false;

    static void i2sPlaySoundTask(void* param);

    static PlaySound* instance;
    const char* currentFile /*= nullptr*/;
    bool playing = false;
    uint8_t volumeLevel=50;

    // i2s pin configuration
    uint8_t bclkPin, lrcPin, doutPin;
};