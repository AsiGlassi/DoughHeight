#ifndef CyrcularAvg_h
#define CyrcularAvg_h

#include <Arduino.h>
#include "Cyrcular.h"

template <class T>
class CyrcularAvg : public Cyrcular<T> {
private:

    int arraySum = 0;
public:
    CyrcularAvg(byte lenx,  T tDefaultN) : Cyrcular<T> {lenx, tDefaultN} {
    };

    virtual void Insert(T value); 
    virtual float Avg();
    virtual void printDebug();
};
#endif