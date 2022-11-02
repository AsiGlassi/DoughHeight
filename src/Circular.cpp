#include "Circular.h"
#include <sstream>

template<class T>
Circular<T>::Circular(byte len, T tDefault) : defaultValue(tDefault) {
    arrayLen = len;
    pointer = arrayLen;
    defaultValue = tDefault;
    // if (itemArray != 0) {
    //     delete [] itemArray;
    // }
    // itemArray = (int *)malloc(len);
    for (byte i = 0; i < arrayLen; i++) {
        itemArray[i] = defaultValue;
    }
}


template<class T>
void Circular<T>::Insert(T value) {

    // Serial.printf("--> %d\n", value);
    
    //get next item pointer
    NextPointer();

    //subtract last item
    if (numOfItems < arrayLen) {       
        numOfItems++;
    }

    //insert new item
    itemArray[pointer] = value;
}


 template<class T>
 void Circular<T>::NextPointer() {

    if (pointer >= (arrayLen-1)) {
        pointer = 0;
    } else {
        pointer++;
    }
}

template<class T> 
T Circular<T>::GetNextItem() {
    T retItem;
    byte nextItem = pointer;
    if (pointer >= (arrayLen-1)) {
        nextItem = 0;
    } else {
        nextItem++;
    }
    retItem = itemArray[nextItem];
    return retItem; 
}

template <typename T> 
std::string toStr(T tmp)
{
    std::stringstream out;
    out << tmp;
    return out.str();
}


// template <typename T> T strTo(std::string tmp)
// {
//     T output;
//     istringstream in(tmp);
//     in >> output;
//     return output;
// }


 template<class T>
 void Circular<T>::printDebug() {
    Serial.print("Array: ");
    for (byte i = 0; i < arrayLen; i++) {
        if (pointer==i) {Serial.print("<");}
        Serial.print(toStr(itemArray[i]).c_str());
        if (pointer==i) {Serial.print(">");}
        Serial.print(" ");
    }
    Serial.println();
}

