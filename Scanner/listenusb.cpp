#include "listenusb.h"


#include <QCoreApplication>
#include <QDir>
#include <qDebug>
#include <QString>
#include <QSharedPointer>
#include <QWidget>
#include <QProcess>
#include <QThread>


#include <QSet>
#include <QTimer>
#include <QObject>


#ifdef Q_OS_WIN
#include <windows.h>
#include <dbt.h>
#include <initguid.h>
#include <usbiodef.h>
#endif


ListenUsb ListenUsb::instance;


//厂商的Vendor ID
const uint16_t ANDROID_VENDOR_IDS[] = {
    0x2717,  // Xiaomi
    0x22D9,  // OPPO
    0x2A45,  // VIVO
    0x2A70,  // OnePlus
    0x2A45,  // Realme
    0x2A45,  // iQOO
    0x2A45,  // Meizu
    0x2A45,  // ZTE
    0x2A45,  // Lenovo
    0x2A45,  // Huawei
    0x2A45   // Honor
};



ListenUsb::ListenUsb(QObject* parent)
    :listeningFlag(false),context(nullptr),useHotplug(false),hotplugHandle(0),
    hwnd(nullptr),hDevNotify(nullptr)
{
    ListenDevice::registerListener(this);
}


ListenUsb::~ListenUsb()
{
    stopListening();   //停止监听
}

ConnectInfo ListenUsb::createConnectInfo(const QString serialNumber)
{
    ConnectInfo info;
    info.serialNumber=serialNumber;
    return info;
}


/**
 * @brief 开始监听设备
 */
void ListenUsb::startListening()
{
    if (listeningFlag) {
        qDebug() << "Already listening";
        return;
    }

    initializeUsbAndHotplug();

    listeningFlag = true;
    qDebug() << "USB device listening started";

    // 如果不支持热插拔，使用定时器扫描
    if (!useHotplug) {
        scanTimer = new QTimer(this);
        connect(scanTimer, &QTimer::timeout, this, &ListenUsb::scanDevices);
        scanTimer->start(3000); // 每2秒扫描一次

        // if (!createMessageWindow()) {
        //     qDebug() << "Failed to create message window";
        //     return;
        // }

        // if (!registerUsbDeviceNotification()) {
        //     qDebug() << "Failed to register USB notification";
        //     return;
        // }

        // qApp->installNativeEventFilter(this); // 将本类注册为全局事件过滤器

    }

}


void ListenUsb::stopListening()
{
    if (!listeningFlag) {
        return;
    }

    qDebug() << "Stopping USB device listening...";
    if (useHotplug) {
        cleanupLibusb();
    }

    if (scanTimer) {
        scanTimer->stop();
        delete scanTimer;
        scanTimer = nullptr;
    }


    listeningFlag = false;
    qDebug() << "USB device listening stopped";
}



bool ListenUsb::isListening() const
{
    return listeningFlag;
}


/**
 * @brief 处理热插拔事件
 * @param device 设备指针
 * @param event 事件类型
 */
void ListenUsb::handleHotplugEvent(libusb_device* device, libusb_hotplug_event event)
{
    qDebug() << "Handling hotplug event...";
    if (!isAndroidDevice(device)) {
        qDebug() << "Not an Android device";
        return;
    }

    QString serialNumber = getDeviceSerialNumber(device);
    if (serialNumber.isEmpty()) {
        qDebug() << "Failed to get device serial number";
        return;
    }

    QSet<ConnectInfo> devices;
    ConnectInfo info;
    info.serialNumber = serialNumber;
    devices.insert(info);

    if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
        qDebug() << "Device connected:" << serialNumber;
        //emit devicesAdd(devices);
    } else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
        qDebug() << "Device disconnected:" << serialNumber;
        //emit devicesRemove(devices);
    }
}



/**
 * @brief libusb 热插拔回调函数
 * @param ctx libusb 上下文
 * @param device 设备指针
 * @param event 事件类型
 * @param user_data 用户数据
 * @return 0
 */
int LIBUSB_CALL ListenUsb::hotplugCallback(libusb_context* ctx, libusb_device* device, libusb_hotplug_event event, void* user_data)
{
    qDebug() << "Hotplug callback called, event:" << (event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED ? "arrival" : "removal");
    ListenUsb* listener = static_cast<ListenUsb*>(user_data);
    listener->handleHotplugEvent(device, event);
    return 0;
}


