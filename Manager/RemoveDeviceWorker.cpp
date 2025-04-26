#include "removedeviceworker.h"
#include<qDebug>

RemoveDeviceWorker::RemoveDeviceWorker(QSet<ConnectInfo>& queue, QMutex& mutex, QWaitCondition& condition)
    : queue(queue), mutex(mutex), condition(condition) {
}

void RemoveDeviceWorker::run() {
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
            qDebug() << "处理移除设备:";
            emit deviceReadyToRemove(device);
        }
    }
}
