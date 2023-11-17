#include "syscall.h" 
#include "copyright.h" 
#define maxlen 32 
int main() 
{ 
    int id;
    char filename[maxlen];
    PrintString("\nEnter filename: ");
    ReadString(filename,maxlen);
    id=Open(filename, 1);
    Close(id);
    Remove(filename);
    Halt(); 
}