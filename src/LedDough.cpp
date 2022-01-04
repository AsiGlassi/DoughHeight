#include "LedDough.h"

void LedDough::initLed() {

    strip.begin(); // Initialize pins for output
    strip.setBrightness(25);

    idle();
}


void LedDough::idle() {
    strip.fill((strip.gamma32(0xAC6199)));
    strip.show(); 
 }

void LedDough::BleConnected() {
    strip.fill((strip.gamma32(0xFF7533)));
    strip.show(); 
}

void LedDough::Fermenting() {
    strip.fill((strip.gamma32(0xF7FF33)));
    strip.show(); 
}