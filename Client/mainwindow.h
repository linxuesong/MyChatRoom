#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include"chatform.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    chatForm* chatform;
// 槽声明
private slots:
    void on_pushButtonLogin_clicked();
};
#endif // MAINWINDOW_H
