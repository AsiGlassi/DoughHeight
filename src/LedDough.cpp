#include "LedDough.h"

void LedDough::initLed() {

    strip.begin(); // Initialize pins for output
    strip.setBrightness(25);

    idle();
}


void LedDough::idle() {
    taskFinished=true;
    strip.fill((strip.gamma32(0xAC6199)));
    strip.show(); 
 }

void LedDough::BleConnected() {
    taskFinished=true;
    strip.fill((strip.gamma32(0xEEEEEE)));
    strip.show(); 
}

void LedDough::Fermenting() {

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

    strip.fill((strip.gamma32(0xFFFF88)));
    strip.show(); 
}

void LedDough::LedBlinkingTask( void * pvParameters ) {

    LedDough* pLedDough = (LedDough*)pvParameters;
    pLedDough->taskFinished=false;

    String taskMessage = "Task running on core ";
    taskMessage = taskMessage + xPortGetCoreID();
    Serial.println(taskMessage);  //log para o serial monitor

    int blinked=0;
  
    while(true){
      digitalWrite(33, !digitalRead(33));
      if (++blinked % 2 == 0 )
        blinked++;
 
        if (pLedDough->taskFinished) {
            digitalWrite(33, LOW);
            delay(50); //Wait a little before killing task
            vTaskDelete(NULL);
        }
        vTaskDelay(500 / portTICK_PERIOD_MS);
    } 
}


void LedDough::ReachedDesiredFerm() {
    taskFinished=true;
    strip.fill((strip.gamma32(0x33FF33)));
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
