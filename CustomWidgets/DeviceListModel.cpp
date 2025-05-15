// DeviceListModel.cpp
#include "DeviceListModel.h"
#include <QColor>

// 构造函数
DeviceListModel::DeviceListModel(QObject *parent)
    : QAbstractListModel(parent) {}

// 设置设备数据
void DeviceListModel::setDeviceMap(const QMap<QString, DeviceInfo> &devices) {
    beginResetModel();   // 通知视图模型数据即将改变
    m_deviceMap = devices;
    endResetModel();     // 通知视图模型数据已改变
}

// 获取设备列表行数
int DeviceListModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return m_deviceMap.size();  // 返回设备数量
}

// 获取设备数据
QVariant DeviceListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    const DeviceInfo &device = m_deviceMap.values().at(index.row());  // 获取设备信息
    switch (role) {
    case Qt::DisplayRole:
        return device.connectInfo.deviceId;  // 显示设备名称
    case Qt::UserRole:
        return device.connectInfo.deviceId;    // 返回设备ID
    case Qt::BackgroundRole:
        //return (device.connectInfostatus == DeviceStatus::Connected) ? QColor(Qt::green) : QColor(Qt::red);  // 根据状态设置背景色
    default:
        return QVariant();
    }
}

// 获取列头数据
QVariant DeviceListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return "Device Name";  // 列头显示“设备名称”
    }
    return QVariant();
}
