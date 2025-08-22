#ifndef __Dough_Servcie_Status_h__
#define __Dough_Servcie_Status_h__

#include <RTClib.h>

enum DoughServcieStatusEnum {
  idle,
  //Connected, it doesnt matter if there is a client connected except
  Fermenting,
  ReachedDesiredFerm,
  OverFerm,
  Error
};

class DoughServcieStatus {

private:
  // Stores the time fermentation started.
  DateTime m_startFermentationTime;

  // he dough initial height, represented by distance.
  int m_doughInitDist = 0;
 
  // Service Status
  DoughServcieStatusEnum m_doughServcieStatusEnum = DoughServcieStatusEnum::idle;
  std::string m_serviceMessage = "Idle";

  // Dough Distance (Height)
  uint8_t m_doughHeight = 0;

  // Fermentation Percentage - What is the desired fermentation amount.
  float m_fermPercentage = 0.00;

public:
  DoughServcieStatusEnum getDoughServcieStatusEnum() { return m_doughServcieStatusEnum; }
  void setDoughServcieStatusEnum(DoughServcieStatusEnum statusEnum) { m_doughServcieStatusEnum = statusEnum; }
  void setDoughServcieStatusEnum(DoughServcieStatusEnum statusEnum, std::string msg) {
    m_doughServcieStatusEnum = statusEnum;
    m_serviceMessage = msg;
  }
  std::string getDoughServcieStatusMessage() { return m_serviceMessage; }

  DateTime getFermentationStart() { return m_startFermentationTime; }
  void setFermentationStart(DateTime regTime) {
    m_doughServcieStatusEnum = DoughServcieStatusEnum::Fermenting;
    m_startFermentationTime = regTime;
  }

  int getDoughInitDist() {
    return m_doughInitDist;
  }
  void setDoughInitDist(int initDist) {
    m_doughInitDist = initDist;
  }

   uint8_t getDoughHeight() {
    return m_doughHeight;
  }
  void setDoughHeight(uint8_t doughHeight) {
    m_doughHeight = doughHeight;
  }

  float getFermPercentage() { return m_fermPercentage; }
  void setFermPercentage(float fermPercentage) { m_fermPercentage = fermPercentage; }
};
#endif