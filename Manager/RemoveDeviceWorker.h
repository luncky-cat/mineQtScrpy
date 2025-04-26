#ifndef REMOVEDEVICEWORKER_H
#define REMOVEDEVICEWORKER_H

#include "../Device/DeviceInfo.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QSet>

class RemoveDeviceWorker : public QThread {
    Q_OBJECT
public:
    RemoveDeviceWorker(QSet<ConnectInfo>& queue, QMutex& mutex, QWaitCondition& condition);
protected:
    void run() override;
signals:
    void deviceReadyToRemove(const ConnectInfo& device);
private:
    QSet<ConnectInfo>& queue;
    QMutex& mutex;
    QWaitCondition& condition;
};

#endif // REMOVEDEVICEWORKER_H
