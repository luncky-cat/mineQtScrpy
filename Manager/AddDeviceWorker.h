#ifndef ADDDEVICEWORKER_H
#define ADDDEVICEWORKER_H

#include "../Device/DeviceInfo.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSet>
#include<QWidget>

class AddDeviceWorker : public QThread {
    Q_OBJECT
public:
    AddDeviceWorker(QSet<ConnectInfo>& queue, QMutex& mutex, QWaitCondition& condition);
protected:
    void run() override;
signals:
    void deviceReadyToAdd(QSet<ConnectInfo> device);
private:
    QSet<ConnectInfo>& queue;
    QMutex& mutex;
    QWaitCondition& condition;
};


#endif // ADDDEVICEWORKER_H


