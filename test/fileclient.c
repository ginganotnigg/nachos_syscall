#include "syscall.h"
#define maxLength 32
int main ()
{
    char buffer[1024];
    char src[maxLength], dst[maxLength];
    int srcID,dstID,size;
    int socket=SocketTCP();
    Connect(socket,"127.0.0.1", 1234);

    PrintString("Enter source filename: ");
    ReadString(src, maxLength);
    srcID=Open(src, 0);
    if(srcID>=0 && srcID<=20)
    {
        size=Seek(-1,srcID);
        Seek(0,srcID);
        Read(buffer,size,srcID);  
    }

    Send(socket, buffer, size);
 
    Receive(socket, buffer, size);

    PrintString("Enter destination filename: ");
    ReadString(dst,maxLength);
    dstID=Open(dst,0);
    if(dstID == -1) 
    {
        Create(dst);
        dstID=Open(dst, 0);
    }

    Write(buffer,size,dstID);
    Close(srcID);
    Close(dstID);
    Halt();
}