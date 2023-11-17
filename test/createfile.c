#include "syscall.h"

int main() {
    char fileName[256];
    int length;

    //PrintString("Enter filename's length: ");
    //length = ReadNum();
    PrintString("Enter filename: ");
    ReadString(fileName, 32);
    
    if (Create(fileName) == 0){
        PrintString("Create file ");
        PrintString(fileName);
        PrintString(" successfully\n");
    }
    else {
        PrintString("Failed to create file\n");
    }
    Halt();
}