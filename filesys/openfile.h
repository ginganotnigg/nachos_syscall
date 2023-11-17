// openfile.h 
//	Data structures for opening, closing, reading and writing to 
//	individual files.  The operations supported are similar to
//	the UNIX ones -- type 'man open' to the UNIX prompt.
//
//	There are two implementations.  One is a "STUB" that directly
//	turns the file operations into the underlying UNIX operations.
//	(cf. comment in filesys.h).
//
//	The other is the "real" implementation, that turns these
//	operations into read and write disk sector requests. 
//	In this baseline implementation of the file system, we don't 
//	worry about concurrent accesses to the file system
//	by different threads.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef OPENFILE_H
#define OPENFILE_H

#include "copyright.h"
#include "utility.h"
#include "sysdep.h"
#include "sys/socket.h"
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>
#include <unistd.h>

#ifdef FILESYS_STUB			// Temporarily implement calls to 
					// Nachos file system as calls to UNIX!
					// See definitions listed under #else
class OpenFile {
  public:
	int type;
	bool isConnect;

    OpenFile(int f) { file = f; currentOffset = 0; isConnect = true;}
	OpenFile(int f, int mode) { file = f; currentOffset = 0; type = mode; isConnect = false; }
    virtual ~OpenFile() {
		if (type != 2) 
		{
			Close(file); 
		} 
		else if (type ==2) 
		{
			CloseSocket(file);
		}
	}

    int ReadAt(char *into, int numBytes, int position) { 
    		Lseek(file, position, 0); 
		return ReadPartial(file, into, numBytes); 
		}	
    int WriteAt(char *from, int numBytes, int position) { 
    		Lseek(file, position, 0); 
		WriteFile(file, from, numBytes); 
		return numBytes;
		}	
    virtual int Read(char *into, int numBytes) {
		int numRead = ReadAt(into, numBytes, currentOffset); 
		currentOffset += numRead;
		return numRead;
    }
    virtual int Write(char *from, int numBytes) {
		int numWritten = WriteAt(from, numBytes, currentOffset); 
		currentOffset += numWritten;
		return numWritten;
	}
	int Seek(int pos)
	{
		Lseek(file, pos, 0); 
		currentOffset=Tell(file);
		return currentOffset;
	}
	int GetCurrentPos()
	{
		currentOffset=Tell(file);
		return currentOffset;
	}
    int Length() { 
		Lseek(file, 0, 2); 
		return Tell(file);
	}
	int Connect(char *ip, int port) {
		if (type!= 2) {
			return -1;
		}
		struct sockaddr_in server;
		server.sin_family = AF_INET;
  		server.sin_addr.s_addr = inet_addr(ip);
  		server.sin_port = htons(port);
		int len = sizeof(server);
		if (connect(file, (struct sockaddr *)&server, len) < 0) {
			isConnect = false;
			return -1;
		}
		isConnect = true;
		return 0;
	}
	int returnFile_SocketID() {
		return file;
	}
  private:
    int file;
    int currentOffset;
};
class Socket : public OpenFile {
public:
	Socket(int f) : OpenFile(f,2)  {
	}
	int Write(char *from, int numBytes) {
		if (isConnect == false) return -1;
		int num = write(returnFile_SocketID() , from, numBytes);
		return num;
	}
	int Read(char *into, int numBytes) {
		if (isConnect == false) return -1;
		return read(returnFile_SocketID(), into, numBytes);
	}
	~Socket(){}
};

#else // FILESYS
class FileHeader;

class OpenFile {
  public:
    OpenFile(int sector);		// Open a file whose header is located
					// at "sector" on the disk
    ~OpenFile();			// Close the file

    void Seek(int position); 		// Set the position from which to 
					// start reading/writing -- UNIX lseek

    int Read(char *into, int numBytes); // Read/write bytes from the file,
					// starting at the implicit position.
					// Return the # actually read/written,
					// and increment position in file.
    int Write(char *from, int numBytes);

    int ReadAt(char *into, int numBytes, int position);
    					// Read/write bytes from the file,
					// bypassing the implicit position.
    int WriteAt(char *from, int numBytes, int position);

    int Length(); 			// Return the number of bytes in the
					// file (this interface is simpler 
					// than the UNIX idiom -- lseek to 
					// end of file, tell, lseek back 
    
  private:
    FileHeader *hdr;			// Header for this file 
    int seekPosition;			// Current position within the file
};

#endif // FILESYS



#endif // OPENFILE_H