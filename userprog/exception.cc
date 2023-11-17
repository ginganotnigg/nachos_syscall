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
#include<iostream>
#include "filesys.h"
#define MaxFileLength 32
#define MAX_LENGTH_STRING 32
#define MAX_LENGTH_IP 46
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
/* 
* Handle syscall
*/

void handle_SC_Halt() {
    DEBUG(dbgSys, "\nShutdown, initiated by user program.\n");
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
    int virAddr;
    int length;
    int inputLength;
    char *strName;
    char c;

    virAddr = kernel->machine->ReadRegister(4);
    length = kernel->machine->ReadRegister(5);
    strName =new char[length];
    inputLength = 0;
    while ((c = kernel->ReadChar()) != '\n') {
        strName[inputLength] = c;
        inputLength++;
    }
    strName[inputLength] = '\0';

    System2User(virAddr, inputLength, strName);
    return move_program_counter();
}

void handle_SC_PrintString() {
    int virtAddr;
    char *buffer;
    virtAddr =kernel->machine->ReadRegister(4);
    buffer = User2System(virtAddr, 255);
    int length = 0;
    while (buffer[length] != 0) length++;
    kernel->PrintBuffer(buffer,length);
    delete buffer;
    return move_program_counter();
}

void handle_SC_Create() {
    int virtAddr;
    char *filename;
    virtAddr = kernel->machine->ReadRegister(4);
    filename = User2System(virtAddr, MaxFileLength + 1);
    if (strlen(filename) == 0 || filename == NULL)
    {
        cout<<"- Filename invalid\n";
        kernel->machine->WriteRegister(2, -1);
        return move_program_counter();
    }
    if (!kernel->fileSystem->Create(filename))
    {
        cout<<"- Cannot create file\n";
        kernel->machine->WriteRegister(2, -1);
        delete filename;
        return move_program_counter();
    }
    cout<<"- Create file " << filename << " successfully\n";
    kernel->machine->WriteRegister(2, 0);
    delete filename;
    return move_program_counter();
}

void handle_SC_Open() {
    int virtAddr=kernel->machine->ReadRegister(4);
    char *filename;
    int type=kernel->machine->ReadRegister(5);
    filename=User2System(virtAddr,MaxFileLength+1);
    int temp=kernel->fileSystem->isFull();
    if(temp==-1)
    {
        cout<<"- Over 20 files/sockets are opened\n";
        kernel->machine->WriteRegister(2,-1);
        delete [] filename;
        return move_program_counter();
    }
    if (strlen(filename) == 0 || filename == NULL)
    {
        cout<<"- Filename invalid\n";
        kernel->machine->WriteRegister(2, -1); //Return -1 vao thanh ghi R2
        return move_program_counter();
    }

    if(type==1 || type==0)
    {
        if((kernel->fileSystem->openfile[temp]=kernel->fileSystem->Open(filename,type))!=NULL)
        {
            cout<<"- Open file " << filename << " successfully, file id: " << temp << "\n";
            kernel->fileSystem->filename[temp]=filename;
            kernel->machine->WriteRegister(2,temp);
        }
        else
        {
            cout<<"- Cannot open file\n";
            kernel->machine->WriteRegister(2,-1);
        }
    }
    else
    {
        cout<<"- Cannot open file\n";
        kernel->machine->WriteRegister(2,-1);
    }
    
    delete[] filename;
    return move_program_counter();

}

void handle_SC_Read() {
    int virtAddr = kernel->machine->ReadRegister(4); 
    int charcount = kernel->machine->ReadRegister(5); 
    int id = kernel->machine->ReadRegister(6); 
    int OldPos;
    int NewPos;
    char *buffer;
    int temp;
    if (id < 0 || id >= 20 || kernel->fileSystem->openfile[id] == NULL)
    {
        cout << "- Cannot open file\n";
        kernel->machine->WriteRegister(2, -1);
        return move_program_counter();
    }
    
    OldPos = kernel->fileSystem->openfile[id]->GetCurrentPos(); 
    buffer = User2System(virtAddr, charcount);                  
    if (id==0)
    {
        int size = 0;
        for (int i = 0; i < charcount; ++i)
        {
            size = size + 1;
            buffer[i] = kernel->ReadChar(); 
            if (buffer[i] == '\n')
            {
                buffer[i + 1] = '\0';
                break;
            }
        }
        buffer[size] = '\0';
        System2User(virtAddr, size, buffer);     
        kernel->machine->WriteRegister(2, size); 
        return move_program_counter();
    }
    
    OpenFile* file = kernel->fileSystem->openfile[id];
    if (file->type == 0 || file->type == 1) 
    {
        if (file->Read(buffer, charcount) > 0)
        {
            NewPos = file->GetCurrentPos();
            System2User(virtAddr, NewPos - OldPos, buffer); 
            kernel->machine->WriteRegister(2, NewPos - OldPos);
        }
        else
        {
            kernel->machine->WriteRegister(2, -1);
        }
    } 
    else if (file->type == 2 && file->isConnect) 
    {
        temp = file->Read(buffer, charcount);
        if (temp > 0) 
        {
            System2User(virtAddr, temp, buffer);
            kernel->machine->WriteRegister(2, temp);
        } 
        else 
        {
            kernel->machine->WriteRegister(2, -1);
        }
    } 
    else 
    {
        kernel->machine->WriteRegister(2, -1);
    }
    return move_program_counter();
}

