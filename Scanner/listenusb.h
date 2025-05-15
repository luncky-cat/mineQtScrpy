#ifndef LISTENUSB_H
#define LISTENUSB_H

#include "ListenDevice.h"

#include<libusb/libusb.h>

#include <QAbstractNativeEventFilter>
#include <QTimer>


/**
 * @brief USB设备监听类
 * 使用libusb库实现USB设备的检测和监听
 * 支持两种检测机制：
 * 1. 热插拔检测（优先使用）
 * 2. 定时扫描（热插拔不可用时使用）
 */


class ListenUsb : public ListenDevice,public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    void startListening() override;  // 开始监听设备
    void stopListening() override;   // 停止监听设备
    bool isListening() const;


private:
    explicit ListenUsb(QObject* parent=nullptr);
    ListenUsb(const ListenUsb&) = delete;
    ListenUsb& operator=(const ListenUsb&) = delete;
    ~ListenUsb() override;

    ConnectInfo createConnectInfo(const QString serialNumber);


    // libusb 相关功能
    bool initializeUsbAndHotplug();              // 初始化和热插拔判断
    void cleanupLibusb();           // 清理libusb资源
    void scanDevices();             // 扫描当前连接的设备
    QString getDeviceSerialNumber(libusb_device* device);  // 获取设备序列号
    bool isAndroidDevice(libusb_device* device);          // 判断是否为安卓设备
    
    // 热插拔相关
    void handleHotplugEvent(libusb_device* device, libusb_hotplug_event event);  // 处理热插拔事件
    static int LIBUSB_CALL hotplugCallback(libusb_context* ctx, libusb_device* device,
                                           libusb_hotplug_event event, void* user_data);  // 热插拔回调函数

private:
    static ListenUsb instance;
    bool listeningFlag;             // 监听状态

    libusb_context* context;
    bool useHotplug;              // 是否使用热插拔功能
    libusb_hotplug_callback_handle hotplugHandle;  // 热插拔回调句柄


    QTimer* scanTimer;
    static void extract_serial_fallback(const char *path);
    static void log_callback(libusb_context *ctx, libusb_log_level level, const char *msg);

#ifdef Q_OS_WIN      //定义windows平台下专用
    HWND hwnd;
    HDEVNOTIFY hDevNotify;
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
    bool registerUsbDeviceNotification();
    bool createMessageWindow();
    void handleDeviceChange(WPARAM wParam, LPARAM lParam);
#endif

};

#endif // LISTENUSB_H
