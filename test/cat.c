#include "syscall.h" 
#include "copyright.h" 
#define maxlen 32 
int main() 
{ 
    int id,size;
    char buffer[1024];
    char filename[maxlen];
    PrintString("\nEnter filename: ");
    ReadString(filename, maxlen);
    id=Open(filename, 1);
    if(id>0 && id<21)
    {
        size=Seek(-1, id);
        Seek(0, id);
        Read(buffer, size, id);
        PrintString("Content:\n");
        PrintString(buffer);
        PrintString("\n");
    }
    Close(id);
    Halt(); 
}