/**
 * @brief 初始化 libusb
 * @return 初始化是否成功
 */
bool ListenUsb::initializeUsbAndHotplug()
{
    qDebug() << "Initializing libusb...";
    int ret = libusb_init(&context);
    if (ret < 0) {
        qDebug() << "Failed to initialize libusb:" << libusb_error_name(ret);
        return false;
    }

    // 设置日志级别和回调
    libusb_set_option(context, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_INFO);
   // libusb_set_option(context, LIBUSB_OPTION_LOG_CB, (void*)log_callback);  //提取信息

    // 检查是否支持热插拔
    if (libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
        qDebug() << "Hotplug capability is supported";
        // 注册热插拔回调
        ret = libusb_hotplug_register_callback(
            context,
            LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED|LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
            LIBUSB_HOTPLUG_ENUMERATE,
            LIBUSB_HOTPLUG_MATCH_ANY,
            LIBUSB_HOTPLUG_MATCH_ANY,
            LIBUSB_HOTPLUG_MATCH_ANY,
            hotplugCallback,
            this,
            &hotplugHandle
            );

        if (ret == LIBUSB_SUCCESS) {   //注册成功
            useHotplug = true;
            qDebug() << "Successfully registered hotplug callback";
            return true;
        } else {
            qDebug() << "Failed to register hotplug callback:" << libusb_error_name(ret);
            useHotplug = false;
        }
    } else {
        qDebug() << "Hotplug capability is not supported";
        useHotplug = false;
    }

    // 如果不支持热插拔或注册失败，则使用轮询方式（定时扫描设备）
    qDebug() << "Hotplug not supported or registration failed, falling back to polling mechanism";
    return true;
}








/**
 * @brief 清理 libusb 资源
 */
void ListenUsb::cleanupLibusb()
{
    qDebug() << "Cleaning up libusb...";
    if (hotplugHandle) {
        libusb_hotplug_deregister_callback(context, hotplugHandle);
        hotplugHandle = 0;
    }

    if (context) {
        libusb_exit(context);
        context = nullptr;
    }
}





/**
 * @brief 扫描并更新设备列表
 */
void ListenUsb::scanDevices()
{
    qDebug() << "Scanning for USB devices...";
    libusb_device** list;
    ssize_t count = libusb_get_device_list(context, &list);
    if (count < 0) {
        qDebug() << "Failed to get device list:" << libusb_error_name(count);
        return;
    }

    qDebug() << "Found" << count << "USB devices";
    QSet<ConnectInfo> currentDevices;
    for (ssize_t i = 0; i < count; i++) {
        libusb_device* device = list[i];
        if (isAndroidDevice(device)) {
            QString serialNumber = getDeviceSerialNumber(device);
            if (!serialNumber.isEmpty()) {
                qDebug() << "Found Android device with serial:" << serialNumber;
                ConnectInfo info;
                info.deviceId=serialNumber;
                info.serialNumber = serialNumber;
                currentDevices.insert(info);
            }
        }
    }

    libusb_free_device_list(list, 1);

    ListenDevice::updateChangeSet(currentDevices);
}

/**
 * @brief 获取设备序列号
 * @param device 设备指针
 * @return 序列号字符串
 */
QString ListenUsb::getDeviceSerialNumber(libusb_device* device)
{
    libusb_device_handle* handle;
    int ret = libusb_open(device, &handle);
    if (ret != 0) {
        qDebug() << "Failed to open device:" << libusb_error_name(ret);
       return QString();
    }

    libusb_device_descriptor desc;
    ret = libusb_get_device_descriptor(device, &desc);
    if (ret < 0) {
        qDebug() << "Failed to get device descriptor:" << libusb_error_name(ret);
        libusb_close(handle);
        return QString();
    }



    unsigned char serial[256] = {0};
    ret = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, serial, sizeof(serial));
    libusb_close(handle);


    qDebug()
             << ": VID =" << QString::number(desc.idVendor, 16).rightJustified(4, '0')
             << ", PID =" << QString::number(desc.idProduct, 16).rightJustified(4, '0');

    if (ret > 0) {
        return QString::fromLatin1(reinterpret_cast<char*>(serial));
    }

    qDebug() << "Failed to get serial number:" << libusb_error_name(ret);
    return QString();
}

