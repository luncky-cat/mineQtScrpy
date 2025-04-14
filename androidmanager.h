#ifndef ANDROIDMANAGER_H
#define ANDROIDMANAGER_H

#include "devicemanager.h"

class androidManager : public DeviceManager
{
    Q_OBJECT
public:
    androidManager();

    QMap<QString, Device*> devices;  // 设备ID -> 设备实例
    QTimer deviceCheckTimer;
};

#endif // ANDROIDMANAGER_H