void handle_SC_Write() {
    int virtAddr = kernel->machine->ReadRegister(4); 
    int charcount = kernel->machine->ReadRegister(5); 
    int id = kernel->machine->ReadRegister(6); 
    int OldPos;
    int NewPos;
    int temp;
    char *buffer;
    if (id < 0 || id >= 20 || kernel->fileSystem->openfile[id] == NULL)
    {
        cout << "- Cannot open file\n";
        kernel->machine->WriteRegister(2, -1);
        return move_program_counter();
    }
    if (id==0 || kernel->fileSystem->openfile[id]->type == 1)
    {
        cout <<"- Cannot write in read-only file\n";
        kernel->machine->WriteRegister(2, -1);
        return move_program_counter();
    }
    OldPos = kernel->fileSystem->openfile[id]->GetCurrentPos();
    buffer = User2System(virtAddr, charcount);      
    if (id ==1)
    {
        int len = 0;
        while (buffer[len] != 0)
        {
            kernel->putChar(buffer[len]);
            len++;
            if (len == 255) 
            {
                delete[] buffer;
                virtAddr = virtAddr + 255;
                buffer = User2System(virtAddr, 255);
                len = 0;
            }
        }
        kernel->machine->WriteRegister(2, len - 1);
        delete buffer;
        return move_program_counter();
    }
    OpenFile* file = kernel->fileSystem->openfile[id];
    if (file->type == 0 )
    {
        if ((file->Write(buffer, charcount)) > 0) 
        {
            NewPos = file->GetCurrentPos();
            kernel->machine->WriteRegister(2, NewPos - OldPos);
        } 
        else 
        {
            kernel->machine->WriteRegister(2, -1);
        }
    } 
    else if (file->type == 2 && file->isConnect == true) 
    {
        temp = file->Write(buffer, charcount);
        if (temp > 0)
        {
            kernel->machine->WriteRegister(2, temp);
        } 
        else 
        {
            kernel->machine->WriteRegister(2, -1);
        }
    } 
    else 
    {
        kernel->machine->WriteRegister(2, -1);
    }
    delete buffer;
    return move_program_counter();
}

void handle_SC_Seek() {
    int pos=kernel->machine->ReadRegister(4);
    int id=kernel->machine->ReadRegister(5);
    OpenFile* file=kernel->fileSystem->openfile[id];
    if(id==0 || id==1 || id>=20 || file==NULL ||  file->type == 2 || pos>file->Length())
    {
        cout<<"- Cannot seek file\n";
        kernel->machine->WriteRegister(2,-1);
        return move_program_counter();
    }
    if(pos==-1)
    {
        pos=file->Length();
    }
    file->Seek(pos);
    kernel->machine->WriteRegister(2,pos);
    return move_program_counter();
}

void handle_SC_Remove() {
    int virtAddr=kernel->machine->ReadRegister(4);
    char *filename=User2System(virtAddr,MaxFileLength+1);
    if (strlen(filename) == 0 || filename == NULL)
    {
        cout << "- Filename invalid\n";
        kernel->machine->WriteRegister(2, -1); //Return -1 vao thanh ghi R2
        delete filename;
        return move_program_counter();
    }
    bool isOpen=false;
    for(int i=0; i<20; i++)
    {
        if(filename== kernel->fileSystem->filename[i])
        {
            isOpen=true;
            break;
        }
    }
    if(isOpen)
    {
        cout<<"- File is opened, cannot remove\n";
        kernel->machine->WriteRegister(2,-1);
        delete[] filename;
        return move_program_counter();
    }
    if(kernel->fileSystem->Remove(filename))
    {
        cout<<"- Remove successfully\n";
        kernel->machine->WriteRegister(2,0);
    }
    else 
    {
        cout<<"- Cannot remove\n";
        kernel->machine->WriteRegister(2,-1);
    }
    delete filename;
    return move_program_counter();
}


void handle_SC_SocketTCP(){
    int temp=kernel->fileSystem->isFull();
    if(temp==-1)
    {
        cout<<"- Over 20 files/sockets are opened\n";
        kernel->machine->WriteRegister(2,-1);
        return move_program_counter();
    }
    if((kernel->fileSystem->openfile[temp]=kernel->fileSystem->socketTCP())!=NULL)
    {
        kernel->fileSystem->filename[temp]= "socket";
        cout<<"- Open socket successfully\n";
        kernel->machine->WriteRegister(2,temp);
    }
    else
    {
        cout<<"- Cannot open socket\n";
        kernel->machine->WriteRegister(2,-1);
    }
    return move_program_counter();
}

