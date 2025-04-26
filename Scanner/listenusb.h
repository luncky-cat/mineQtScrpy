#ifndef LISTENUSB_H
#define LISTENUSB_H

#include "listenDevice.h"

#include <QProcess>
#include<QTimer>
#include <QSharedPointer>
#include <QAbstractNativeEventFilter>

class ListenUsb : public ListenDevice  //,public QAbstractNativeEventFilter
{
    Q_OBJECT
private:
    ListenUsb();
    ~ListenUsb();

private:
    void initSignals();

public:
    void startListening() override;
    void stopListening() override;

public slots:
    void scanDevices() override;

protected:
    //bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;// 处理系统消息

private:
    qint32 interval;
    QTimer timer;
    QProcess process;
    static ListenUsb instance;

};
#endif // LISTENUSB_H
