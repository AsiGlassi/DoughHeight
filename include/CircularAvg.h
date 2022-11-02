#ifndef CircularAvg_h
#define CircularAvg_h

#include <Arduino.h>
#include "Circular.h"

template <class T>
class CircularAvg : public Circular<T> {
private:

    int arraySum = 0;
public:
    CircularAvg(byte lenx,  T tDefaultN) : Circular<T> {lenx, tDefaultN} {
    };

    virtual void Insert(T value); 
    virtual float Avg();
    virtual void printDebug();
};
#endif