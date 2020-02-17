#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "TcpClient/tcp_client.h"
#include<QString>
#include<QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonLogin_clicked() {
    // 1.初始化Tcp client
    TcpClient tcp_client;

    // 2.Tcp client发送账号密码信息
    QByteArray ba = ui->lineEditAccount->text().toLatin1();
    char* account =ba.data();

    QByteArray ba2 = ui->lineEditPassword->text().toLatin1();
    char* password =ba2.data();
    int log_result = tcp_client.Login(account, password);
    if(log_result) {
        //successful ,启动聊天界面
        chatform = new chatForm(ui->lineEditAccount->text(), ui->lineEditPassword->text());
        chatform->show();
        this->close();
        
    } else {
        // false, 弹出错误对话框
       QMessageBox::warning(NULL, "错误", QString::fromStdString(tcp_client.error_info));
    }
}
