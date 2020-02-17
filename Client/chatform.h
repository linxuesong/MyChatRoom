#ifndef CHATFORM_H
#define CHATFORM_H
#include"UdpClient/udpclient.h"
#include <QMainWindow>
#include<QString>
#include<QMap>
#include<QCloseEvent>
namespace Ui {
class chatForm;
}

class chatForm : public QMainWindow
{
    Q_OBJECT

public:
    explicit chatForm(QWidget *parent = nullptr);
    chatForm(QString my_account, QString my_password, QWidget *parent = nullptr);
    ~chatForm();


private slots:
    void on_pushButtonExit_clicked();

    void on_pushButtonSend_clicked();

    void on_chatForm_destroyed();

private:
    //刚登陆发送一个L 表示 登录    L+ 用户账号
    void Log();
    // 动态加载在线用户的功能
    void FlushOnLinePersion();
    // 接受服务器返回的其他在线用户
    void RecvLogResponse();
    // 私聊模式
    void PrivateChat();
    // 群聊模式
    void GroupChat();
    // 接受udp聊天信息
    void LUdpProcess(char* log_id);
    void CUdpProcess(char* sender_id, char* message);
    void OUdpProcess(char* exit_id);
    static unsigned RecvChatMessage(void* args);
    // 开启接受udp聊天信息的线程
    void StartRecvChatMessageThread();

    // 退出时发送一个O 表示退出     O+用户账号
    void Exit();
protected:
    void closeEvent(QCloseEvent* event);


private:
    Ui::chatForm *ui;
    QString _account;
    QString _password;
    // 使用vector存储当前在线用户
    QMap<QString, QString> onLineUsrs;
    UdpClient _udp_client;
};

#endif // CHATFORM_H
