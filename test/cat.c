#include "syscall.h"

int main() {
    char fileName[256];
    int id;
    int fileSize;
    char buffer[1024];

    PrintString("Enter filename: ");
    ReadString(fileName, 32);

    if ((id = Open(fileName, 0)) != -1) {
        PrintString("Opened file ");
        PrintString(fileName);
        PrintString(" successfully!\n");
        PrintString("ID: ");
        PrintNum(id);
        PrintString("\n");
        fileSize = Seek(-1, id);
        Seek(0, id);
        PrintString("Number of characters read: ");
        PrintNum(fileSize);
        PrintString("\n");
        Read(buffer, fileSize, id);
        PrintString("File's content: \n");
        PrintString(buffer);
        PrintString("\n");
        if (Close(id) != -1) {
            PrintString("Closed file successfully\n");
        }
    } 
    else {
        PrintString("Open file failed\n");
    }
    Halt();
}