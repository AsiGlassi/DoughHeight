#include "CircularAvg.h"


template<class T>
void CircularAvg<T>::Insert(T value) {

    // Serial.printf("--> %d\n", value);
    
    //get next item pointer
    T theLastItem = Circular<T>::GetNextItem();

    //subtract last item
    if (Circular<T>::numOfItems >= Circular<T>::arrayLen) {       
        arraySum -= theLastItem;
    }
    
    //insert new item
    Circular<T>::Insert(value);
    arraySum += value;
};

template<class T>
float CircularAvg<T>::Avg() {
    if (Circular<T>::numOfItems > Circular<T>::arrayLen) {
        return (float)Circular<T>::arraySum/Circular<T>::arrayLen;
    } else {
        return (float)arraySum/Circular<T>::numOfItems;
    }
};

template <class T>
void CircularAvg<T>::printDebug() {
    Serial.print("Array: ");
    Circular<T>::printDebug();
    Serial.print(" Avg ");Serial.println(Avg());
};

