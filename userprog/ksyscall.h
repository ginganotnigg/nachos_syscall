/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"
#include "synchconsole.h"
#include <stdlib.h>


/*
* Function for handling ReadNum and PrintNum
*/

#define LF ((char)10)
#define CR ((char)13)
#define TAB ((char)9)
#define SPACE ((char)' ')
#define INT32_MIN -2147483647-1

/* maximum length of an integer (included the minus sign) */
#define MAX_NUM_LENGTH 11

/* A buffer to read and write number */
char _numberBuffer[MAX_NUM_LENGTH + 2];

char isBlank(char c) { 
    return c == LF || c == CR || c == TAB || c == SPACE;
}

/**
 * Read and store characters in the _numberBuffer until blank
 * or end of file
 *
 * It will read at most MAX_NUM_LENGTH + 1 character
 **/
void readUntilBlank() {
    memset(_numberBuffer, 0, sizeof(_numberBuffer));
    char c = kernel->synchConsoleIn->GetChar();

    if (c == EOF) {
        DEBUG(dbgSys, "Unexpected end of file - number expected");
        return;
    }

    if (isBlank(c)) {
        DEBUG(dbgSys, "Unexpected white-space - number expected");
        return;
    }

    int n = 0;

    while (!(isBlank(c) || c == EOF)) {
        _numberBuffer[n++] = c;
        if (n > MAX_NUM_LENGTH) {
            DEBUG(dbgSys, "Number is too long");
            return;
        }
        c = kernel->synchConsoleIn->GetChar();
    }
}

/**
 * Return true of the interger equals to the
 * interger stored in the string
 **/
bool compareNumAndString(int integer, const char *s) {
    if (integer == 0) return strcmp(s, "0") == 0;

    int len = strlen(s);

    if (integer < 0 && s[0] != '-') return false;

    if (integer < 0) s++, --len, integer = -integer;

    while (integer > 0) {
        int digit = integer % 10;

        if (s[len - 1] - '0' != digit) return false;

        --len;
        integer /= 10;
    }

    return len == 0;
}

void increase_PC() {
    /* set previous programm counter (debugging only)
     * similar to: registers[PrevPCReg] = registers[PCReg];*/
    kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

    /* set programm counter to next instruction
     * similar to: registers[PCReg] = registers[NextPCReg]*/
    kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(NextPCReg));

    /* set next programm counter for brach execution
     * similar to: registers[NextPCReg] = pcAfter;*/
    kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(NextPCReg) + 4);
}

