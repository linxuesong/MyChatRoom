#include<string>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include<string>
#include<QMessageBox>
#ifndef UDPCLIENT_H
#define UDPCLIENT_H
const int WriteBufferSize = 1024;
const int ReadBufferSize = 1024;
class UdpClient
{
public:
    UdpClient();
    void SendMessage(const char* message);
    void ReceiveMessage(char* read_buf);
    void ErrorHandling(const char* info);
    ~UdpClient();
public:
    std::string error_info; // 错误信息
private:
    WSADATA _wsaData;
    SOCKET _udpSocket;
    SOCKADDR_IN _servAddr;
};

#endif // UDPCLIENT_H
