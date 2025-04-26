#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include<QObject>

class DeviceManager:public QObject
{
    Q_OBJECT
public:
    //virtual void getDevices()=0;   //获得所有设备
    //virtual void allDescription()=0;  //获得所有设备的描述
    //virtual void description()=0;   //获得某个设备描述
    //virtual void closeDevices()=0;  //关闭所有设备
    //virtual void closeDevice()=0;   //关闭某个设备
    //virtual void selectDevice()=0;
    virtual ~DeviceManager()=default;
};

#endif // DEVICEMANAGER_H
