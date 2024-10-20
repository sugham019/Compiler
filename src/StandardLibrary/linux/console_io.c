#include "../console_io.h"
#include "../util.h"
#include <unistd.h>
#include "../syscall.h"

#define BUFFER_SIZE 12

void printChar(char ch){
    sys_write(1, &ch, sizeof(ch));
}

void printlnChar(char ch){
    char buffer[2] = {ch, '\n'};
    sys_write(1, buffer, sizeof(buffer));
}

void printInt(int num){
    char buffer[BUFFER_SIZE] = {'\0'};
    int len = convertIntToAscii(num, buffer, BUFFER_SIZE);
    sys_write(1, buffer, len);
}

void printlnInt(int num){
    char buffer[BUFFER_SIZE] = {'\0'};
    int len = convertIntToAscii(num, buffer, BUFFER_SIZE);
    buffer[len] = '\n';
    sys_write(1, buffer, len+1);
}

int getNextInt(){
    char buffer[BUFFER_SIZE] = {'\0'};
    int index = 0;
    char temp = getNextChar();
    while(temp != '\n'){
        if(index == BUFFER_SIZE){
            break;
        }
        buffer[index++] = temp;
        temp = getNextChar();
    }
    int num = 0;
    convertAsciiToInt(buffer, index+1, &num);
    return num;
}        

char getNextChar(){
    char ch;
    sys_read(0, &ch, sizeof(ch));
    return ch;
}
