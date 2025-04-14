#include "listenusb.h"

void ListenUsb::init() {
    timer = new QTimer(this);
    process=new QProcess(this);
    connect(timer, &QTimer::timeout, this, &ListenUsb::scanDevices);
    startListening();
}

ListenUsb::ListenUsb():interval(300) {
    init();
}

ListenUsb::ListenUsb(qint32 interval):interval(interval)
{
    init();
}

void ListenUsb::startListening()
{
    timer->start(interval);
}

void ListenUsb::stopListening()
{
    timer->stop();
}

void ListenUsb::scanDevices()
{
    process->start("adb", {"devices"});
    process->waitForFinished();
    QString output = process->readAllStandardOutput();
    QStringList lines = output.split("\n",Qt::SkipEmptyParts);
    QString errorOutput = process->readAllStandardError();
    if (!errorOutput.isEmpty()) {
        qWarning() << "Error in adb command:" << errorOutput;
    }
    QStringList devices;
    for (const QString &line : lines) {
        if (line.contains("\tdevice")) {
            QStringList parts = line.split("\t");
            if (parts.size() >= 2) {
                devices.append(parts.first());  // 获取设备 ID
            }
        }
    }
    emit updateDeviceConnection(devices);
}


