// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"

#define MAX_STRLEN 255
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------

/* 
* Handle syscall
*/

void handle_SC_Halt() {
    DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
    SysHalt();
    ASSERTNOTREACHED();
}

void handle_SC_Add() {
    DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + "
                         << kernel->machine->ReadRegister(5) << "\n");

    /* Process SysAdd Systemcall*/
    int result;
    result = SysAdd(
        /* int op1 */ (int)kernel->machine->ReadRegister(4),
        /* int op2 */ (int)kernel->machine->ReadRegister(5));

    DEBUG(dbgSys, "Add returning with " << result << "\n");
    /* Prepare Result */
    kernel->machine->WriteRegister(2, (int)result);

    return increase_PC();
}

void handle_SC_ReadNum() {
    int result = SysReadNum();
    kernel->machine->WriteRegister(2, result);
    return increase_PC();
}

void handle_SC_PrintNum() {
    int character = kernel->machine->ReadRegister(4);
    SysPrintNum(character);
    return increase_PC();
}

void handle_SC_ReadChar() {
    char result = SysReadChar();
    kernel->machine->WriteRegister(2, (int)result);
    return increase_PC();
}

void handle_SC_PrintChar() {
    char character = (char)kernel->machine->ReadRegister(4);
    SysPrintChar(character);
    return increase_PC();
}

void handle_SC_RandomNum() {
    int result;
    result = SysRandomNum();
    kernel->machine->WriteRegister(2, result);
    return increase_PC();
}

void handle_SC_ReadString() {
    int memPtr = kernel->machine->ReadRegister(4);  // read address of C-string
    int length = kernel->machine->ReadRegister(5);  // read length of C-string
    if (length > MAX_STRLEN || length < 1) {  // avoid allocating large memory
        DEBUG(dbgSys, "Invalid string length\n");
        return increase_PC();
    }
    char* buffer = SysReadString(length + 1);
    System2User(buffer, memPtr, length + 1);
    delete buffer;
    return increase_PC();
}

void handle_SC_PrintString() {
    int memPtr = kernel->machine->ReadRegister(4);  // read address of C-string
    char* buffer = User2System(memPtr, MAX_STRLEN);

    SysPrintString(buffer, strlen(buffer));
    delete buffer;
    return increase_PC();
}

void handle_SC_Create() {
    int virtAddr = kernel->machine->ReadRegister(4);
    char* fileName = User2System(virtAddr, MAX_STRLEN);

    kernel->machine->WriteRegister(2, SysCreateFile(fileName) - 1);
    
    delete fileName;
    return increase_PC();
}

void handle_SC_Open() {
    int virtAddr = kernel->machine->ReadRegister(4);
    char* fileName = User2System(virtAddr, MAX_STRLEN);
    int type = kernel->machine->ReadRegister(5);

    //Check whether there is error on reading filename
    if (fileName == NULL || strlen(fileName) == 0 || 
        !OpenForRead(fileName, FALSE) || !OpenForReadWrite(fileName, FALSE)) 
    {
        kernel->machine->WriteRegister(2, -1);
        if (fileName) delete[] fileName;
        return increase_PC();
    }

    kernel->machine->WriteRegister(2, SysOpen(fileName, type));

    delete[] fileName;
    return increase_PC();
}

void handle_SC_Close() {
    int id = kernel->machine->ReadRegister(4);
    if (id >= 0 && id < MAX_FILES){
        kernel->machine->WriteRegister(2, SysClose(id) - 1);
    }
    else {
        kernel->machine->WriteRegister(2, -1);
    }

    return increase_PC();
}

void handle_SC_Read() {
    int virtAddr = kernel->machine->ReadRegister(4);
    int length = kernel->machine->ReadRegister(5);
    OpenFileId fileID = kernel->machine->ReadRegister(6);
    if (length < 0 || fileID < 0 || fileID >= MAX_FILES) 
    {
        kernel->machine->WriteRegister(2, -1);
    }
    else 
    {
        int n = SysRead(virtAddr, length, fileID);
        kernel->machine->WriteRegister(2, n);
    }
    return increase_PC();
}

void handle_SC_Write() {
    int virtAddr = kernel->machine->ReadRegister(4);
    int length = kernel->machine->ReadRegister(5);
    OpenFileId fileID = kernel->machine->ReadRegister(6);

    if (length < 1 || fileID < 1 || fileID >= MAX_FILES) 
    {
        kernel->machine->WriteRegister(2, -1);
    }
    else 
    {
        kernel->machine->WriteRegister(2, SysWrite(virtAddr, length, fileID));
    }
    return increase_PC();
}

void handle_SC_Seek() {
    int pos = kernel->machine->ReadRegister(4);
    int fileID = kernel->machine->ReadRegister(5);
    if (fileID < 2 || fileID >= MAX_FILES) 
    {
        kernel->machine->WriteRegister(2, -1);
    }
    else 
    {
        kernel->machine->WriteRegister(2, SysSeek(pos, fileID));
    }
    return increase_PC();
}

void handle_SC_Remove() {
    int virtAddr = kernel->machine->ReadRegister(4);
    char *fileName = User2System(virtAddr, MAX_STRLEN);
    if (fileName == NULL || strlen(fileName) == 0) 
    {
        kernel->machine->WriteRegister(2, -1);
        if (fileName) {
            delete[] fileName;
        }
    }
    else {
        kernel->machine->WriteRegister(2, SysRemove(fileName));
    }

    return increase_PC();
}

/* 
* Handle exception
*/

void ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    //DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
        case SyscallException:
			switch(type) {
                case SC_Halt:
                    return handle_SC_Halt();
				case SC_Add:
                    return handle_SC_Add();
                // Helper system call
                case SC_ReadNum:
                    return handle_SC_ReadNum();
                case SC_PrintNum:
                    return handle_SC_PrintNum();
                case SC_ReadChar:
                    return handle_SC_ReadChar();
                case SC_PrintChar:
                    return handle_SC_PrintChar();
                case SC_RandomNum:
                    return handle_SC_RandomNum();
                case SC_ReadString:
                    return handle_SC_ReadString();
                case SC_PrintString:
                    return handle_SC_PrintString();
                // File system call
                case SC_Create:
                    return handle_SC_Create();
                case SC_Open:
                    return handle_SC_Open();
                case SC_Close:
                    return handle_SC_Close();
                case SC_Read:
                    return handle_SC_Read();
                case SC_Write:
                    return handle_SC_Write();
                case SC_Seek:
                    return handle_SC_Seek();
                case SC_Remove:
                    return handle_SC_Remove();
				default:
                    cerr << "Unexpected system call " << type << "\n";
                    break;
			}
			break;
		default:
			cerr << "Unexpected user mode exception " << (int)which << "\n";
		    break;
    }
    ASSERTNOTREACHED();
}
