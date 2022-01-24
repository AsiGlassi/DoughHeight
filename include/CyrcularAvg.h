#ifndef CyrcularAvg_h
#define CyrcularAvg_h

#include <Arduino.h>

class CyrcularAvg {
private:

    // int*  itemArray = 0;//Cant work with dynamic allocation - Mem overload
    int itemArray[40];
    byte arrayLen = 0;
    byte numOfItems = 0;

    int arraySum = 0;
    byte pointer = 0;
  
    void NextPointer();
public:
    CyrcularAvg(byte len);
 
  float Avg();
  void Insert(int value); 
  void printDebug();
  
};
#endif