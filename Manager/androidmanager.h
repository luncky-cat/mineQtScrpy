#ifndef ANDROIDMANAGER_H
#define ANDROIDMANAGER_H

#include "devicemanager.h"

#include <QProcess>
#include <QTimer>
#include<QMutex>
#include <QtConcurrent>

class Device;    //设备类
struct DeviceInfo;  //设备信息类

class androidManager : public DeviceManager
{
    Q_OBJECT
public:
    androidManager(QObject* parent);
    androidManager();
private:
    void init();
    void signalConnect();
public slots:
    void onDevicesAdded(QSet<DeviceInfo>&DeviceChangeSet);
    void onDevicesRemoved(QSet<DeviceInfo>& DeviceChangeSet);
    void processAddQueue();
    void processRemoveQueue();
public:
    void getDevices()override;
    void closeDevices()override;
    void allDescription()override;
    void closeDevice()override;
    void description()override;
    void selectDevice() override;
public:
    void listen();  //监听usb设备
    void addDevice(const QString &deviceId);
private:
private:
    class AddDeviceWorker : public QThread {
    public:
        AddDeviceWorker(QSet<DeviceInfo>& queue, QMutex& mutex, QWaitCondition& condition)
            : queue(queue), mutex(mutex), condition(condition) {
        }

        void run() override {
            while (!isInterruptionRequested()) {
                QMutexLocker locker(&mutex);
                if (queue.isEmpty()) {
                    condition.wait(&mutex);
                }

                // 再次检查是否中断
                if (isInterruptionRequested()) break;

                // 从队列取数据处理
                QSet<DeviceInfo> toProcess = queue;
                queue.clear();
                locker.unlock();

                // 处理逻辑，比如 qDebug()
                for (const auto& device : toProcess) {
                    qDebug() << "处理新增设备:" << device;
                }
            }
        }

    private:
        QSet<DeviceInfo>& queue;
        QMutex& mutex;
        QWaitCondition& condition;
    };
private:
    class RemoveDeviceWorker : public QThread {
    public:
        RemoveDeviceWorker(QSet<DeviceInfo>& queue, QMutex& mutex, QWaitCondition& condition)
            : queue(queue), mutex(mutex), condition(condition) {
        }

        void run() override {
            while (!isInterruptionRequested()) {
                QMutexLocker locker(&mutex);
                if (queue.isEmpty()) {
                    condition.wait(&mutex);
                }

                if (isInterruptionRequested()) break;

                QSet<DeviceInfo> toProcess = queue;
                queue.clear();
                locker.unlock();

                for (const auto& device : toProcess) {
                    qDebug() << "处理移除设备:" << device;
                }
            }
        }

    private:
        QSet<DeviceInfo>& queue;
        QMutex& mutex;
        QWaitCondition& condition;
    };
private:
   AddDeviceWorker* addWorker = nullptr;
   RemoveDeviceWorker* removeWorker = nullptr;
   QProcess *process;
   QList<Device*>devices;   //当前设备列表
   QMutex addMutex, removeMutex;
   QWaitCondition addCondition, removeCondition;
   QSet<DeviceInfo> addQueue,removeQueue;
};

#endif // ANDROIDMANAGER_H
