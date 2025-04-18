#ifndef DEVICE_H
#define DEVICE_H
#include<QWidget>
class Device : public QObject
{
    Q_OBJECT
public:
    explicit Device(QObject *parent = nullptr);

signals:


};

#endif // DEVICE_H
