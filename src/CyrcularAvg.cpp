#include "CyrcularAvg.h"

CyrcularAvg::CyrcularAvg(byte len) {
    arrayLen = len;
    pointer = arrayLen;
    // if (itemArray != 0) {
    //     delete [] itemArray;
    // }
    // itemArray = new int [arrayLen];
    // itemArray = (int *)malloc(len);
    for (byte i = 0; i < arrayLen; i++) {
        itemArray[i] = 0;
    }
}


void CyrcularAvg::Insert(int value) {

    // Serial.printf("--> %d\n", value);
    
    //get next item pointer
    NextPointer();

    //subtract last item
    if (numOfItems >= arrayLen) {       
        arraySum -= itemArray[pointer];
    } else {
        numOfItems++;
    }

    //insert new item
    itemArray[pointer] = value;
    arraySum += value;
}

void CyrcularAvg::NextPointer() {

    if (pointer >= (arrayLen-1)) {
        pointer = 0;
    } else {
        pointer++;
    }
}

float CyrcularAvg::Avg() {
    if (numOfItems > arrayLen) {
        return (float)arraySum/arrayLen;
    } else {
        return (float)arraySum/numOfItems;
    }
}

void CyrcularAvg::printDebug() {
    Serial.print("Array: ");
    for (byte i = 0; i < arrayLen; i++) {
        if (pointer==i) {Serial.print("<");}
        Serial.print(itemArray[i]);
        if (pointer==i) {Serial.print(">");}
        Serial.print(" ");
    }
    Serial.print(" Avg ");Serial.println(Avg());
}

