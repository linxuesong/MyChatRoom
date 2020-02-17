#include "udpclient.h"

UdpClient::UdpClient() {
    // 初始化winsock库
    if(WSAStartup(MAKEWORD(2, 2), &_wsaData) != 0) {
        ErrorHandling("WSAStartUp error");
    }
    // 初始化socket
    _udpSocket = socket(PF_INET, SOCK_DGRAM, 0);
    // 服务器地址变量赋值
    _servAddr.sin_family = AF_INET;
    _servAddr.sin_addr.s_addr = inet_addr("192.168.0.110");
    _servAddr.sin_port = htons(atoi("12345"));
}
UdpClient::~UdpClient() {


}
void UdpClient::SendMessage(const char* message) {
    char writeBuffer[WriteBufferSize];
    strcpy(writeBuffer, message);
    sendto(_udpSocket, writeBuffer, WriteBufferSize, 0, (sockaddr*)&_servAddr, sizeof(_servAddr));
}
void UdpClient::ReceiveMessage(char* read_buf) {
    sockaddr_in my_serv_addr;
    int serv_addr_len = sizeof(sockaddr_in);
    int recvlen = recvfrom(_udpSocket, read_buf, ReadBufferSize, 0, (sockaddr*) &my_serv_addr, &serv_addr_len);
    if(recvlen == -1) {
        int errorWord = WSAGetLastError();
        QMessageBox::warning(nullptr, (QString)errorWord,"recv error");
    }
}
void UdpClient::ErrorHandling(const char* info) {
    error_info = info;
    QMessageBox::warning(nullptr, "UDPCLient Error", QString::fromStdString(error_info));
}
