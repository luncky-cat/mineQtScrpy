#include "mainwindow.h"

#include <QApplication>

#include <QMetaType>
#include <QSet>

#include "Device/connectinfo.h"  // 包含 ConnectInfo 的定义

Q_DECLARE_METATYPE(ConnectInfo)

int main(int argc, char *argv[])
{
    qRegisterMetaType<ConnectInfo>("ConnectInfo");
    qRegisterMetaType<QSet<ConnectInfo>>("QSet<ConnectInfo>");

    QApplication a(argc, argv);

    MainWindow w;
    w.show();
    return a.exec();
}
