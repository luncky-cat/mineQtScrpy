#ifndef ADBPROTOCOL_H
#define ADBPROTOCOL_H

#include "AdbMessage.h"

#include <string>

class AdbProtocol {
public:
    static const unsigned ADB_HEADER_LENGTH;
    static const unsigned CMD_SYNC;
    static const unsigned CMD_CNXN;
    static const unsigned CONNECT_VERSION;
    static const unsigned CONNECT_MAXDATA;
    static const unsigned CMD_AUTH;
    static const unsigned AUTH_TYPE_TOKEN;
    static const unsigned AUTH_TYPE_SIGNATURE;
    static const unsigned AUTH_TYPE_RSA_PUBLIC;
    static const unsigned CMD_OPEN;
    static const unsigned CMD_OKAY;
    static const unsigned CMD_CLSE;
    static const unsigned CMD_WRTE;
    static std::vector<uint8_t> CONNECT_PAYLOAD;

    static unsigned getPayloadChecksum(const std::vector<uint8_t>& payload, unsigned offset);
    static std::vector<uint8_t> generateMessage(unsigned cmd, unsigned arg0, unsigned arg1, const std::vector<uint8_t>& payload);
    static std::vector<uint8_t> generateConnect();
    static std::vector<uint8_t> generateAuth(int type, const std::vector<uint8_t>& data);
    static std::vector<uint8_t> generateOpen(int localId, const std::string& dest);
    static std::vector<uint8_t> generateWrite(int localId, int remoteId, const std::vector<uint8_t>& data);
    static std::vector<uint8_t> generateClose(int localId, int remoteId);
    static std::vector<uint8_t> generateReady(int localId, int remoteId);
    static AdbMessage parseAdbMessage(std::vector<uint8_t>& data);
    static void printAdbMessage(const AdbMessage& msg);
};

#endif // AdbProtocol_H

