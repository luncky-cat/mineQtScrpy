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
INCLUDEPATH += $$PWD/third_party/asio/include
INCLUDEPATH += $$PWD/adbServer
INCLUDEPATH += $$PWD/Device
#INCLUDEPATH += $$PWD/Scanner

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
    adbServer/code/AdbServer.cpp \
    adbServer/code/ConfigCenter.cpp \
    adbServer/code/ScannerManager.cpp \
    adbServer/config/WifiScannerConfig.cpp \
    adbServer/context/AdbStreamContext.cpp \
    adbServer/context/DeviceContext.cpp \
    adbServer/context/DeviceSession.cpp \
    adbServer/context/ForwardBridge.cpp \
    adbServer/context/RemoteSession.cpp \
    adbServer/context/RemoteSession.cpp \
    adbServer/protocol/AdbProtocol.cpp \
    adbServer/protocol/AdbSyncProtocol.cpp \
    #adbServer/scanner/ConnectConfig/WifiConnectConfig.cpp \
    adbServer/scanner/WifiScanner/NetworkScanner.cpp \
    adbServer/server/WifiServer.cpp \
    adbServer/states/AuthenticatingState.cpp \
    adbServer/states/ConnectingState.cpp \
    adbServer/states/DisconnectedState.cpp \
    adbServer/states/ExecutingState.cpp \
    adbServer/states/IState.cpp \
    adbServer/utils/CryptoUtils.cpp \
    adbServer/utils/NetWorkUtils.cpp \
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
    # Device/ConnectInfo.h \
    # Device/DeviceInfo.h \
    # Device/StreamStatus.h \
    adbServer/code/AdbServer.h \
    # adbServer/config/ConnectConfig.h \
    adbServer/code/ConfigCenter.h \
    adbServer/code/ScannerManager.h \
    adbServer/config/WifiScannerConfig.h \
    adbServer/context/AdbStreamContext.h \
    adbServer/context/CommandInfo.h \
    adbServer/context/DeviceContext.h \
    adbServer/context/DeviceSession.h \
    adbServer/context/DeviceStatus.h \
    # adbServer/context/ForwardBridge.h \
    adbServer/context/ForwardBridge.h \
    adbServer/context/RemoteSession.h \
    #adbServer/handlers/ForwardHandler.h \
    #adbServer/handlers/pushHandler.h \
    #adbServer/handlers/shellHandler.h \
    adbServer/interfaces/ICommandHandler.h \
    adbServer/interfaces/IConfig.h \
    adbServer/interfaces/IScanner.h \
    adbServer/interfaces/IServer.h \
    adbServer/interfaces/IState.h \
    #adbServer/interfaces/ITransPort.h \
    adbServer/protocol/AdbMessage.h \
    adbServer/protocol/AdbProtocol.h \
    adbServer/protocol/AdbSyncProtocol.h \
    adbServer/scanner/ConnectConfig/WifiConnectConfig.h \
    #adbServer/scanner/SerialScanner/listenDevice.h \
    #adbServer/scanner/SerialScanner/listenusb.h \
    #adbServer/scanner/WifiConnectConfig.h \
    #adbServer/scanner/WifiScanner/WifiScanner.h \
    #adbServer/scanner/WifiScanner/WifiScannerTask.h \
    #adbServer/scanner/WifiScanner/listenwifi.h \
    adbServer/scanner/WifiScanner/NetworkScanner.h \
    #adbServer/scanner/WifiScanner/networkutils.h \
    adbServer/server/IServerFactory.hpp \
    adbServer/server/WifiServer.h \
    adbServer/states/AuthenticatingState.h \
    adbServer/states/ConnectingState.h \
    adbServer/states/DisconnectedState.h \
    adbServer/states/ExecutingState.h \
    adbServer/utils/CryptoUtils.h \
    adbServer/utils/NetWorkUtils.h \
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

