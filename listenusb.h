#ifndef LISTENUSB_H
#define LISTENUSB_H
#include "listenDevice.h"
#include <QProcess>
#include<QTimer>

class ListenUsb : public ListenDevice
{
    Q_OBJECT
public:
    ListenUsb();
    explicit ListenUsb(qint32 time);
    void startListening() override;
    void stopListening()override;
private:
    void init() override;
public slots:
     void scanDevices() override;
signals:
    void updateDeviceConnection(const QStringList &devices);
private:
    QTimer* timer;
    qint32 interval;
    QProcess *process;
};

#endif // LISTENUSB_H
