#include "listenDevice.h"

QList<ListenDevice*> ListenDevice::listeners;   

void ListenDevice::registerListener(ListenDevice* listener) {    //注册
    listeners.append(listener);
}

QList<ListenDevice*>ListenDevice::getListeners() {    //获得全部监听器
    return listeners;
}

void ListenDevice::updateChangeSet(const QSet<DeviceInfo>& newDevices) {
    QSet<DeviceInfo> currentNewDevices = newDevices - lastDevices;// 计算新增设备
    QSet<DeviceInfo> currentRemovedDevices = lastDevices - newDevices;   // 计算移除设备
    lastDevices = newDevices;// 更新 lastDevices 为当前设备
    auto changes = qMakePair(currentNewDevices, currentRemovedDevices);

    emit devicesChanged(changes);  //发送置manger
}
