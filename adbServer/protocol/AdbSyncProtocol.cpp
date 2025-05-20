#include "AdbSyncProtocol.h"

std::vector<uint8_t> AdbSyncProtocol::generateSEND(const std::string& remotePathWithMode) {
    std::vector<uint8_t> payload;
    uint32_t pathLen = static_cast<uint32_t>(remotePathWithMode.size());

    // 构造 SEND header (command + path length)
    payload.push_back('S'); payload.push_back('E');
    payload.push_back('N'); payload.push_back('D');

    payload.push_back(pathLen & 0xFF);
    payload.push_back((pathLen >> 8) & 0xFF);
    payload.push_back((pathLen >> 16) & 0xFF);
    payload.push_back((pathLen >> 24) & 0xFF);

    // 添加路径和权限（如 "/data/local/tmp/test.txt,33206"）
    payload.insert(payload.end(), remotePathWithMode.begin(), remotePathWithMode.end());

    return payload;
}

/// 构造 DATA 块 payload（每块最大 64K）
std::vector<uint8_t> AdbSyncProtocol::generateDATA(const std::vector<uint8_t>& buf, size_t len) {
    std::vector<uint8_t> payload;

    // 写入 "DATA" 标识
    payload.push_back('D'); payload.push_back('A');
    payload.push_back('T'); payload.push_back('A');

    // 写入数据长度
    payload.push_back(len & 0xFF);
    payload.push_back((len >> 8) & 0xFF);
    payload.push_back((len >> 16) & 0xFF);
    payload.push_back((len >> 24) & 0xFF);

    // 附加实际文件内容
    payload.insert(payload.end(), buf.begin(), buf.begin() + len);

    return payload;
}

/// 构造 DONE payload（mtime 为最后修改时间）
std::vector<uint8_t> AdbSyncProtocol::generateDONE(uint32_t mtime) {
    std::vector<uint8_t> payload;

    payload.push_back('D'); payload.push_back('O');
    payload.push_back('N'); payload.push_back('E');

    payload.push_back(mtime & 0xFF);
    payload.push_back((mtime >> 8) & 0xFF);
    payload.push_back((mtime >> 16) & 0xFF);
    payload.push_back((mtime >> 24) & 0xFF);

    return payload;
}
