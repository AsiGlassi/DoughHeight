#include "LedDough.h"

void LedDough::initLed() {

    strip.begin(); // Initialize pins for output
    strip.setBrightness(25);

    idle();
}


void LedDough::idle() {
    SetFillColor(colors[DoughServcieStatusEnum::idle]);
 }

void LedDough::BleConnected() {
    SetFillColor(0xEEEEEE);
}

void LedDough::Fermenting() {
    StartFadeLedTask(colors[DoughServcieStatusEnum::Fermenting]);
}


void LedDough::ReachedDesiredFerm() {
    SetFillColor(colors[DoughServcieStatusEnum::ReachedDesiredFerm]);
}

void LedDough::OverFermentation() {
    SetFillColor(colors[DoughServcieStatusEnum::OverFerm]);
}

void LedDough::Error() {
    SetFillColor(colors[DoughServcieStatusEnum::Error]);
}


//set all pixel to the same color
void LedDough::SetFillColor(uint32_t color) {
    FadeLedTaskRunning=false;
    strip.fill((strip.gamma32(color)));
    strip.show(); 
}

void LedDough::StartFadeLedTask(uint32_t color) {
    
    //set selected color
    selectedColor = color;

    //Blinking Task
    if (!FadeLedTaskRunning) {//(String)eTaskGetState(&BlinkLedTaskHandle)
        xTaskCreatePinnedToCore(this->LedBlinkingTask, /* Function to implement the task */
                          "FadeLedTask", /* Name of the task */
                          1024,  /* Stack size in words */
                          this,  /* Task input parameter */
                          0,  /* Priority of the task */
                          &BlinkLedTaskHandle,  /* Task handle. */
                          0); /* Core where the task should run */
    }
}

//Fade on/off all leds 
void LedDough::LedBlinkingTask( void * pvParameters ) {

    //get main class ref
    LedDough* pLedDough = (LedDough*)pvParameters;
    //Mark Task Started
    pLedDough->FadeLedTaskRunning=true;

    // String taskMessage = "Task running on core ";
    // taskMessage = taskMessage + xPortGetCoreID();
    // Serial.println(taskMessage);  //log para o serial monitor

    uint16_t deg = 90;
    uint8_t degStep = 15;
    uint32_t origColor = pLedDough->selectedColor;
    uint16_t curr_r, curr_g, curr_b;
    curr_b = origColor & 0x00FF; curr_g = (origColor >> 8) & 0x00FF; curr_r = (origColor >> 16) & 0x00FF;  // separate into RGB components
  
    // Serial.printf(" R: %dX\tG: %d\tB:%d\n", curr_r, curr_g, curr_b);

    while(pLedDough->FadeLedTaskRunning) {

        deg = deg + degStep;
        if (deg > 360) {deg = deg - 360;}

        float sinRes = (sin(deg*PI/180) + 1) /2;
        // Serial.printf("%d-%f \t R: %d\tG: %d\tB:%d\n", deg, sinRes, (uint16_t)(sinRes * curr_r), (uint16_t)(sinRes * curr_g), (uint16_t)(sinRes * curr_b));   
        uint32_t newColor = (uint32_t)(sinRes * curr_b) + ((uint32_t)(sinRes * curr_g) << 8) + ((uint32_t)(sinRes * curr_r) << 16);

        pLedDough->strip.fill(pLedDough->strip.gamma32(newColor));
        pLedDough->strip.show();
        
        vTaskDelay(125 / portTICK_PERIOD_MS);
    } 

    delay(50); //Wait a little before killing task
    vTaskDelete(NULL);
}

