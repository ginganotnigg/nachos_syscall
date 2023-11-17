#include "syscall.h"

int main() {
    char content[1024];
    int src, dst;
    int fileLength;
    char srcFile[256];
    char dstFile[256];

    //PrintString("Enter source filename's length: ");
    //strLength = ReadNum();
    PrintString("Enter source filename: ");
    ReadString(srcFile, 32);

    PrintString("Enter destination filename: ");
    ReadString(dstFile, 32);

    src = Open(srcFile, 1);
    if (src == -1) 
    {
        PrintString("Non-exist source file!!\n");
    }
    else {
        fileLength = Seek(-1, src);
        Seek(0, src);
        PrintString("Number of characters read: ");
        PrintNum(fileLength);
        PrintString("\n");
        Read(content, fileLength, src);
        Close(src);

        dst = Open(dstFile, 1);
        if (dst == -1) {
            if (Create(dstFile) == -1) 
            {
                PrintString("Cannot create destination file!!\n");
                Halt();
            }
            else {
                PrintString("New file created!\n");
                dst = Open(dstFile, 1);
            }
        }
        Write(content, fileLength, dst);
        PrintString("Successfully copy file!!\n");
        Close(dst);
    }
    Halt();
}