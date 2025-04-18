#include "androidmanager.h"
#include "../Scanner/listenDevice.h"  //监听类
#include "../Device/DeviceInfo.h"

androidManager::androidManager(QObject* parent)
    : QObject(parent),
    addWorker(new AddDeviceWorker(addQueue, addMutex, addCondition, this)),
    removeWorker(new RemoveDeviceWorker(removeQueue, removeMutex, removeCondition, this)),
    process(new QProcess(this))  // 可用于 adb 操作
{
    // 启动工作线程
    addWorker->start();
    removeWorker->start();

    // 可选：初始化设备列表为空
    devices.clear();

    // 可选：连接 worker 发出的信号，回调添加/移除设备
    connect(addWorker, &AddDeviceWorker::deviceReadyToAdd, this, &androidManager::handleDeviceAdded);
    connect(removeWorker, &RemoveDeviceWorker::deviceReadyToRemove, this, &androidManager::handleDeviceRemoved);
}



inline QDebug operator<<(QDebug debug, const DeviceInfo& device) {
    debug.nospace() 
        << ", type: " << device.deviceType
        << ", ip: " << device.ipAddress
        << ", serialNumber: " << device.serialNumber
        << ")";
    return debug;
}

androidManager::androidManager() {
    init();
    signalConnect();  //初始信号


    //初始化线程



}

void androidManager::init()
{
    process=new QProcess();
}

void androidManager::signalConnect()
{
    QList<ListenDevice*> listeners = ListenDevice::getListeners();
    for (ListenDevice* listener : listeners) {
        connect(listener, &ListenDevice::devicesAdd, this, &androidManager::onDevicesAdded);
        connect(listener, &ListenDevice::devicesRemove, this, &androidManager::onDevicesRemoved);
    }
}


void androidManager::onDevicesAdded(QSet<DeviceInfo>& DeviceChangeSet) {
    qDebug() << "新增设备:";
    for (const DeviceInfo& device : DeviceChangeSet) {
        qDebug() << device;  // 打印设备信息
    }
   
    QMutexLocker locker(&addMutex); // 加锁共享数据
    addQueue.unite(DeviceChangeSet);
    addCondition.wakeOne();                
}

void androidManager::onDevicesRemoved(QSet<DeviceInfo>& DeviceChangeSet)
{
    qDebug() << "移除设备:";
    for (const DeviceInfo& device : DeviceChangeSet) {
        qDebug() << device;  // 打印设备信息
    }

    QMutexLocker locker(&removeMutex); // 加锁共享数据
    removeQueue.unite(DeviceChangeSet);
    removeCondition.wakeOne();
}

void androidManager::processAddQueue() {
    while (true) {
        QMutexLocker locker(&addMutex);
        while (addQueue.isEmpty()) {
            addCondition.wait(&addMutex);  // 只等待新增设备的信号
        }
        



    }
}

void androidManager::processRemoveQueue() {
    while (true) {
        QMutexLocker locker(&removeMutex);
        while (removeQueue.isEmpty()) {
            removeCondition.wait(&removeMutex);  // 只等待移除设备的信号
        }
        



    }
}



void androidManager::handleDeviceAdded() {


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
    QList<ListenDevice*> listeners=ListenDevice::getListeners(); //监听
    qDebug() << "开启监听";
    for(auto listen:listeners){
        listen->startListening();    //开启监听
    }
}

//void androidManager::checkDevices() {
//
//}



void androidManager::addDevice(const QString &deviceId) {   

}

