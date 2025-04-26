#include "listenDevice.h"

QList<ListenDevice*> ListenDevice::listeners;   

void ListenDevice::registerListener(ListenDevice* listener) {    //注册
    listeners.append(listener);
}

QList<ListenDevice*>ListenDevice::getListeners() {    //获得全部监听器
    return listeners;
}

void ListenDevice::listen()      //全部开启监听
{
    for (auto& listener: listeners) {
        listener->startListening();
    }
}

void ListenDevice::updateChangeSet(const QSet<ConnectInfo>& newDevices) {
    QSet<ConnectInfo> currentNewDevices = newDevices - lastDevices;// 计算新增设备
    QSet<ConnectInfo> currentRemovedDevices = lastDevices - newDevices;   // 计算移除设备
    lastDevices = newDevices;// 更新 lastDevices 为当前设备

    if (!currentNewDevices.isEmpty()) {
        emit devicesAdd(currentNewDevices);
    }

    if (!currentRemovedDevices.isEmpty()) {
        emit devicesRemove(currentRemovedDevices);
    }
}

