#include "LedDough.h"

void LedDough::initLed() {

    strip.begin(); // Initialize pins for output
    strip.setBrightness(25);

    idle();
}


void LedDough::idle() {

        //Blinking Task
    if (taskFinished) {//(String)eTaskGetState(&h_gsm_loop
        xTaskCreatePinnedToCore(this->LedBlinkingTask, /* Function to implement the task */
                          "Blinking", /* Name of the task */
                          10000,  /* Stack size in words */
                          this,  /* Task input parameter */
                          0,  /* Priority of the task */
                          &BlinkLedTaskHandle,  /* Task handle. */
                          0); /* Core where the task should run */
    }

    // strip.fill((strip.gamma32(0xAC6199)));
    // strip.show(); 
 }

void LedDough::BleConnected() {
    taskFinished=true;
    strip.fill((strip.gamma32(0xEEEEEE)));
    strip.show(); 
}

void LedDough::Fermenting() {
    taskFinished=true;
    strip.fill((strip.gamma32(0xFFFF88)));
    strip.show(); 
}

void LedDough::LedBlinkingTask( void * pvParameters ) {

    LedDough* pLedDough = (LedDough*)pvParameters;
    pLedDough->taskFinished=false;

    String taskMessage = "Task running on core ";
    taskMessage = taskMessage + xPortGetCoreID();
    Serial.println(taskMessage);  //log para o serial monitor

    uint16_t deg = 90;
    uint8_t degStep = 15;
    uint32_t origColor = 0xAC6199;//pLedDough->strip.getPixelColor(0);
    uint16_t curr_r, curr_g, curr_b;
    curr_b = origColor & 0x00FF; curr_g = (origColor >> 8) & 0x00FF; curr_r = (origColor >> 16) & 0x00FF;  // separate into RGB components
  
    // Serial.printf(" R: %dX\tG: %d\tB:%d\n", curr_r, curr_g, curr_b);

    while(!pLedDough->taskFinished) {

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


void LedDough::ReachedDesiredFerm() {
    taskFinished=true;
    strip.fill(strip.gamma32(0x33FF33));
    strip.show(); 
}

void LedDough::OverFermentation() {
    taskFinished=true;
    strip.fill((strip.gamma32(0xFF7533)));
    strip.show(); 
}

void LedDough::Error() {
    taskFinished=true;
    strip.fill((strip.gamma32(0xEE3333)));
    strip.show(); 
}
