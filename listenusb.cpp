#include "listenusb.h"
#include "DeviceInfo.h"

ListenUsb ListenUsb::instance;

ListenUsb::ListenUsb():interval(2000)
{
    init();
}

ListenUsb::~ListenUsb()
{
    //stopListening();
}

void ListenUsb::init() {
    timer = new QTimer(this);
    process=new QProcess(this);
    connect(timer, &QTimer::timeout, this, &ListenUsb::scanDevices);
    ListenDevice::registerListener(this);
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
    process->start("scrcpy/adb.exe", { "devices" });
    process->waitForFinished();
    QString output = process->readAllStandardOutput();
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);

    QString errorOutput = process->readAllStandardError();
    if (!errorOutput.isEmpty()) {
        qWarning() << "Error in adb command:" << errorOutput;
    }

    // 获取当前设备列表
    QSet<DeviceInfo> currentDevices;
    for (const QString& line : lines) {
        if (line.contains("\tdevice")&&!line.contains(":")) {
            QStringList parts = line.split("\t");
            if (parts.size() >= 2) {
                DeviceInfo info;
                info.deviceType = QString("USB");
                info.serialNumber = parts.first();
                currentDevices.insert(info);
            }
        }
    }

 
    ListenDevice::updateChangeSet(currentDevices);
}


