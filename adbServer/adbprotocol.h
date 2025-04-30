#ifndef ADBPROTOCOL_H
#define ADBPROTOCOL_H


#include <vector>
#include <qbytearray.h>

class AdbProtocol {
public:
    static const unsigned ADB_HEADER_LENGTH = 24;
    static const unsigned CMD_SYNC = 0x434e5953;
    static const unsigned CMD_CNXN = 0x4e584e43;
    static const unsigned CONNECT_VERSION = 0x01000000;
    static const unsigned CONNECT_MAXDATA = 4096;
    static const unsigned CMD_AUTH = 0x48545541;
    static const unsigned AUTH_TYPE_TOKEN = 1;
    static const unsigned AUTH_TYPE_SIGNATURE = 2;
    static const unsigned AUTH_TYPE_RSA_PUBLIC = 3;
    static const unsigned CMD_OPEN = 0x4e45504f;
    static const unsigned CMD_OKAY = 0x59414b4f;
    static const unsigned CMD_CLSE = 0x45534c43;
    static const unsigned CMD_WRTE = 0x45545257;

    struct AdbMessage {
        unsigned command;
        unsigned arg0;
        unsigned arg1;
        unsigned payloadLength;
        unsigned checksum;
        unsigned magic;
        std::vector<uint8_t> payload;
    };

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

#endif // ADBPROTOCOL_H



