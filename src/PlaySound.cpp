#include "PlaySound.h"

PlaySound* PlaySound::instance /*= nullptr*/;

PlaySound::PlaySound(uint8_t bclk, uint8_t lrc, uint8_t dout)
    : bclkPin(bclk), lrcPin(lrc), doutPin(dout) {
    instance = this;
    audio.setPinout(bclkPin, lrcPin, doutPin);
    audio.setVolume(volumeLevel);
}

void PlaySound::setVolume(uint8_t volume) {
    volumeLevel = volume;
    audio.setVolume(volumeLevel);
}

void PlaySound::playSound(const char* filename) {
    if (filename == nullptr || strlen(filename) == 0) {
        Serial.println("Error: Invalid filename provided to playSound");
        return;
    }
    Serial.print("-- Starting sound playback task for file: ");
    Serial.println(filename);
    try {
        xTaskCreatePinnedToCore(this->i2sPlaySoundTask, "i2sPlaySoundTask", 4096, (void*)filename, 5, &soundTaskHandle, 1);
        // xTaskCreate(this->i2sPlaySoundTask, /* Function to implement the task */
        //                 "i2sPlaySoundTask", /* Name of the task */
        //                 4096,  /* 1024 Stack size in words */
        //                 (void*)filename,  /* Task input parameter */
        //                 0,  /* 5 Priority of the task */
        //                 &soundTaskHandle  /* Task handle. */
        //                 ); 
        
    }
    catch (const std::exception& e) {
        Serial.printf("Error creating task: %s\n", e.what());
        return;
    }
}   

void PlaySound::i2sPlaySoundTask(void* param) {

    // get the filename
    if (param == nullptr) {
        Serial.println("Error: No filename provided to i2sPlaySoundTask");
        vTaskDelete(NULL);
        return;
    }    
    const char* filename = (const char*)param;

    while (instance->audio.isRunning()) {
        Serial.println("Waiting for audio to finish...");
        //wait to finish previous sound to finish
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
 
    Serial.println("Starting audio playback...");
    instance->audio.connecttoFS(SPIFFS, filename);
    while (instance->audio.isRunning()) {
        instance->audio.loop();
        vTaskDelay(1);
    }
    instance->playing = false;
    vTaskDelete(NULL);
}

bool PlaySound::isRunning() {
    return audio.isRunning();
}

void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}