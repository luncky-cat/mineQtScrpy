QT += core gui
QT += multimedia multimediawidgets
QT += network
QT += concurrent


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += debug

INCLUDEPATH += $$PWD/third_party/openssl/include
INCLUDEPATH += $$PWD/third_party/libusb/include
INCLUDEPATH += $$PWD/third_party/nlohmann/include
INCLUDEPATH += $$PWD/adbServer
INCLUDEPATH += $$PWD/Device
INCLUDEPATH += $$PWD/Scanner

# Windows SDK 路径
win32 {
    INCLUDEPATH += $$(WINDOWS_SDK_PATH)/Include/$$(WINDOWS_SDK_VERSION)/um
    INCLUDEPATH += $$(WINDOWS_SDK_PATH)/Include/$$(WINDOWS_SDK_VERSION)/shared
    INCLUDEPATH += $$(WINDOWS_SDK_PATH)/Include/$$(WINDOWS_SDK_VERSION)/ucrt
}

# 链接库

LIBS += -L$$PWD/third_party/openssl/lib
LIBS += -L$$PWD/third_party/libusb/lib
LIBS += -llibssl -llibcrypto -llibusb-1.0

# Windows USB 相关库
win32 {
    LIBS += -lsetupapi
    LIBS += -lwinusb
    LIBS += -luser32
    LIBS += -lws2_32
}

# 复制 DLL 到输出目录
win32 {
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y $$PWD/third_party/openssl/bin/libssl-1_1-x64.dll $$OUT_PWD/$${TARGET}.exe)
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y $$PWD/third_party/openssl/bin/libcrypto-1_1-x64.dll $$OUT_PWD/$${TARGET}.exe)
    QMAKE_POST_LINK += $$quote(cmd /c copy /Y $$PWD/third_party/libusb/bin/libusb-1.0.dll $$OUT_PWD/$${TARGET}.exe)
}

# 源文件
SOURCES += \
    CustomWidgets/DeviceListModel.cpp \
    CustomWidgets/DeviceListView.cpp \
    CustomWidgets/SidebarWidget.cpp \
    CustomWidgets/phoneshow.cpp \
    CustomWidgets/showlayout.cpp \
    #Device/androiddevice.cpp \
    Device/device.cpp \
    #Manager/AddDeviceWorker.cpp \
    #Manager/RemoveDeviceWorker.cpp \
    Manager/androidmanager.cpp \
    #Manager/devicemanager.cpp \
    Scanner/WifiScanner.cpp \
    Scanner/WifiScannerTask.cpp \
    Scanner/listenDevice.cpp \
    #Scanner/listenusb.cpp \
    Scanner/listenwifi.cpp \
    Scanner/networkutils.cpp \
    adbServer/code/AdbServer.cpp \
    adbServer/handlers/pushHandler.cpp \
    adbServer/protocol/AdbProtocol.cpp \
    adbServer/protocol/AdbSyncProtocol.cpp \
    adbServer/server/WifiServer.cpp \
    adbServer/states/AuthenticatingState.cpp \
    adbServer/states/ConnectingState.cpp \
    adbServer/states/DisconnectedState.cpp \
    adbServer/states/ExecutingState.cpp \
    adbServer/states/IState.cpp \
    adbServer/transport/ITransPort.cpp \
    adbServer/transport/socketTransPort.cpp \
    adbServer/utils/CryptoUtils.cpp \
    adbServer/utils/ThreadPool.cpp \
    main.cpp \
    mainwindow.cpp

# 头文件
HEADERS += \
    CustomWidgets/DeviceListModel.h \
    CustomWidgets/DeviceListView.h \
    CustomWidgets/SidebarWidget.h \
    CustomWidgets/phoneshow.h \
    CustomWidgets/showlayout.h \
    Device/ConnectInfo.h \
    Device/DeviceInfo.h \
    Device/StreamStatus.h \
    #Device/androiddevice.h \
    Device/device.h \
    #Manager/AddDeviceWorker.h \
    #Manager/RemoveDeviceWorker.h \
    Manager/androidmanager.h \
    Manager/devicemanager.h \
    Scanner/WifiScanner.h \
    Scanner/WifiScannerTask.h \
    Scanner/listenDevice.h \
    #Scanner/listenusb.h \
    Scanner/listenwifi.h \
    Scanner/networkutils.h \
    adbServer/code/AdbServer.h \
    adbServer/context/CommandInfo.h \
    adbServer/context/DeviceContext.h \
    adbServer/context/DeviceStatus.h \
    adbServer/handlers/pushHandler.h \
    adbServer/interfaces/ICommandHandler.h \
    adbServer/interfaces/IServer.h \
    adbServer/interfaces/IState.h \
    adbServer/interfaces/ITransPort.h \
    adbServer/protocol/AdbMessage.h \
    adbServer/protocol/AdbProtocol.h \
    adbServer/protocol/AdbSyncProtocol.h \
    adbServer/server/DeviceServerFactory.hpp \
    adbServer/server/WifiServer.h \
    adbServer/states/AuthenticatingState.h \
    adbServer/states/ConnectingState.h \
    adbServer/states/DisconnectedState.h \
    adbServer/states/ExecutingState.h \
    adbServer/transport/TransPortFactory.hpp \
    adbServer/transport/socketTransPort.h \
    adbServer/utils/CryptoUtils.h \
    adbServer/utils/ThreadPool.h \
    mainwindow.h \
    ui_mainwindow.h

FORMS += \
    mainwindow.ui






# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    .gitignore

