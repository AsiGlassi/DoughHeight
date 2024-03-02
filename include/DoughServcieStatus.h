#ifndef __Dough_Servcie_Status_h__
#define __Dough_Servcie_Status_h__

#include <RTClib.h>


enum DoughServcieStatusEnum {
  idle,
  Connected,
  Fermenting, 
  ReachedDesiredFerm, 
  OverFerm,
  Error
};


class DoughServcieStatus {

private:  
  //Stores the time fermentation started.
  DateTime m_startFermentationTime;
  
  //he dough initial height, represented by distance. 
  int m_doughInitDist=0;

  //The cup base height, represented by the distance. essential for understand the initiate height of the dough and fermentation calculation..
  int m_cupBaseDist=0;

  //Desired Fermentation Percentage - What is the desired fermentation amount.
  float m_desiredFermPercentage=0.40;

  //Fermentation Percentage above the desired fermentation percentage, will be recognized as Over Fermentation Percentage
  float m_overFermPercentage = 0.07;

  //Service Status
  DoughServcieStatusEnum m_doughServcieStatusEnum = DoughServcieStatusEnum::idle;

  //Dough Distance (Height)
  uint8_t m_doughHeight=0;

  //Fermentation Percentage - What is the desired fermentation amount.
  float m_fermPercentage=0.00;


public:
  DoughServcieStatusEnum getDoughServcieStatusEnum() {return m_doughServcieStatusEnum;}
  void setDoughServcieStatusEnum(DoughServcieStatusEnum statusEnum) {m_doughServcieStatusEnum = statusEnum;}

  DateTime getFermentationStart() {return m_startFermentationTime;}
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

  int getCupBaseDist() {
    return m_cupBaseDist;
  }
  void setCupBaseDist(int capBaseDist) {
    m_cupBaseDist = capBaseDist;
  }

  float getDesiredFermPercentage() {return m_desiredFermPercentage;}
  void setDesiredFermPercentage(float desiredPercentage) {m_desiredFermPercentage = desiredPercentage;}

  float getOverFermPercentage() {return m_overFermPercentage;}
  void setOverFermPercentage(float overFermPercentage) {m_overFermPercentage = overFermPercentage;}

  uint8_t getDoughHeight() {
    return m_doughHeight;
  }
  void setDoughHeight(uint8_t doughHeight) {
    m_doughHeight = doughHeight;
  }

  float getFermPercentage() {return m_fermPercentage;}
  void setFermPercentage(float fermPercentage) {m_fermPercentage = fermPercentage;}

};
#endif