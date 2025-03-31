#pragma once
#include <Arduino.h>

template <class T>
class Circular {

protected:

    // int*  itemArray = 0;//Cant work with dynamic allocation - Mem overload
    T itemArray[40];
    T defaultValue;

    byte arrayLen = 0;
    byte numOfItems = 0;

    int arraySum = 0;
    byte pointer = 0;
  
    void NextPointer();

public:
    Circular(byte len, T tDefault);
 
    T GetLastItem() {return itemArray[pointer];};
    T GetNextItem();
    virtual void Insert(T value);
    void SetAll(T value); 
    virtual void printDebug();
    bool BufferFull() {return numOfItems >= arrayLen;}
};
