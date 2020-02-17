#include "chatform.h"
#include "ui_chatform.h"
#include<QList>

#include<process.h> /*_beginthreadex*/
chatForm::chatForm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::chatForm)
{
    ui->setupUi(this);
}
chatForm::chatForm(QString my_account, QString my_password, QWidget *parent): QMainWindow(parent), ui(new Ui::chatForm) {
       ui->setupUi(this);
       _account = my_account;
       _password = my_password;
       Log(); // 1.刚一登录创建聊天界面进行登录信息的交换,初始化用户列表
       // 2.开启接受消息子线程
       StartRecvChatMessageThread();

}
chatForm::~chatForm()
{
    delete ui;
}


//注销当前用户按钮
void chatForm::on_pushButtonExit_clicked() {
    // 关闭窗口
    this->close();
}

void chatForm::on_pushButtonSend_clicked() {
    int tab_index = ui->tabWidgetChat->currentIndex(); //根据标签窗口的index确定发送私聊还是群聊
    switch(tab_index) {
        // 私聊 下标 0
        case 0:
            // 1.获取联系人的信息（对没有选人的情况做处理） 2. 对指定的人发送信息
            PrivateChat();
            break;
        // 群聊
        case 1:
            break;
        default:
            break;
    }
}
void chatForm::Log() {
    // 发送L+账号
    char log_info[10];
    log_info[0] = 'L';
    QByteArray ba = _account.toLatin1();
    char* account =ba.data();
    strcat(&log_info[1], account);
    _udp_client.SendMessage(log_info);
    // 循环接受服务器反馈
    RecvLogResponse();
    // 在线用户已经都在vector数组变量中了 下面刷新用户界面
    FlushOnLinePersion();
}

void chatForm::FlushOnLinePersion() {
    //根据本地私有数组变量  -》去显示   界面上空间的
    //1先清空界面上所有的用户
    ui->tableWidgetOnLineUsrs->clear();
    //2遍历本地私有变量逐个放上去
    QMap<QString,QString>::const_iterator it = onLineUsrs.begin();
    while(it != onLineUsrs.end()) {
        int rowCount = ui->tableWidgetOnLineUsrs->rowCount();
        ui->tableWidgetOnLineUsrs->insertRow(rowCount);
        ui->tableWidgetOnLineUsrs->setItem(rowCount, 0, new QTableWidgetItem(it.key())); //第一列
        ui->tableWidgetOnLineUsrs->setItem(rowCount, 1, new QTableWidgetItem(it.value()));
        it++;
    }
    //3显示
    ui->tableWidgetOnLineUsrs->show();
}


void chatForm::RecvLogResponse() {
    // 接受服务器端返回的若干在线用户的信息,直到L+EOF
    char response[ReadBufferSize];
    char other_account[9];
    while(1) {
        _udp_client.ReceiveMessage(response);
        if(response[0] == 'L') {
            strcpy(other_account, &response[1]);
            if(strcmp(other_account, "EOF") == 0) {
                break;
            } else {
                onLineUsrs[QLatin1String(other_account)] = "";  // 将客户端返回的一个用户加入到数组中去
            }
        }
    }

}
// 私聊模式 向udp服务器发送C+接受方id+发送方本人id+消息
void chatForm::PrivateChat() {
    // 1.获取联系人的信息（对没有选人的情况做处理） 2. 对指定的人发送信息  3.把自己发送的信息显示在对话框上
    int curRow = ui->tableWidgetOnLineUsrs->currentIndex().row();
    QAbstractItemModel* model = ui->tableWidgetOnLineUsrs->model();
    QModelIndex index = model->index(curRow, 0); // 选中行的第0列
    QVariant data = model->data(index);
    //获取的行转成char*
    QString data_str = data.toString();
    QByteArray ba3 = data_str.toLatin1();
    const char* other_usr = ba3.data();
    // 发送给指定的信息给服务器
    //C 表示 私聊某人 向指定的人发送消息  C+聊天对象账号+消息
    //加上C+接受方账号
    char message[1024];
    message[0] = 'C';
    strncpy(&message[1], other_usr, 9);
    // C+接受方id+发送方本人id
    QByteArray ba4 = _account.toLatin1();
    char* account =ba4.data();
    strncpy(&message[10], account, 9);
    //C+账号+编辑的要发送给的信息
    QString text = ui->textEditChat->document()->toPlainText();
    std::string temp = text.toStdString();
    const char* edit_text = temp.c_str();

    strcat(&message[18], edit_text);
    //2.发送
   _udp_client.SendMessage(message);
   // 3.对话框添加自己信息
   ui->textBrowserChat->append("我:" + text);
   // 想一下如何右侧显示自己的内容
   ui->textBrowserChat->moveCursor(ui->textBrowserChat->textCursor().End);
}

