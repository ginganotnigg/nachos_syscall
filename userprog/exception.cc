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
* Other functions
*/

void move_program_counter() {
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

char* stringUser_Sys(int addr, int convert_length = -1) {
    int length = 0;
    bool stop = false;
    char* str;

    do {
        int oneChar;
        kernel->machine->ReadMem(addr + length, 1, &oneChar);
        length++;
        // if convert_length == -1, we use '\0' to terminate the process
        // otherwise, we use convert_length to terminate the process
        stop = ((oneChar == '\0' && convert_length == -1) ||
                length == convert_length);
    } while (!stop);

    str = new char[length];
    for (int i = 0; i < length; i++) {
        int oneChar;
        kernel->machine->ReadMem(addr + i, 1, &oneChar);  // copy characters to kernel space
        str[i] = (unsigned char)oneChar;
    }
    return str;
}

void stringSys_User(char* str, int addr, int convert_length = -1) {
    int length = (convert_length == -1 ? strlen(str) : convert_length);
    for (int i = 0; i < length; i++) {
        kernel->machine->WriteMem(addr + i, 1, str[i]);  // copy characters to user space
    }
    kernel->machine->WriteMem(addr + length, 1, '\0');
}

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

    return move_program_counter();
}

void handle_SC_ReadNum() {
    int result = SysReadNum();
    kernel->machine->WriteRegister(2, result);
    return move_program_counter();
}

void handle_SC_PrintNum() {
    int character = kernel->machine->ReadRegister(4);
    SysPrintNum(character);
    return move_program_counter();
}

void handle_SC_ReadChar() {
    char result = SysReadChar();
    kernel->machine->WriteRegister(2, (int)result);
    return move_program_counter();
}

void handle_SC_PrintChar() {
    char character = (char)kernel->machine->ReadRegister(4);
    SysPrintChar(character);
    return move_program_counter();
}

void handle_SC_RandomNum() {
    int result;
    result = SysRandomNum();
    kernel->machine->WriteRegister(2, result);
    return move_program_counter();
}

#define MAX_READ_STRING_LENGTH 255
void handle_SC_ReadString() {
    int memPtr = kernel->machine->ReadRegister(4);  // read address of C-string
    int length = kernel->machine->ReadRegister(5);  // read length of C-string
    if (length > MAX_READ_STRING_LENGTH) {  // avoid allocating large memory
        DEBUG(dbgSys, "String length exceeds " << MAX_READ_STRING_LENGTH);
        SysHalt();
    }
    char* buffer = SysReadString(length);
    stringSys_User(buffer, memPtr);
    delete[] buffer;
    return move_program_counter();
}

void handle_SC_PrintString() {
    int memPtr = kernel->machine->ReadRegister(4);  // read address of C-string
    char* buffer = stringUser_Sys(memPtr);

    SysPrintString(buffer, strlen(buffer));
    delete[] buffer;
    return move_program_counter();
}

void handle_SC_Create() {
    int virtAddr = kernel->machine->ReadRegister(4);
    char* fileName = stringUser_Sys(virtAddr);

    if (SysCreateFile(fileName)) {
        kernel->machine->WriteRegister(2, 0);
        DEBUG(dbgSys, "Create file successfully\n");
    }
    else {
        kernel->machine->WriteRegister(2, -1);
        DEBUG(dbgSys, "Failed to create file\n");
    }
    delete[] fileName;
    return move_program_counter();
}

/* 
* Handle exception
*/

void ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
		case SyscallException:
			switch(type) {
				case SC_Add:
                    return handle_SC_Add();
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
                case SC_Create:
                    return handle_SC_Create();
                case SC_Halt:
                    return handle_SC_Halt();
				default:
                    cerr << "Unexpected system call " << type << "\n";
                    break;
			}
			break;
		default:
			cerr << "Unexpected user mode exception" << (int)which << "\n";
		    break;
    }
    ASSERTNOTREACHED();
}
