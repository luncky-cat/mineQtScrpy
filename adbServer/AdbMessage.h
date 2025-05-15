#ifndef ADBMESSAGE_H
#define ADBMESSAGE_H

#include <vector>

struct AdbMessage {
    unsigned command;
    unsigned arg0;
    unsigned arg1;
    unsigned payloadLength;
    unsigned checksum;
    unsigned magic;
    std::vector<uint8_t> payload;
};

#endif // ADBMESSAGE_H
