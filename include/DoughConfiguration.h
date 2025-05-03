#ifndef __Dough_Configuration_h__
#define __Dough_Configuration_h__


class DoughConfiguration {

    private:
      // Desired Fermentation Percentage - What is the desired fermentation amount.
      float m_desiredFermPercentage = 0.10;
    
      // Fermentation Percentage above the desired fermentation percentage, will be recognized as Over Fermentation Percentage
      float m_overFermPercentage = 0.07;
    
      // The cup base height, represented by the distance.
      int m_cupBaseDist = 0;
    
    public:
      // Getters and Setters for Desired Fermentation Percentage
      float getDesiredFermPercentage() { return m_desiredFermPercentage; }
      void setDesiredFermPercentage(float desiredPercentage) { m_desiredFermPercentage = desiredPercentage; }
    
      // Getters and Setters for Over Fermentation Percentage
      float getOverFermPercentage() { return m_overFermPercentage; }
      void setOverFermPercentage(float overFermPercentage) { m_overFermPercentage = overFermPercentage; }
    
      // Getters and Setters for Cup Base Distance
      int getCupBaseDist() { return m_cupBaseDist; }
      void setCupBaseDist(int cupBaseDist) { m_cupBaseDist = cupBaseDist; }
    };
    
    #endif