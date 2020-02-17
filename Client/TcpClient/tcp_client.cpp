#include "tcp_client.h"
#include <iostream>
using namespace std;
TcpClient::TcpClient()
{
    // Tcp客户端地址
    if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            ErrorHandling("WSAStartup() error!");
    hSocket=socket(PF_INET, SOCK_STREAM, 0);
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=inet_addr("192.168.0.110");
    servAddr.sin_port=htons(atoi("12345"));
}

bool TcpClient::Login(char* account,char* password) {
    if(SocketConnect() == false) {
        // 服务器反馈了L+错误信息
          error_info = "Tcp client connect false";
          SocketClose();
          return false;
    }
    // 发送信息给Tcp服务器 L+账号+密码
    char write_buf[SendBufferSize];
    write_buf[0] = 'L';
    strcpy(&write_buf[1], account);
    strcat(&write_buf[10], password);
    //send();
    SendMessage(write_buf);

    //接受客户端返回消息，这里我们应该设置超时反馈 目前还没有！！！！！！！！！！！，所以如果服务器没开就连接会卡死
    char read_buf[RecvBufferSize];
    ReceiveMessage(read_buf);
    // 根据反馈消息做出判断
    if(read_buf[0] == 'L') {
        switch(read_buf[1]) {
            case 'Y':
                SocketClose();
                return true;
            case 'N':
                // 服务器反馈了L+错误信息
                error_info = read_buf;
                break;
            default:
                error_info = "服务器反馈L+未知信息";
                break;
        }
    } else {
        error_info = "服务器反馈信息没有L开头";
    }
    SocketClose();
    return false;
}
void TcpClient::SendMessage(const char* message) {
    char write_buf[SendBufferSize];
    strcpy(write_buf, message);
    int send_result = send(hSocket, write_buf, SendBufferSize,0);
    if(send_result == -1) {
         TcpClient::ErrorHandling("send() error!");
    }
}
void TcpClient::ReceiveMessage(char* read_buf) {
    int strLen;
    strLen = recv(hSocket, read_buf, RecvBufferSize, 0);
    if(strLen==-1) {
        TcpClient::ErrorHandling("recv() error!");
    }
}
void TcpClient::ErrorHandling(const char *info) {
    cout << info << endl;
}
bool TcpClient::SocketConnect() {
    if(connect(hSocket, (SOCKADDR*)&servAddr, sizeof(servAddr))==SOCKET_ERROR)
        return false;
    else
        return true;
}
void TcpClient::SocketClose() {
    closesocket(hSocket);
    WSACleanup();
}
