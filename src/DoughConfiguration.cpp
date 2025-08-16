#include "DoughConfiguration.h"
#include <SPIFFS.h>

// Serialize to JSON
std::string DoughConfiguration::serializeToJson() {
    StaticJsonDocument<200> doc;
    doc["desiredFermPercentage"] = m_desiredFermPercentage;
    doc["overFermPercentage"] = m_overFermPercentage;
    doc["cupBaseHeight"] = m_cupBaseHeight;

    std::string jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

// Deserialize from JSON
void DoughConfiguration::deserializeFromJson(const std::string& jsonString) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, jsonString);

    if (!error) {
        m_desiredFermPercentage = doc["desiredFermPercentage"] | m_desiredFermPercentage;
        m_overFermPercentage = doc["overFermPercentage"] | m_overFermPercentage;
        m_cupBaseHeight = doc["cupBaseHeight"] | m_cupBaseHeight;
    }
}


bool DoughConfiguration::SaveConfigurationToFile(const char* fileName) {
    std::string jsonString = serializeToJson();
    File file = SPIFFS.open(fileName, "w");
    if (!file) {
        Serial.println("Failed to open configuration file for writing");
        return false;
    }
    file.print(jsonString.c_str());
    file.close();
    return true;
  }

  bool DoughConfiguration::LoadConfigurationFromFile(const char* fileName) {
    File file = SPIFFS.open(fileName, "r");
    if (!file) {
        Serial.println("Failed to open configuration file for reading");
        return false;
    }
    std::string jsonString = file.readString().c_str();
    deserializeFromJson(jsonString);
    file.close();
    return true;
  }