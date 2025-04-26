#include "WifiScannerTask.h"

#include <QTcpSocket>
#include <QDebug>
#include <QTimer>
#include <QNetworkProxy>
#include <QEventLoop>
#include <QThread>

WifiScannerTask::WifiScannerTask(const QString& ip, quint16 port)
    : ip(ip), port(port)
{
    socket = new QTcpSocket();
    socket->setProxy(QNetworkProxy::NoProxy);
    initSignals();
}

WifiScannerTask::WifiScannerTask(const QString& ip)
    : ip(ip), port(5555)
{
    socket = new QTcpSocket(this);  // 将 'this' 设置为父对象，确保父对象是正确的线程
    socket->moveToThread(QThread::currentThread());  // 将socket移动到当前线程
    socket->setProxy(QNetworkProxy::NoProxy);
    initSignals();
}

void WifiScannerTask::initSignals() {

    connect(socket, &QTcpSocket::connected, this, &WifiScannerTask::onConnected, Qt::QueuedConnection);
    connect(socket, &QTcpSocket::errorOccurred, this, &WifiScannerTask::onErrorOccurred, Qt::QueuedConnection);

}

void WifiScannerTask::onConnected() {
    qDebug()<<"成："<<ip<<port;
    emit deviceFound(ip,port);
    QTimer::singleShot(0, this, [this]() {
        socket->abort();  // 强制清理连接和内部状态
        socket->close();  // 确保完全关闭
        delete socket;    // 释放 socket
        socket = nullptr; // 防止后续使用
    });
}

void WifiScannerTask::onErrorOccurred(QAbstractSocket::SocketError socketError) {
    QTimer::singleShot(0, this, [this]() {
        socket->abort();  // 强制清理连接和内部状态
        socket->close();  // 确保完全关闭
        delete socket;    // 释放 socket
        socket = nullptr; // 防止后续使用
    });
}

void WifiScannerTask::run() {

    socket->connectToHost(ip, port);  // 尝试连接目标 IP 和端口
    if (socket->waitForConnected(1000)) {
        // 如果连接成功，触发 deviceFound 信号
        emit deviceFound(ip, port);
    } else {
        // 处理连接失败
    }
}

WifiScannerTask::~WifiScannerTask(){
    //delete socket;
}
