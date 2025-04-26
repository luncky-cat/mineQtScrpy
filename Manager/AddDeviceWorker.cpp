#include "adddeviceworker.h"
#include<Qdebug>

AddDeviceWorker::AddDeviceWorker(QSet<ConnectInfo>& queue, QMutex& mutex, QWaitCondition& condition)
    : queue(queue), mutex(mutex), condition(condition) {
}

void AddDeviceWorker::run() {
    while (!isInterruptionRequested()) {
        QMutexLocker locker(&mutex);

        if (queue.isEmpty()) {
            condition.wait(&mutex);
        }

        if (isInterruptionRequested()) break;

        QSet<ConnectInfo> toProcess = queue;
        queue.clear();
        locker.unlock();

        for (const auto& device : toProcess) {
            qDebug() << "处理新增设备:";
            emit deviceReadyToAdd(device);
        }
    }
}

//验证设备
