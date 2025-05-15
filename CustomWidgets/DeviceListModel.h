// DeviceListModel.h
#ifndef DEVICELISTMODEL_H
#define DEVICELISTMODEL_H

#include <QAbstractListModel>
#include <QMap>
#include "DeviceInfo.h"  // 包含 DeviceInfo 结构体

// 自定义模型类，继承自 QAbstractListModel
class DeviceListModel : public QAbstractListModel {
    Q_OBJECT

public:
    // 构造函数
    explicit DeviceListModel(QObject *parent = nullptr);

    // 设置设备映射，更新数据模型
    void setDeviceMap(const QMap<QString, DeviceInfo> &devices);

    // 获取行数
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // 获取数据
    QVariant data(const QModelIndex &index, int role) const override;

    // 获取列头数据
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // 自定义角色
    enum DeviceRoles {
        DeviceNameRole = Qt::UserRole + 1,  // 设备名称角色
        DeviceIdRole,                        // 设备ID角色
        DeviceStatusRole                     // 设备状态角色
    };

protected:
    QMap<QString, DeviceInfo> m_deviceMap;  // 存储设备信息的映射
};

#endif // DEVICELISTMODEL_H