/**
 * @brief 判断是否为 Android 设备
 * @param device 设备指针
 * @return 是否为 Android 设备
 */
bool ListenUsb::isAndroidDevice(libusb_device* device)
{
    libusb_device_descriptor desc;
    int ret = libusb_get_device_descriptor(device, &desc);
    if (ret < 0) {
        qDebug() << "Failed to get device descriptor:" << libusb_error_name(ret);
        return false;
    }

    // 检查是否是已知的安卓厂商
    for (uint16_t vendorId : ANDROID_VENDOR_IDS) {
        if (desc.idVendor == vendorId) {
            qDebug() << "Found Android device with vendor ID:" << QString::number(desc.idVendor, 16);
            return true;
        }
    }

    return false;
}

/**
 * @brief Windows 消息过滤器
 * @param eventType 事件类型
 * @param message 消息指针
 * @param result 结果指针
 * @return 是否处理消息
 */
bool ListenUsb::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result)
{
    if (!listeningFlag || useHotplug) {
        return false;
    }

    if (eventType != "windows_generic_MSG") {
        return false;
    }

    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_DEVICECHANGE) {
        qDebug() << "Received device change message, wParam:" << msg->wParam;
        handleDeviceChange(msg->wParam, msg->lParam);
    }

    return false;
}

/**
 * @brief 处理设备变化事件
 * @param wParam 消息参数
 * @param lParam 消息参数
 */
void ListenUsb::handleDeviceChange(WPARAM wParam, LPARAM lParam)
{
    DEV_BROADCAST_DEVICEINTERFACE* devInterface = reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE*>(lParam);
    if (devInterface && devInterface->dbcc_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
        QString devicePath = QString::fromWCharArray(devInterface->dbcc_name);
        qDebug() << "设备路径：" << devicePath;
    }

}

/**
 * @brief 创建消息窗口
 * @return 是否创建成功
 */
bool ListenUsb::createMessageWindow()
{
    qDebug() << "Creating message window...";
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"USBDeviceListener";

    if (!RegisterClassEx(&wc)) {
        qDebug() << "Failed to register window class";
        return false;
    }

    hwnd = CreateWindowEx(
        0,
        L"USBDeviceListener",
        L"USB Device Listener",
        0,
        0, 0, 0, 0,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        nullptr
        );

    if (!hwnd) {
        qDebug() << "Failed to create message window";
        return false;
    }

    qDebug() << "Message window created successfully";
    return true;
}


/**
 * @brief 注册 USB 设备通知
 * @return 是否注册成功
 */
bool ListenUsb::registerUsbDeviceNotification()
{
    if (!hwnd) {
        qDebug() << "No window handle available";
        return false;
    }

    qDebug() << "Registering USB device notification...";
    DEV_BROADCAST_DEVICEINTERFACE dbdi = {0};
    dbdi.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    dbdi.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    dbdi.dbcc_classguid = GUID_DEVINTERFACE_USB_DEVICE;

    hDevNotify = RegisterDeviceNotification(
        hwnd,
        &dbdi,
        DEVICE_NOTIFY_WINDOW_HANDLE
        );

    if (!hDevNotify) {
        qDebug() << "Failed to register device notification";
        return false;
    }

    qDebug() << "USB device notification registered successfully";
    return true;
}


/**
 * @brief 从日志消息中提取序列号
 * @param path 包含序列号的路径字符串
 */
void ListenUsb::extract_serial_fallback(const char* path) {
    const char* start = strstr(path, "USB#");
    if (!start) return;

    const char* serial_start = strchr(start, '#');
    if (!serial_start) return;
    serial_start++;

    const char* serial_end = strchr(serial_start, '#');
    if (!serial_end) return;

    size_t len = serial_end - serial_start;
    if (len < 128) {
        char serial[128] = {0};
        strncpy(serial, serial_start, len);
        serial[len] = '\0';
        qDebug() << "（路径中提取）序列号:" << serial;
    }
}

/**
 * @brief libusb 日志回调函数
 * @param ctx libusb 上下文
 * @param level 日志级别
 * @param msg 日志消息
 */
void ListenUsb::log_callback(libusb_context* ctx, enum libusb_log_level level, const char* msg) {
    qDebug() << "libusb log:" << msg;
    if (strstr(msg, "could not open device") && strstr(msg, "USB#")) {
        extract_serial_fallback(msg);
    }
}
