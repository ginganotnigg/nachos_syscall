#include "syscall.h" 
#include "copyright.h"
#define maxlen 32 
int main() 
{ 
    int id, headID, tailID, headSize, tailSize;
    char head[500],tail[500];
    char headName[32];
    char tailName[32];
    char concatName[32];

    PrintString("Enter first filename: ");
    ReadString(headName,32);
    PrintString("Enter second filename: ");
    ReadString(tailName,32);
    PrintString("Enter concatenated filename: ");
    ReadString(concatName,32);

    headID=Open(headName, 0);
    tailID=Open(tailName, 0);

    id=Open(concatName,0);
    if(id == -1) 
    {
        Create(concatName);
        id=Open(concatName,0);
    }

    if((id>0&&id<20) && (headID>0&&headID<20) && (tailID>0&&tailID<20))
    {
        headSize= Seek(-1, headID);
        tailSize= Seek(-1, tailID);
        
        Seek(0,headID);
        Seek(0,tailID);

        Read(head,headSize,headID);
        Read(tail,tailSize,tailID);
        Write(head,headSize,id);
        Seek(headSize,id);
        Write(tail,tailSize,id);
    }
    Close(headID);
    Close(tailID);
    Close(id);
    Halt(); 
}