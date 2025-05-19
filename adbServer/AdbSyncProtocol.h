#ifndef ADBSYNCPROTOCOL_H
#define ADBSYNCPROTOCOL_H

#include <string>
#include <vector>

// constexpr uint32_t SYNC_SEND = ('S') | ('E' << 8) | ('N' << 16) | ('D' << 24);
// constexpr uint32_t SYNC_DATA = ('D') | ('A' << 8) | ('T' << 16) | ('A' << 24);
// constexpr uint32_t SYNC_DONE = ('D') | ('O' << 8) | ('N' << 16) | ('E' << 24);

class AdbSyncProtocol
{
public:
    static std::vector<uint8_t> generateSEND(const std::string &remotePathWithMode);
    static std::vector<uint8_t> generateDATA(const std::vector<uint8_t> &buf, size_t len);
    static std::vector<uint8_t> generateDONE(uint32_t mtime);
};

#endif // ADBSYNCPROTOCOL_H
