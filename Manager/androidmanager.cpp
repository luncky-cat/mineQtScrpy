#include "androidmanager.h"

#include "AddDeviceWorker.h"            // 假设你分到单独头文件
#include "RemoveDeviceWorker.h"         // 同上
#include "Scanner/listenDevice.h"

#include<qDebug>

androidManager::androidManager()
    :addWorker(new AddDeviceWorker(addQueue, addMutex,addCondition)),
    removeWorker(new RemoveDeviceWorker(removeQueue, removeMutex,removeCondition)),
    process(new QProcess())
{
    devices.clear();
    signalConnect();
    addWorker->start();
    removeWorker->start();
}

void androidManager::signalConnect()
{
    connect(addWorker,&AddDeviceWorker::deviceReadyToAdd, this,&androidManager::handleDeviceAdded);
    connect(removeWorker, &RemoveDeviceWorker::deviceReadyToRemove, this, &androidManager::handleDeviceRemoved);

    qDebug() << "正确";

    QList<ListenDevice*> listeners = ListenDevice::getListeners();
    for (ListenDevice* listener : listeners) {
        connect(listener, &ListenDevice::devicesAdd, this, &androidManager::onDevicesAdded);
        connect(listener, &ListenDevice::devicesRemove, this, &androidManager::onDevicesRemoved);
    }
}


androidManager::~androidManager() {

}


void androidManager::onDevicesAdded(const QSet<ConnectInfo>& DeviceChangeSet) {
    for (const ConnectInfo& device : DeviceChangeSet) {
        qDebug() << "新增设备:"<< device.serialNumber;  // 打印设备信息
    }

    //QMutexLocker locker(&addMutex); // 加锁共享数据
   // addQueue.unite(DeviceChangeSet);
   // addCondition.wakeOne();
}

void androidManager::onDevicesRemoved(const QSet<ConnectInfo>& DeviceChangeSet)
{
    for (const ConnectInfo& device : DeviceChangeSet) {
       qDebug() << "移除设备:"<< device.serialNumber;  // 打印设备信息
    }

    //QMutexLocker locker(&removeMutex); // 加锁共享数据
  //  removeQueue.unite(DeviceChangeSet);
    //removeCondition.wakeOne();
}

void androidManager::handleDeviceAdded(const ConnectInfo& device) {


}

void androidManager::handleDeviceRemoved(const ConnectInfo& device) {

}



void androidManager::getDevices()
{

}

void androidManager::closeDevices()
{

}

void androidManager::allDescription()
{

}

void androidManager::closeDevice()
{

}

void androidManager::description()
{

}

void androidManager::selectDevice()
{

}


void androidManager::listen()  //监听usb设和wf备的接入
{
    qDebug() << "开启监听";
    ListenDevice::listen();
}
