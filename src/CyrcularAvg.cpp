#include "CyrcularAvg.h"


template<class T>
void CyrcularAvg<T>::Insert(T value) {

    // Serial.printf("--> %d\n", value);
    
    //get next item pointer
    T theLastItem = Cyrcular<T>::GetNextItem();

    //subtract last item
    if (Cyrcular<T>::numOfItems >= Cyrcular<T>::arrayLen) {       
        arraySum -= theLastItem;
    }
    
    //insert new item
    Cyrcular<T>::Insert(value);
    arraySum += value;
};

template<class T>
float CyrcularAvg<T>::Avg() {
    if (Cyrcular<T>::numOfItems > Cyrcular<T>::arrayLen) {
        return (float)Cyrcular<T>::arraySum/Cyrcular<T>::arrayLen;
    } else {
        return (float)arraySum/Cyrcular<T>::numOfItems;
    }
};

template <class T>
void CyrcularAvg<T>::printDebug() {
    Serial.print("Array: ");
    Cyrcular<T>::printDebug();
    Serial.print(" Avg ");Serial.println(Avg());
};

