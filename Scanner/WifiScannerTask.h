#ifndef WIFISCANNERTASK_H
#define WIFISCANNERTASK_H

#include <QString>
#include<QObject>
#include <QTcpSocket>

class WifiScannerTask :public QObject{
    Q_OBJECT
public:
    explicit WifiScannerTask(const QString& ip,quint16 port);
    explicit WifiScannerTask(const QString& ip);
    ~WifiScannerTask();
    void run();  // 执行扫描

private:
    void initSignals();

public slots:
    void onErrorOccurred(QAbstractSocket::SocketError socketError);
    void onConnected();

signals:
    void deviceFound(const QString &ip, quint16 port);  // 设备发现的信号

private:
    QString ip;
    quint16 port;
    QTcpSocket* socket;
};

#endif // WIFISCANNERTASK_H
