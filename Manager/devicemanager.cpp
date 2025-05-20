#include "devicemanager.h"
#include <QDebug>

DeviceManager& DeviceManager::getInstance() {
    static DeviceManager instance;
    return instance;
}

DeviceManager::DeviceManager(QObject* parent)
    : QObject(parent)
    , m_adbServer(AdbServer::getInstance())
{
    // // 设置 AdbServer 回调
    // m_adbServer.setDeviceStatusChangedCallback([this](const std::string& deviceId, DeviceStatus status) {
    //     emit deviceStatusChanged(QString::fromStdString(deviceId), status);
    // });

    // m_adbServer.setDeviceConnectedCallback([this](const std::string& deviceId) {
    //     emit deviceConnected(QString::fromStdString(deviceId));
    // });

    // m_adbServer.setDeviceDisconnectedCallback([this](const std::string& deviceId) {
    //     emit deviceDisconnected(QString::fromStdString(deviceId));
    // });
}

bool DeviceManager::addDevice(const ConnectInfo& info) {
    QMutexLocker locker(&m_mutex);
    
    QString deviceId = QString::fromStdString(info.deviceId);
    if (m_devices.contains(deviceId)) {
        return false;
    }

    // 注册到 AdbServer
    if (!m_adbServer.registerDevice(info)) {
        return false;
    }

    // 添加到本地设备列表
    DeviceInfo deviceInfo;
    deviceInfo.connectInfo = info;
    deviceInfo.status = DeviceStatus::Disconnected;
    m_devices[deviceId] = deviceInfo;

    emit deviceAdded(deviceInfo);
    return true;
}

bool DeviceManager::removeDevice(const QString& deviceId) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_devices.contains(deviceId)) {
        return false;
    }

    // 从 AdbServer 注销
    if (!m_adbServer.unregisterDevice(deviceId.toStdString())) {
        return false;
    }

    m_devices.remove(deviceId);
    emit deviceRemoved(deviceId);
    return true;
}

QList<DeviceInfo> DeviceManager::getDevices() const {
    QMutexLocker locker(&m_mutex);
    return m_devices.values();
}

bool DeviceManager::connectDevice(const QString& deviceId) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_devices.contains(deviceId)) {
        return false;
    }

    return m_adbServer.connectDevice(deviceId.toStdString());
}

bool DeviceManager::disconnectDevice(const QString& deviceId) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_devices.contains(deviceId)) {
        return false;
    }

    return m_adbServer.disconnectDevice(deviceId.toStdString());
}

bool DeviceManager::executeCommand(const QString& deviceId, const QString& command) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_devices.contains(deviceId)) {
        return false;
    }

    return m_adbServer.executeCommand(deviceId.toStdString(), command.toStdString(),
        [this](const std::string& deviceId, const std::vector<uint8_t>& response) {
            handleCommandResponse(deviceId, response);
        });
}

DeviceStatus DeviceManager::getDeviceStatus(const QString& deviceId) const {
    QMutexLocker locker(&m_mutex);
    
    if (!m_devices.contains(deviceId)) {
        return DeviceStatus::Disconnected;
    }

    return m_adbServer.getDeviceStatus(deviceId.toStdString());
}

bool DeviceManager::isDeviceConnected(const QString& deviceId) const {
    QMutexLocker locker(&m_mutex);
    
    if (!m_devices.contains(deviceId)) {
        return false;
    }

    return m_adbServer.isDeviceConnected(deviceId.toStdString());
}

ConnectInfo DeviceManager::getDeviceInfo(const QString& deviceId) const {
    QMutexLocker locker(&m_mutex);
    
    if (!m_devices.contains(deviceId)) {
        return ConnectInfo();
    }

    return m_adbServer.getDeviceInfo(deviceId.toStdString());
}

void DeviceManager::onDeviceDiscovered(const ConnectInfo& info) {
    if (addDevice(info)) {
        qDebug() << "New device discovered:" << QString::fromStdString(info.deviceId);
    }
}

void DeviceManager::onDeviceRemoved(const QString& deviceId) {
    if (removeDevice(deviceId)) {
        qDebug() << "Device removed:" << deviceId;
    }
}

void DeviceManager::updateDeviceList() {
    QMutexLocker locker(&m_mutex);
    
    // 更新设备状态
    for (auto it = m_devices.begin(); it != m_devices.end(); ++it) {
        it->status = m_adbServer.getDeviceStatus(it->connectInfo.deviceId);
    }
}

void DeviceManager::handleCommandResponse(const std::string& deviceId, const std::vector<uint8_t>& response) {
    QByteArray data(reinterpret_cast<const char*>(response.data()), response.size());
    emit commandResponse(QString::fromStdString(deviceId), data);
}
