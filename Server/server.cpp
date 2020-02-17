#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>


#include<sys/epoll.h>
#include<unistd.h>

#include<pthread.h>

#include<stdlib.h> // malloc函数
#include<cstring>
#include<map>

#include<error.h>

#include<iostream>
using namespace std;
// 需要变量保存所有的在线联系人 这里使用map<账号，对应客户地址>来保存
map<string, sockaddr_in> OnLineUsers;




// TCP的缓冲大小
const int READ_BUFFER_SIZE = 128;
const int Write_Buffer_SIZE = 128;

// 错误处理
void DisplayError(string error) {
    cout << "tcp server error:" << error << endl;
    exit(1);
}

// udp处理流程   

//　大小端转换ｓｏｃｋａｄｄｒ_in显示地址
void DisplayOneAddr(sockaddr_in client_addr) {
    char* clnt_ip = inet_ntoa(client_addr.sin_addr);
    int clnt_port = ntohs(client_addr.sin_port);
    cout << "ip地址：" << client_addr.sin_addr.s_addr <<"  端口："<< client_addr.sin_port << endl;
}

    // 打印所有在线用户的账号
void DisplayAllOnlineUser() {
    cout << "在线账号" << endl;
    cout << "----------------------------" << endl;
    for(auto it = OnLineUsers.begin(); it != OnLineUsers.end(); it++) {    
        cout << "账号" << it->first << "   ";
        DisplayOneAddr(it -> second);
    }
    if(OnLineUsers.empty()){
        cout << "当前无人登录" << endl;
    }
    cout << "----------------------------" << endl;
}




void UdpSendMessage(int sock, char* buf, int buf_len, int flag, struct sockaddr* addr, socklen_t addr_len) {
    // 这里有个addr_len突然变为0的bug，所以sendto就用sizeof吧
    int send_result = sendto(sock, buf, buf_len, flag, (sockaddr*)addr, sizeof(sockaddr_in));
    if(send_result == -1) {
	perror("");
        cout << "udpSendMessage error" << endl;
    }

}




void LUdpProcess(int serv_udp_sock, char* log_id, sockaddr_in* client_addr, socklen_t client_addr_len) {
    char* log_message = (char*)malloc(11);
	log_message[0] = 'L';
	strncpy(&log_message[1], log_id, 9);
    log_message[10] = '\0';
	// 要发给新登陆用户的信息
    char* old_message = (char*)malloc(11);
    old_message[0] = 'L';
    old_message[10] = '\0';
	// 遍历发送
    for(auto it = OnLineUsers.begin(); it != OnLineUsers.end(); it++) {
		// 1.旧用户信息发给新用户
		it->first.copy(&old_message[1], 9, 0);
        cout << "旧用户发给新用户"  << endl;			
		UdpSendMessage(serv_udp_sock, old_message, 11, 0, (sockaddr*)client_addr, client_addr_len);
	    // 2.新用户发给旧用户    
        cout << "新用户发给就用户" << endl;
		UdpSendMessage(serv_udp_sock, log_message, 11, 0, (sockaddr*)&it->second, sizeof(it->second));
	}
	// 3.客户端账号-》该客户端地址
        cout << log_id << "有人登录" << endl;
	OnLineUsers[log_id] = *client_addr;
    DisplayAllOnlineUser();
    // 4.向所有人发送一个EOF信息
	strcpy(&old_message[1], "EOF");
	old_message[5] = '\0';
	for(auto it = OnLineUsers.begin(); it != OnLineUsers.end(); it++) {    
		UdpSendMessage(serv_udp_sock, old_message, 5, 0, (sockaddr*)&it->second, sizeof(it->second));
	}

}

// 私聊  参数只需服务器ｓｏｃｋｅｔ　接受者账号　发送的消息(发送方账号和信息)　不需要发送者账号了
void CUdpProcess(int serv_udp_sock, char* to_id, char* message) {
    //　在message之前加上一Ｃ
    char* chat_message = (char*)malloc(strlen(message) + 1);
    chat_message[0] = 'C';
    strcpy(&chat_message[1], message);
    // 接受方的地址OnLineUsers[to_id]
    UdpSendMessage(serv_udp_sock,chat_message, Write_Buffer_SIZE, 0, (sockaddr*)&OnLineUsers[to_id], sizeof(sockaddr_in));
    cout << "向账号" << to_id << "发送消息" << chat_message << endl;
    cout << "接受方信息" ;
    DisplayOneAddr(OnLineUsers[to_id]); 
}

