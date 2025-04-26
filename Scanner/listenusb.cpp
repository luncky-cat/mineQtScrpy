#include "listenusb.h"

#include "../Device/DeviceInfo.h"

#include <QCoreApplication>
#include<QDir>
#include<qDebug>
#include <QString>
#include <QSharedPointer>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dbt.h>
#endif

ListenUsb ListenUsb::instance;

ListenUsb::ListenUsb():interval(5000)
{
    initSignals();
    //qApp->installNativeEventFilter(this);
    ListenDevice::registerListener(this);
}

ListenUsb::~ListenUsb()
{

}

void ListenUsb::initSignals(){
     connect(&timer, &QTimer::timeout, this, &ListenUsb::scanDevices);
}

void ListenUsb::startListening()
{
   timer.start(interval);
}

void ListenUsb::stopListening()
{
    timer.stop();
}

void ListenUsb::scanDevices()
{
    process.start(QStringLiteral("D:/Documents/mineQtScrcpy/scrcpy/adb.exe"), { "devices" });
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    QString errorOutput = process.readAllStandardError();
    if (!errorOutput.isEmpty()) {
        qWarning() << "Error in adb command:" << errorOutput;
    }

    // 获取当前设备列表
    QSet<ConnectInfo> currentDevices;
    for (const QString& line : lines) {
        if (line.contains("\tdevice")&&!line.contains(":")) {
            QStringList parts = line.split("\t");
            if (parts.size() >= 2) {
                ConnectInfo info;
                info.deviceType=DeviceType::USB;
                info.serialNumber=parts.first();
                currentDevices.insert(info);
            }
        }
    }

    ListenDevice::updateChangeSet(currentDevices);
}
//nativeEventFilter
// bool ListenUsb::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
// {
//     Q_UNUSED(eventType);  // 告诉编译器该参数未使用

// #ifdef Q_OS_WIN
//     MSG* msg = static_cast<MSG*>(message);
//     if (msg->message == WM_DEVICECHANGE) {
//         if (msg->wParam == DBT_DEVICEARRIVAL || msg->wParam == DBT_DEVICEREMOVECOMPLETE) {
//             qDebug() << "USB device plugged in or removed";
//             // 触发设备扫描
//             scanDevices();
//         }
//     }
// #endif
//     return false; // 返回 false 让其他过滤器继续处理该事件
// }


