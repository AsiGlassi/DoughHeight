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
    strip.fill((strip.gamma32(0xEEEEEE)));
    strip.show(); 
}

void LedDough::Fermenting() {
    strip.fill((strip.gamma32(0xFFFF88)));
    strip.show(); 
}

void LedDough::ReachedDesiredFerm() {
    strip.fill((strip.gamma32(0x33FF33)));
    strip.show(); 
}

void LedDough::OverFermentation() {
    strip.fill((strip.gamma32(0xFF7533)));
    strip.show(); 
}

void LedDough::Error() {
    strip.fill((strip.gamma32(0xEE3333)));
    strip.show(); 
}
