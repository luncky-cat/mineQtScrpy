#include "WifiScannerTask.h"

#include <QTcpSocket>
#include <QDebug>
#include <qprocess.h>

WifiScannerTask::WifiScannerTask(const QString& ip, quint16 startPort, quint16 endPort) :ip(ip), startPort(startPort), endPort(endPort){
    process = new QProcess();
}

WifiScannerTask:: WifiScannerTask(const QString& ip)
    :ip(ip), startPort(5555), endPort(5555) {
    process = new QProcess();
}

QPair<QString, quint16> WifiScannerTask::run() {
    for (quint16 port = startPort; port <= endPort; ++port) {
        if (isAdbService(ip, port)) {
            qDebug() << "WI-FI连接找到了!!!!"<<ip<<"port:"<<port;
            return {ip,port};
        }
    }
    return {};
}

bool WifiScannerTask::isAdbService(const QString& ip, quint16 port) {
    QString fullAddress = QString("%1:%2").arg(ip).arg(port);
    process->start("scrcpy/adb.exe",QStringList() << "connect" << fullAddress);
    process->waitForFinished();
    QString output = process->readAllStandardOutput();
    QStringList lines = output.split("\n", Qt::SkipEmptyParts);
    QString errorOutput = process->readAllStandardError();
    if (!errorOutput.isEmpty()) {
        qWarning() << "Error in adb command:" << errorOutput;
        process->start("scrcpy/adb.exe", QStringList() << "connect" << fullAddress);
        return false;
    }

   // qDebug() << output;
    if (output.contains("connected") || output.contains("already connected")) {
        return true;
    }

    process->start("scrcpy/adb.exe", QStringList() << "disconnect" << fullAddress);
    return false;
}