void handle_SC_Close(){
    int id=kernel->machine->ReadRegister(4); 
    if(id <2 || id >=20 || kernel->fileSystem->openfile[id]==NULL)
    {
        cout<<"- Cannot close\n"<<endl;
        kernel->machine->WriteRegister(2, -1);
        return move_program_counter();
    } 
    kernel->machine->WriteRegister(2, 0);
    cout<<"- Close " << kernel->fileSystem->filename[id] << " successfully\n";
    kernel->fileSystem->filename[id]="";

    delete kernel->fileSystem->openfile[id];
    kernel->fileSystem->openfile[id]=NULL;
    
    return move_program_counter();
}

void handle_SC_Connect(){
    int socketid = kernel->machine->ReadRegister(4); 
    int virtAddr = kernel->machine->ReadRegister(5); 
    int port = kernel->machine->ReadRegister(6); 
    char* ip;
    if (socketid < 2 || socketid >=20 || kernel->fileSystem->openfile[socketid] == NULL)
    {
        cout<<"- Cannot connect to server\n";
        kernel->machine->WriteRegister(2, -1);
        return move_program_counter();
    }
    ip = User2System(virtAddr, MAX_LENGTH_IP); 
    if(kernel->fileSystem->openfile[socketid]->Connect(ip, port)<0)
    {
        cout<<"- Cannot connect to server\n";
        kernel->machine->WriteRegister(2, -1);
    } 
    else 
    {
        cout<<"- Connect to server successfully\n";
        kernel->machine->WriteRegister(2, 0);
    }
    delete [] ip;
    return move_program_counter();
}

void handle_SC_Send(){
    int socketid = kernel->machine->ReadRegister(4);
    int raw_buf = kernel->machine->ReadRegister(5);
    int charcount = kernel->machine->ReadRegister(6);
    int temp;
    char* buffer;
    if (socketid < 2 || socketid >=20 || kernel->fileSystem->openfile[socketid] == NULL)
    {
        cout<<"- Cannot connect to server\n";
        kernel->machine->WriteRegister(2, -1);
        return move_program_counter();
    }
    buffer = User2System(raw_buf, charcount);
    if (kernel->fileSystem->openfile[socketid]->type == 2 && kernel->fileSystem->openfile[socketid]->isConnect == true)
    {
        temp = kernel->fileSystem->openfile[socketid]->Write(buffer, charcount);
        if (temp > 0)
        {
            cout<<"- Send message successfully\n";
            kernel->machine->WriteRegister(2, temp);
        } 
        else 
        {
            kernel->machine->WriteRegister(2, -1);
        }
    } 
    else 
    {
        kernel->machine->WriteRegister(2, -1);
    }
    delete buffer;
    return move_program_counter();
}

void handle_SC_Receive(){
    int socketid = kernel->machine->ReadRegister(4);
    int raw_buf = kernel->machine->ReadRegister(5);
    int charcount = kernel->machine->ReadRegister(6);
    char* buffer;
    int receivesize;
    if (socketid < 2 || socketid >=20 || kernel->fileSystem->openfile[socketid] == NULL)
    {
        cout<<"- Cannot connect to server\n";
        kernel->machine->WriteRegister(2, -1);
        return move_program_counter();
    }
    buffer = User2System(raw_buf, charcount); 
    if (kernel->fileSystem->openfile[socketid]->type == 2 && kernel->fileSystem->openfile[socketid]->isConnect == true) 
    {
        receivesize = kernel->fileSystem->openfile[socketid]->Read(buffer, charcount);
        if (receivesize > 0 )
        {
            System2User(raw_buf, receivesize, buffer);
            cout<<"- Receive from server successfully\n";
            kernel->machine->WriteRegister(2, receivesize);
        } 
        else 
        {
            kernel->machine->WriteRegister(2, -1);
        }
    } 
    else 
    {
        kernel->machine->WriteRegister(2, -1);
    }
    delete buffer;
    return move_program_counter();
}
/* 
* Handle exception
*/

void ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);
    //cerr << "Receive exception type: " << type << "\n";

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
                case SC_Open:
                    return handle_SC_Open();
                case SC_ReadString:
                    return handle_SC_ReadString();
                case SC_PrintString:
                    return handle_SC_PrintString();
                case SC_Create:
                    return handle_SC_Create();
                case SC_Seek:
                    return handle_SC_Seek();
                case SC_Read:
                    return handle_SC_Read();
                case SC_Write:
                    return handle_SC_Write();
                case SC_Remove:
                    return handle_SC_Remove();
                case SC_Halt:
                    return handle_SC_Halt();
                case SC_SocketTCP:
                    return handle_SC_SocketTCP();
                case SC_Close:
                    return handle_SC_Close();
                case SC_Connect:
                    return handle_SC_Connect();
                case SC_Send:
                    return handle_SC_Send();
                case SC_Receive:
                    return handle_SC_Receive();
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