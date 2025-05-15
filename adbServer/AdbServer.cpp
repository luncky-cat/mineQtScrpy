#include "Device/ConnectInfo.h"
#include "adbstate.h"
#include "adbserver.h"
#include "adbprotocol.h"

#include <WinSock2.h>
#include <vector>
#include <ws2tcpip.h>
// #pragma comment(lib, "ws2_32.lib")
// #include "Device/DeviceContext.h"

#include "AdbCryptoUtils.h"
AdbCryptoUtils AdbCrypto;


bool AdbServer::connect(const std::string &deviceId)    //管理者外部调用
{
    //切换到连接状态，后续如果只是共享算法则则可以共享
    setState(deviceId,std::make_unique<ConnectingState>());

    //获得结果
    return false;
}

void AdbServer::setDeviceInfos(const std::vector<ConnectInfo> &infos)
{
    //将外部数据转移到这里
    //DeviceContext context;

    for(auto info:infos){

    }

}

int AdbServer::connect(const ConnectInfo& info)
{
    if(info.deviceType==DeviceType::WiFi){
        SOCKET sock=connectWiFi(info);
        return sock;
    }

    if(info.deviceType==DeviceType::USB){    //先不实现

            //连接usb设备...
            //return connectUSB(info);
    }
    return 0;  //默认失败
}

//WIFI连接

SOCKET AdbServer::connectWiFi(const ConnectInfo &info)
{
    if(false==initNetwork()){
        return false;
    }
    SOCKET sock=connectDevice(info.ipAddress.toStdString().c_str(),info.port);
    return sock;
}

bool AdbServer::initNetwork() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);  // 初始化 Winsock 2.2
    if (result != 0) {
        printf("WSAStartup failed: %d\n", result);
        return false;
    }
    return true;
}


SOCKET AdbServer::connectDevice(const char* ip, int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Failed to create socket: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (::connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("Connect failed: %d\n", WSAGetLastError());
        closesocket(sock);
        return INVALID_SOCKET;
    }
    printf("Connected to %s:%d\n", ip, port);
    return sock;
}



bool AdbServer::authWiFi(const SOCKET &sock)
{
    //连接请求cnxm
    std::vector<uint8_t> connectText = AdbProtocol::generateConnect();
    int sendResult = send(sock, (const char*)connectText.data(), connectText.size(), 0);
    if (sendResult < 0) {
        return false;
    }

    std::vector<uint8_t> recvData(4096);  // 初步开个缓冲区
    int recvLen = recv(sock, (char*)recvData.data(), recvData.size(), 0);
    if (recvLen <= 0) {
        return false;
    }
    AdbProtocol::AdbMessage msg = AdbProtocol::parseAdbMessage(recvData);
    AdbProtocol::printAdbMessage(msg);


    if (msg.command ==0x48545541) {   // ==auth
        std::vector<uint8_t> sigload=AdbCrypto.signAdbTokenPayload(msg.payload);
        std::vector<uint8_t> sigMsg= AdbProtocol::generateAuth(2, sigload);
        sendResult=send(sock, (const char*)sigMsg.data(), sigMsg.size(), 0);
        if (sendResult < 0) {
            return false;
        }

        recvData.clear();
        int recvLen = recv(sock, (char*)recvData.data(), recvData.size(), 0);
        if (recvLen <= 0) {
            return false;
        }
        AdbProtocol::AdbMessage nextMsg = AdbProtocol::parseAdbMessage(recvData);
        AdbProtocol::printAdbMessage(nextMsg);
        return true;
    }
    return false;
}

bool AdbServer::setState(const std::string &deviceId, std::unique_ptr<IAdbState> newState)
{

    auto it=deviceContextMap.find(deviceId);    //查找状态

    if(it==deviceContextMap.end()){
        return false;
    }

    DeviceContext& context=deviceContextMap[deviceId];

    context.currentState=std::move(newState);

    context.currentState->handle(*this,context);

    return true;
}




