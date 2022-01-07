#ifndef __Dough_Servcie_Status_h__
#define __Dough_Servcie_Status_h__

#include <RTClib.h>


enum DoughServcieStatusEnum {
  idle,
  Fermenting, 
  ReachedDesiredFerm, 
  OverFerm,
  Error
};


class DoughServcieStatus {

private:  
  DoughServcieStatusEnum m_doughServcieStatusEnum = DoughServcieStatusEnum::idle;
  DateTime m_startFermentationTime;
  int m_doughInitDist=0;
  int m_cupBaseDist=0;


public:
  DoughServcieStatusEnum getDoughServcieStatusEnum() {return m_doughServcieStatusEnum;}
  void setDoughServcieStatusEnum(DoughServcieStatusEnum statusEnum) {m_doughServcieStatusEnum = statusEnum;}

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

};
#endif