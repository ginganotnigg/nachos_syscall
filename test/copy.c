#include "syscall.h"
#include "copyright.h" 
#define maxlen 32 
int main() 
{ 
    int srcID,dstID,size,temp;
    char buffer[1024];
    char src[maxlen];
    char dst[maxlen];

    PrintString("Enter source filename: ");
    ReadString(src, maxlen);
        
    PrintString("Enter destination filename: ");
    ReadString(dst, maxlen);

    srcID=Open(src, 0);
    dstID=Open(dst, 0);
    if(dstID == -1) 
    {
        Create(dst);
        dstID=Open(dst, 0);
    }
        
    if((srcID>0 && srcID<20) && (dstID>0 && dstID<20))
    {
        size=Seek(-1,srcID);
        temp=Seek(0,srcID);
        Read(buffer,size,srcID);
        Write(buffer,size,dstID);
        Close(srcID);
        Close(dstID);
    }
    Halt(); 
}