QT += core gui
QT += multimedia
QT += multimediawidgets
QT += network
QT += concurrent


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH+=$$PWD/include
LIBS+=$$PWD/dll/libcrypto.dll
LIBS+=$$PWD/dll/libssl.dll
# LIBS += -lws2_32


# 在.pro文件中定义宏



SOURCES += \
    Manager/AddDeviceWorker.cpp \
    Manager/RemoveDeviceWorker.cpp \
    Manager/androidmanager.cpp \
    Scanner/WifiScanner.cpp \
    Scanner/WifiScannerTask.cpp \
    Scanner/listenDevice.cpp \
    Scanner/listenusb.cpp \
    Scanner/listenwifi.cpp \
    Scanner/networkutils.cpp \
    adbServer/AdbCryptoUtils.cpp \
    adbServer/adbprotocol.cpp \
    adbServer/adbserver.cpp \
    adbServer/adbstate.cpp \
    main.cpp \
    mainwindow.cpp \
    phoneshow.cpp \
    showlayout.cpp \

HEADERS += \
    Device/ConnectInfo.h \
    Device/DeviceContext.h \
    Device/DeviceInfo.h \
    Device/StreamStatus.h \
    Device/device.h \
    Manager/AddDeviceWorker.h \
    Manager/RemoveDeviceWorker.h \
    Manager/androidmanager.h \
    Manager/devicemanager.h \
    RemoveDeviceWorker.h \
    Scanner/WifiScanner.h \
    Scanner/WifiScannerTask.h \
    Scanner/listenDevice.h \
    Scanner/listenusb.h \
    Scanner/listenwifi.h \
    Scanner/networkutils.h \
    adbServer/AdbCryptoUtils.h \
    adbServer/adbprotocol.h \
    adbServer/adbserver.h \
    adbServer/adbstate.h \
    androidmanager.h \
    log.h \
    mainwindow.h \
    phoneshow.h \
    showlayout.h \
    ui_mainwindow.h \

FORMS += \
    mainwindow.ui






# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