char* User2System(int virtAddr, int limit)
{
	int i; //chi so index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; //can cho chuoi terminal
	if (kernelBuf == NULL)
		return kernelBuf;
		
	memset(kernelBuf, 0, limit + 1);
	
	for (i = 0; i < limit; i++)
	{
		kernel->machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}

int System2User(char* buffer, int virtAddr, int len)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0;
	do{
		oneChar = (int)buffer[i];
		kernel->machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}

/*
* System functions
*/


void SysHalt() {
    kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2) {
    return op1 + op2;
}

int SysReadNum() {
    readUntilBlank();

    int len = strlen(_numberBuffer);
    // Read nothing -> return 0
    if (len == 0) return 0;

    // Check comment below to understand this line of code
    if (strcmp(_numberBuffer, "-2147483648") == 0) return INT32_MIN;

    bool nega = (_numberBuffer[0] == '-');
    int zeros = 0;
    bool is_leading = true;
    int num = 0;
    for (int i = nega; i < len; ++i) {
        char c = _numberBuffer[i];
        if (c == '0' && is_leading)
            ++zeros;
        else
            is_leading = false;
        if (c < '0' || c > '9') {
            DEBUG(dbgSys, "Expected number but " << _numberBuffer << " found");
            return 0;
        }
        num = num * 10 + (c - '0');
    }

    // 00            01 or -0
    if (zeros > 1 || (zeros && (num || nega))) {
        DEBUG(dbgSys, "Expected number but " << _numberBuffer << " found");
        return 0;
    }

    if (nega)
        /**
         * This is why we need to handle -2147483648 individually:
         * 2147483648 is larger than the range of int32
         */
        num = -num;

    // It's safe to return directly if the number is small
    if (len <= MAX_NUM_LENGTH - 2) return num;

    /**
     * We need to make sure that number is equal to the number in the buffer.
     *
     * Ask: Why do we need that?
     * Answer: Because it's impossible to tell whether the number is bigger
     * than INT32_MAX or smaller than INT32_MIN if it has the same length.
     *
     * For example: 3 000 000 000.
     *
     * In that case, that number will cause an overflow. However, C++
     * doens't raise interger overflow, so we need to make sure that the input
     * string and the output number is equal.
     *
     */
    if (compareNumAndString(num, _numberBuffer))
        return num;
    return 0;
}

void SysPrintNum(int num) {
    if (num == 0) return kernel->synchConsoleOut->PutChar('0');

    if (num == INT32_MIN) {
        kernel->synchConsoleOut->PutChar('-');
        for (int i = 0; i < 10; ++i)
            kernel->synchConsoleOut->PutChar("2147483648"[i]);
        return;
    }

    if (num < 0) {
        kernel->synchConsoleOut->PutChar('-');
        num = -num;
    }
    int n = 0;
    while (num) {
        _numberBuffer[n++] = num % 10;
        num /= 10;
    }
    for (int i = n - 1; i >= 0; --i)
        kernel->synchConsoleOut->PutChar(_numberBuffer[i] + '0');
}

char SysReadChar() { 
    return kernel->synchConsoleIn->GetChar(); 
}

void SysPrintChar(char character) {
    kernel->synchConsoleOut->PutChar(character);
}

int SysRandomNum() { 
    return random();
}

char* SysReadString(int length) {
    char* buffer = new char[length + 1];
    char tmp;
    for (int i = 0; i < length; i++) {
        tmp = kernel->synchConsoleIn->GetChar();
        if (tmp == '\0' || tmp == '\n') 
        {
            buffer[i] = '\0';
            break;
        }
        buffer[i] = tmp;
    }
    buffer[length] = '\0';
    return buffer;
}

void SysPrintString(char* buffer, int length) {
    int i = 0;
    while (buffer[i] != '\0') {
        kernel->synchConsoleOut->PutChar(buffer[i]);
        i++;
    }
}

int SysCreateFile(char* fileName) {
    int fileNameLength = strlen(fileName);

    if (fileNameLength == 0) {
        return 0;
    }
    if (fileName == NULL) {
        return 0;
    }
    if (!kernel->fileSystem->Create(fileName)) {
        return 0;
    }

    return 1;
}

int SysOpen(char* fileName, int type) {
    if (type != 0 && type != 1) return -1;

    int id = kernel->fileSystem->Insert(fileName, type);
    if (id == -1) return -1;
    return id;
}

int SysClose(int id) { return kernel->fileSystem->Close(id); }

int SysRead(int virtAddr, int length, OpenFileId fileID) 
{
    //Number of bytes read
    int bytes = -1;
    if (fileID == 0 || fileID == 1) {
        return bytes;
    }

    char *buffer = new char[length + 1];
            
    //Read the content into buffer
    bytes = kernel->fileSystem->openFile[fileID]->Read(buffer, length);
    if (bytes > 0) 
    {
        //buffer[bytes] = '\0';
        System2User(buffer, virtAddr, bytes);
    }

    delete buffer;
    return bytes;
}

int SysWrite(int virtAddr, int length, OpenFileId fileID) 
{
    // Number of bytes read
    int bytes = -1;

    if (fileID == 0 || fileID == 1) {
        return bytes;
    }

    char *buffer = User2System(virtAddr, length);
    OpenFile *filePtr = kernel->fileSystem->openFile[fileID];
    if (buffer == NULL) 
    {
        return -1;
    }
    if (filePtr->GetFileType() == 0) 
    {
        return -1;
    }

    //Write to file 
    bytes = filePtr->Write(buffer, length);
    if (bytes > 0) buffer[bytes] = 0;

    delete[] buffer;
    return bytes;
}

int SysSeek(int pos, int fileID) 
{
    OpenFile *filePtr = kernel->fileSystem->openFile[fileID];

    //Check whether file exist
    if (filePtr == NULL) 
    {
        return -1;
    }
    //Get file length
    if (pos == -1) pos = filePtr->Length();

    if (pos > filePtr->Length() || pos < 0) 
    {
        return -1;
    }
    filePtr->Seek(pos);
    return pos;
}

int SysRemove(char* fileName) {
    int result;
    
    //Check whether file is opening
    OpenFile *file = kernel->fileSystem->GetFileDes(fileName);
    if (file != NULL) {
        delete[] fileName;
        return -1;
    }
    result = kernel->fileSystem->Remove(fileName);

    delete[] fileName;
    return result;
}



#endif /* ! __USERPROG_KSYSCALL_H__ */