// 群聊模式(之后再补充)
void chatForm::GroupChat() {

}

// 接受udp聊天消息 ,包括了对有用户推出的信息的处理，新登陆用户信息的处理
/*
 * L 有人登录 L+登录人账号  对于已经登录的人来说只会收到一条这样的信息 外加一条EOF，同RecvLogResponse几乎一样，差别只是已经收到一条了，就差一个EOF
 * C 表示私聊消息  C+发信人账号 +发信人信息
 * W 表示群聊消息 W+发信人账号 + 发信人信息    （是否可以归为C类）
 * O 有人退出  O+登录人账号  发给所有在线人
*/
void chatForm::LUdpProcess(char* log_id) {
    // 已经获知登陆者的id，就差收取一条LEOF了
    //1把登陆者加入在线用户中
    onLineUsrs[QLatin1String(log_id)] = "";
    //2读取EOF
    char response[ReadBufferSize];
    char other_account[9];
    while(1) {
        _udp_client.ReceiveMessage(response);
        if(response[0] == 'L') {
            strcpy(other_account, &response[1]);
            if(strcmp(other_account, "EOF") == 0) {
                break;
            }
         }
    }
    //3刷新界面
    FlushOnLinePersion();
}
void chatForm::CUdpProcess(char *sender_id, char *message){
    // 将参数以 发送者账号：信息的形式显示到对话框
    // 转换为Qstring显示
    QString sum_message(sender_id);
    sum_message += ": ";
    sum_message += QString(message);
    // 显示
    ui->textBrowserChat->append(sum_message);
    ui->textBrowserChat->moveCursor(ui->textBrowserChat->textCursor().End);
}

void chatForm::OUdpProcess(char *exit_id) {
    // 1. 在本地在线用户数组中删除 2.刷新界面
    QString tmp_exit_id(exit_id);
    onLineUsrs.erase(onLineUsrs.find(tmp_exit_id));
    //2.
    FlushOnLinePersion();
}
unsigned chatForm::RecvChatMessage(void* args) { // args就是this指针
    chatForm* this_form = (chatForm*) args;
    char* read_buf = (char*)malloc(ReadBufferSize);
    char id[10];
    while(1) {
        memset(read_buf, 0, ReadBufferSize);
        this_form -> _udp_client.ReceiveMessage(read_buf);

        // 解析服务器发过来的信息
        //1
        char type = read_buf[0];
        //2
        strncpy(id, &read_buf[1], 9);
        id[9] = '\0';
        switch(type) {
            case 'L':
                this_form -> LUdpProcess(id);
                break;
            case 'C':
                this_form -> CUdpProcess(id, &read_buf[10]);
                break;
            case 'W':

                break;
            case 'O':
                this_form -> OUdpProcess(id);
                break;
            default:
                break;
        }


    }
}
// 需要一个线程去开启RecvChatMessage
void  chatForm::StartRecvChatMessageThread() {
    unsigned pthread_id;
    HANDLE child_thread;
    child_thread = (HANDLE)_beginthreadex(nullptr, 0, chatForm::RecvChatMessage, (void*) this, 0, &pthread_id);
}
void chatForm::Exit() {
    // 退出时发送一个O（大写的o） 表示退出     O+用户账号
    char exit_info[10];
    exit_info[0] = 'O';
    QByteArray ba = _account.toLatin1();
    char* account =ba.data();
    strcpy(&exit_info[1], account);
    _udp_client.SendMessage(exit_info);
}


void chatForm::closeEvent(QCloseEvent *event) {
    QMessageBox::Button button = QMessageBox::question(this, "退出程序", "确认退出程序？", QMessageBox::No|QMessageBox::Yes);
    if(button == QMessageBox::No) {
        event->ignore();
    } else if(button == QMessageBox::Yes) {
        Exit();
        event->accept();
    }

}


void chatForm::on_chatForm_destroyed() {
    Exit();
}
