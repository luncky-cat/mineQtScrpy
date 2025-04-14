#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QWidget>

class DeviceManager : public QWidget
{
    Q_OBJECT
public:
    explicit DeviceManager(QWidget *parent = nullptr);
private:
    virtual void getDevices()=0;   //获得所有设备
    virtual void closeDevices()=0;  //关闭所有设备
    virtual void allDescription()=0;  //获得所有设备的描述
    virtual void closeDevice()=0;   //关闭某个设备
    virtual void description()=0;   //获得某个设备描述
    virtual void init()=0;   //初始化
private:
    QList<Devic>
signals:
    


};

#endif // DEVICEMANAGER_H
