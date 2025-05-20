#include "androidmanager.h"

/*#include "AddDeviceWorker.h"            // 假设你分到单独头文件
#include "RemoveDeviceWorker.h"  */       // 同上
#include "Scanner/listenDevice.h"
#include "adbServer/code/AdbServer.h"

//#include<qDebug>

androidManager::androidManager():
    // :addWorker(new AddDeviceWorker(addQueue, addMutex,addCondition)),
    //removeWorker(new RemoveDeviceWorker(removeQueue, removeMutex,removeCondition)),
    process(new QProcess())
{
    devices.clear();
    signalConnect();
    //addWorker->start();
    // removeWorker->start();
}

void androidManager::signalConnect()
{
    //connect(addWorker,&AddDeviceWorker::deviceReadyToAdd, this,&androidManager::handleDeviceAdded);
    // connect(removeWorker, &RemoveDeviceWorker::deviceReadyToRemove, this, &androidManager::handleDeviceRemoved);

    //qDebug() << "正确";

    QList<ListenDevice*> listeners = ListenDevice::getListeners();
    for (ListenDevice* listener : listeners) {
        connect(listener, &ListenDevice::devicesAdd, this, &androidManager::onDevicesAdded);
        connect(listener, &ListenDevice::devicesRemove, this, &androidManager::onDevicesRemoved);
    }
}


androidManager::~androidManager() {

}


void androidManager::onDevicesAdded(const QSet<ConnectInfo>& DeviceChangeSet) {

    qDebug()<<"androidManager新增";
    for (const ConnectInfo& device : DeviceChangeSet) {   //假设只有wifi设备的情况
        if(device.ConnectType==ConnectType::WiFi){
            qDebug() << "新增设备:"<< device.ipAddress;  // 打印设备信息
        }
    }


    if (DeviceChangeSet.isEmpty()) {
        qDebug() << "No device info to process.";
        return;
    }

    for(auto info:DeviceChangeSet){
        AdbServer::getInstance().registerDevice(info);
    }

    AdbServer::getInstance().getRegisteredDevices();

    //渲染到ui中，这些可连接设备

    for(auto info:DeviceChangeSet){   //调试输出
        //AdbServer::getInstance().asyncConnectDevice(info.deviceId.toStdString());
    }


}

void androidManager::onDevicesRemoved(const QSet<ConnectInfo>& DeviceChangeSet)
{
    qDebug()<<"androidManager移除";
    for (const ConnectInfo& device : DeviceChangeSet) {
        qDebug() << "移除设备:"<< device.serialNumber;  // 打印设备信息
    }



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