//　用户退出处理　　１.在本地在线用户变量中删除　２．通知剩下的所有用户
void OUdpProcess(int serv_udp_sock, char* exit_id, sockaddr_in* client_addr, socklen_t client_addr_len) {
    //1
    OnLineUsers.erase(exit_id);
    cout << "用户账号"<< exit_id << "退出" << endl;
    DisplayAllOnlineUser();
    //2
    char exit_message[11];
    exit_message[0] = 'O';
    exit_message[10] = '\0';
    strncpy(&exit_message[1], exit_id, 9);
    for(auto it = OnLineUsers.begin(); it != OnLineUsers.end(); it++) {
        UdpSendMessage(serv_udp_sock, exit_message, 10, 0, (sockaddr*)&(it->second), sizeof(sockaddr_in));
    }
}
//未知类型信息处理
void UnKnowUdpProcess(int serv_udp_sock, sockaddr_in* client_addr, socklen_t client_addr_len) {
   char error_message[30] = "Xerror, UnKonw Message Type";
   UdpSendMessage(serv_udp_sock, error_message, 30, 0, (sockaddr*)client_addr, sizeof(sockaddr_in));
}


  //1开始udp服务 //1.sock 2.bind 3.sendto recvfrom
void* StartUdpServer(void* arg) {
    int serv_udp_sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(serv_udp_sock == -1)
        DisplayError("create sock_udp_server");
    
    sockaddr_in serv_udp_addr;
    serv_udp_addr.sin_family = AF_INET;
    serv_udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_udp_addr.sin_port = htons(atoi("12345"));
    
    int bind_result = bind(serv_udp_sock, (sockaddr*)&serv_udp_addr, sizeof(serv_udp_addr));
    if(bind_result == -1)
        DisplayError("serv_udp_sock bind");
    // 开始发送结束数据
    char read_buffer[READ_BUFFER_SIZE];
    sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(sockaddr_in) ;
    cout << "Udp服务器开始接受消息" << endl;
    while(1) {
	    memset(&client_addr, 0, sizeof(sockaddr_in));
        client_addr_len = sizeof(struct sockaddr_in);
        int len_read = recvfrom(serv_udp_sock, read_buffer, READ_BUFFER_SIZE, 0, (sockaddr*)&client_addr, &client_addr_len); // 阻塞的读取客户端数据
        //  对读取到的数据进行分解 X(类型)+ 收信人账号+信息
        char type = read_buffer[0];
        char id[10]; // 表示发送给何人  
        strncpy(id, &read_buffer[1], 9);
        // 执行下面这条语句后，上面的client_addr_len由16突然变成0，好绝望，想不明白为什么
        id[10] = '\0';
        
        char* message = (char*)malloc(strlen(read_buffer)-10); // message包含发送方账号　和　发送信息
        strncpy(message, &read_buffer[10], strlen(read_buffer)-10);
        //　得到　type+id+message
        switch(type) {
            case 'C':  //  私聊
                CUdpProcess(serv_udp_sock, id, message);
                break;
            case 'W':  // 群聊
                break;
            case 'L':  // 新人登录     一个新用户  若干旧用户    1.旧用户信息 it-》first发给新用户clientaddr 2.新用户信息 log_id发给旧用户的地址it->second     3.把新登陆的客户端，加入到本地当前在线用户                 // 要发给旧用户的信息 				
	        LUdpProcess(serv_udp_sock, id, &client_addr, client_addr_len); 
		break;
            case 'O':   // 旧人退出
                OUdpProcess(serv_udp_sock, id, &client_addr, client_addr_len);
                break;
            default:
                UnKnowUdpProcess(serv_udp_sock, &client_addr, client_addr_len);
                break;
        }
    }
    close(serv_udp_sock);
}


//Tcp服务流程
//　账号重复登录检测 重复返回ｔｒｕｅ　　不重复返回false
bool CheckDuplicateAccount(char* log_id) {
    if(OnLineUsers.find(log_id) == OnLineUsers.end()) { //　返回指向末尾的迭代器说明没找到不重复
        return false;
    } else {        
        return true;
    }
}



