#ifndef __Dough_Configuration_h__
#define __Dough_Configuration_h__


#include <ArduinoJson.h> // Include ArduinoJson library
#include <string>   

class DoughConfiguration {

    private:

      // Default floor distance - At the moment doesnt subject to change
      int floorDist = 146;  

      // Desired Fermentation Percentage - What is the desired fermentation amount.
      float m_desiredFermPercentage = 0.3;
    
      // Fermentation Percentage above the desired fermentation percentage, will be recognized as Over Fermentation Percentage
      float m_overFermPercentage = 0.15;
    
      // The cup base height, represented by the height.
      int m_cupBaseHeight = 10;
    
    public:

      // Preaty print 
      void PrintConfiguration() {
        Serial.printf("Desired Fermentation Percentage: %2f\n", m_desiredFermPercentage);
        Serial.printf("Cup Base Height: %d\n", m_cupBaseHeight);
      }

      // Getters for Floor Distance
      int getFloorDist() { return floorDist; }

      // Getters and Setters for Desired Fermentation Percentage
      float getDesiredFermPercentage() { return m_desiredFermPercentage; }
      void setDesiredFermPercentage(float desiredPercentage) { m_desiredFermPercentage = desiredPercentage; }
    
      // Getters and Setters for Over Fermentation Percentage
      float getOverFermPercentage() { return m_overFermPercentage; }
      void setOverFermPercentage(float overFermPercentage) { m_overFermPercentage = overFermPercentage; }
    
      // Getters and Setters for Cup Base Height
      int getCupBaseHeight() { return m_cupBaseHeight; }
      void setCupBaseHeight(int cupBaseHeight) { m_cupBaseHeight = cupBaseHeight; }

      std::string serializeToJson();
      void deserializeFromJson(const std::string& jsonString);

      bool SaveConfigurationToFile(const char* fileName);

      bool LoadConfigurationFromFile(const char* fileName);
    };
    
    #endif