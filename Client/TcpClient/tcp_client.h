#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include<string>
#include<QApplication>
#ifndef TCPCLIENT_H
#define TCPCLIENT_H
const int SendBufferSize = 1024;
const int RecvBufferSize = 1024;
class TcpClient
{
public:
    TcpClient();
    bool Login(char* account,char* password);
private:
    void SendMessage(const char* message);
    void ReceiveMessage(char* read_buf);
    void ErrorHandling(const char* info);
    bool SocketConnect();
    void SocketClose();
public:
    std::string error_info; // 错误信息
private:
    WSADATA wsaData;
    SOCKET hSocket;
    SOCKADDR_IN servAddr;
};

#endif // TCPCLIENT_H
