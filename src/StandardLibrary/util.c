#include "util.h"

int convertIntToAscii(int num, char *destination, int destinationSize){
    if(destinationSize < 2){
        return -1;
    }
    int totalDigits = 0;

    if(num == 0){
        destination[0] = '0';
        destination[1] = '\0';
        return ++totalDigits;
    }else if(num < 0){
        destination[0] = '-';
        num = -num;
        totalDigits++;
    }

    int temp = num; 
    while (temp != 0) {
        temp /= 10;
        totalDigits++;
    }

    if(totalDigits >= destinationSize){
        return -1;
    }
    destination[totalDigits] = '\0'; 
    int startIndex = (num < 0) ? 1 : 0; 
    for (int i = totalDigits - 1; i >= startIndex; --i) {
        destination[i] = (num % 10) + '0'; 
        num /= 10;
    }

    return totalDigits;
}

int convertAsciiToInt(char* buffer, int size, int* num){
    if(size < 2){
        return 0;
    }
    int number = 0;
    int sign = 1;
    int index = 0;
    if(buffer[index] == '-'){
        sign = -1;
        index++;
    }

    char temp = buffer[index];
    while(temp >= '0' && temp <= '9'){
        int digit = temp - '0';
        number = number * 10 + digit;
        temp = buffer[++index];
    }
    number *= sign;
    *num = number;
    return 1;
}