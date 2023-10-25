#include "syscall.h"

int main() {
    /*char fileName[256];
    int length;

    PrintString("Enter file's name's length: ");
    length = ReadNum();
    PrintString("Enter file's name: ");
    ReadString(fileName, length);*/

    Create("temp.txt");
    return 0;
}