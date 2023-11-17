#include "syscall.h"
#define maxLength 32

int main()
{
    char buffer[256];
    int socket1, socket2, socket3, socket4;

    PrintString("\nSocket 1:\n");
    socket1 = SocketTCP();
    Connect(socket1, "127.0.0.1", 1234);
    Send(socket1, "This is a message sent from socket 1", 40);
    if (Receive(socket1, buffer, 40) > 0)
    {
        PrintString("Message: ");
        PrintString(buffer);
    }
    Close(socket1);

    PrintString("\nSocket 2:\n");
    socket2 = SocketTCP();
    Connect(socket2, "127.0.0.1", 1234);
    Send(socket2, "This is a message sent from socket 2", 40);
    if (Receive(socket2, buffer, 40) > 0)
    {
        PrintString("Message: ");
        PrintString(buffer);
    }
    Close(socket2);

    PrintString("\nSocket 3:\n");
    socket3 = SocketTCP();
    Connect(socket3, "127.0.0.1", 1234);
    Send(socket3, "This is a message sent from socket 3", 40);
    if (Receive(socket3, buffer, 40) > 0)
    {
        PrintString("Message: ");
        PrintString(buffer);
    }
    Close(socket3);

    PrintString("\nSocket 4:\n");
    socket4 = SocketTCP();
    Connect(socket4, "127.0.0.1", 1234);
    Send(socket4, "This is a message sent from socket 4", 40);
    if (Receive(socket4, buffer, 40) > 0)
    {
        PrintString("Message: ");
        PrintString(buffer);
    }
    Close(socket4);

    Halt();
}