// 开启Tcp服务器
void* StartTcpServer(void* arg) {
    // 服务器tcp的地址信息
    sockaddr_in serv_tcp_addr;
    serv_tcp_addr.sin_family = AF_INET;
    serv_tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_tcp_addr.sin_port = htons(atoi("12345"));
    int serv_tcp_sock = 0;
    
    // 启动tcp服务器四部 1. sock  2.bind 3. listen 4.accept 三报文握手发生 第二个报文发送 第三个报文接受
    serv_tcp_sock = socket(PF_INET, SOCK_STREAM, 0);
    if(serv_tcp_sock == -1) 
        DisplayError("create serv_tcp_sock ");
    int bind_tcp_result = bind(serv_tcp_sock, (sockaddr*)&serv_tcp_addr, sizeof(sockaddr));
    if(bind_tcp_result == -1) 
        DisplayError("bind  serv_tcp_sock");
    int listen_tcp_result = listen(serv_tcp_sock, 10); // 客户端连接等待队列为10
    if(listen_tcp_result == -1)
        DisplayError("listen serv_tcp_sock");
    // 这里我们先采用一个epoll的形式实现tcp服务器，一方面不用创建大量进线程 另一方面服务多客户. 单线程的epoll，触发量达到15000，但加上业务后会有阻塞情况，可以用多线程创建多个epoll进行操作
    // epoll的流程 1.创建epoll 2.向epoll注册要检测的文件描述符及事件类型 3.等待操作系统告知  4.重复23 直到销毁epoll
	int epoll_fd = epoll_create(100);
	if(epoll_fd == -1)
		DisplayError("epoll create");
	epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = serv_tcp_sock;
	// 把服务器tcp套接字注册监听其是否由新的连接
	int epoll_ctl_result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, serv_tcp_sock, &event);
	if(epoll_ctl_result == -1)
		DisplayError("epoll_ctl");
	// 开始等待
	epoll_event* read_events; // 所有有待读数据的套接字事件
    read_events = (epoll_event*)malloc(sizeof(struct epoll_event) * 100);
	cout << "tcp服务器开始监听套接字" << endl;
	while(1) {
	    int events_cnt = epoll_wait(epoll_fd, read_events, 100, -1);
		
		for(int i = 0; i<events_cnt; i++) {
			if(read_events[i].data.fd == serv_tcp_sock) {
				// 服务器套接字处理
				sockaddr_in client_tcp_addr;
				socklen_t client_tcp_len;
				int client_tcp_sock = accept(serv_tcp_sock, (sockaddr*)&client_tcp_addr, &client_tcp_len);
				if(client_tcp_sock == -1) 
					DisplayError("accept client connect");
				// 将该client端放到有待读的监听事件中去
				event.events = EPOLLIN;
				event.data.fd = client_tcp_sock;
				int epoll_ctl_result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_tcp_sock, &event);				
			} else {
				// 客户端套接字处理
				char read_buffer[READ_BUFFER_SIZE]; // 读缓冲大小
                 char write_buffer[Write_Buffer_SIZE]; // 写缓冲
				//1.接受客户端发过来的数据
 				int read_result = read(read_events[i].data.fd, read_buffer, READ_BUFFER_SIZE);
                 if(read_result == 0) {
                     cout << "读取错误" << endl;
                 }
					//对读取的信息进行解读
					// 举例 X(消息类型) XXXXXXX(9位账号) XXXXXXXXXX(密码若干位)
					
					char type = read_buffer[0];
					// 把账号拿出来
					char log_id[9];
					strncpy(log_id, &read_buffer[1], 9);
                     log_id[9] = '\0';
                     // 确保账号正确比对密码以防止出错 举例Ｌ+8位账号+若干密码()非法数据 
                     char * password = (char*)malloc(strlen(read_buffer)-10);
					strncpy(password, &read_buffer[10], strlen(read_buffer)-10);
					switch(type) {
						case 'L': // 登录
                             // 在数据库中找到东西然后验证是否正确
                             //0.判断账号是否重复登录
                             //1.判断账号存在
                             //2.密码是否则正确
                             if(CheckDuplicateAccount(log_id)) { //要检测当前已经登录的账号是否重复
                                 strcpy(write_buffer, "LNaccount already log,please dont repeate log");
                                 break;
                             }
                             if(strcmp(log_id, "123456789") != 0 && strcmp(log_id, "987654321") != 0) {// 账号不存在
                                 strcpy(write_buffer, "LNaccount is not existed");
                                 break;
                             }

                             if(strcmp(password, "123456789") != 0) { // 密码不正确
                                 strcpy(write_buffer, "LNpassword is error");
                                 break;
                             }
                             strcpy(write_buffer, "LY"); // 通过考验
						    break;
						case 'R': // 注册
                             // 写入数据库中
                             strcpy(write_buffer, "注册成功");                      
                             break;
                         case 'P': // 找回密码
						    break;
						default: // 未知消息类型
						    break;
					}										
				
				//2.将处理的反馈结果发回客户端
			        write(read_events[i].data.fd, write_buffer, Write_Buffer_SIZE);
                    close(read_events[i].data.fd);
                    int epoll_ctl_result = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, read_events[i].data.fd, &event);	 // 将其删除监视
			}		
		}			
	}
    close(epoll_fd);
    close(serv_tcp_sock);
}

int main() {
	// 开启Tcp服务的线程
	pthread_t thr_tcp_server;
	int result_create_udp_pthread = pthread_create(&thr_tcp_server, NULL, StartTcpServer, NULL);
	pthread_detach(thr_tcp_server);
	// 开启Udp服务的线程
	pthread_t thr_udp_server;
	int result_create_tcp_pthread = pthread_create(&thr_udp_server, NULL, StartUdpServer, NULL);
	pthread_detach(thr_udp_server);
	// 等待关闭
	string command = "";
        do{
            if(command == "quit")
	        break;
	    cout << "输入quit退出" << endl;
	} while(cin >> command);
		
}
