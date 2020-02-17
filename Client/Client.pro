QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
LIBS += -lWs2_32
CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    TcpClient/tcp_client.cpp \
    UdpClient/udpclient.cpp \
    chatform.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    TcpClient/tcp_client.h \
    UdpClient/udpclient.h \
    chatform.h \
    mainwindow.h

FORMS += \
    chatform.ui \
    mainwindow.ui

TRANSLATIONS += \
    Client_zh_SG.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../../Desktop/文件.png \
    H:/林雪松毕业设计/参考代码/HTYChat-master/bg.jpg \
    img/文件.png \
    img/视频.png \
    img/邮箱-合上.png
