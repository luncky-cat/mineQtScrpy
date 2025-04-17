#ifndef LISTENUSB_H
#define LISTENUSB_H

#include "listenDevice.h"

#include <QProcess>
#include<QTimer>

class ListenUsb : public ListenDevice
{
    Q_OBJECT
private:
    ListenUsb();
    ~ListenUsb();
    void init();
public:
    void startListening() override;
    void stopListening()override;
public slots:
     void scanDevices() override;
private:
    QTimer* timer;
    qint32 interval; 
    QProcess *process;
    static ListenUsb instance; 
};

#endif // LISTENUSB_H
