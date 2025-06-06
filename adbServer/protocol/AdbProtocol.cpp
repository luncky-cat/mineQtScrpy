#include "AdbProtocol.h"

#include <iostream>
#include <iomanip>
#include <optional>

const unsigned AdbProtocol::ADB_HEADER_LENGTH = 24;
const unsigned AdbProtocol::CMD_SYNC = 0x434e5953;
const unsigned AdbProtocol::CMD_CNXN = 0x4e584e43;
const unsigned AdbProtocol::CONNECT_VERSION = 0x01000000;
const unsigned AdbProtocol::CONNECT_MAXDATA = 4096;
const unsigned AdbProtocol::CMD_AUTH = 0x48545541;
const unsigned AdbProtocol::AUTH_TYPE_TOKEN = 1;
const unsigned AdbProtocol::AUTH_TYPE_SIGNATURE = 2;
const unsigned AdbProtocol::AUTH_TYPE_RSA_PUBLIC = 3;
const unsigned AdbProtocol::CMD_OPEN = 0x4e45504f;
const unsigned AdbProtocol::CMD_OKAY = 0x59414b4f;
const unsigned AdbProtocol::CMD_CLSE = 0x45534c43;
const unsigned AdbProtocol::CMD_WRTE = 0x45545257;

std::vector<uint8_t> AdbProtocol::CONNECT_PAYLOAD = {'h','o','s','t',':',':','\0'};

unsigned AdbProtocol::getPayloadChecksum(const std::vector<uint8_t>& payload, unsigned offset) {
    unsigned checksum = 0;
    unsigned length = payload.size();
    for (unsigned i = offset; i < offset + length; ++i) {
        checksum += payload[i] & 0xFF;
    }
    return checksum;
}

std::vector<uint8_t> AdbProtocol::generateMessage(unsigned cmd, unsigned arg0, unsigned arg1, const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> message;

    unsigned dataLength = payload.size();
    unsigned checksum = getPayloadChecksum(payload, 0);
    unsigned magic = cmd ^ 0xFFFFFFFF;

    auto append = [&](unsigned val) {
        message.push_back((val) & 0xFF);
        message.push_back((val >> 8) & 0xFF);
        message.push_back((val >> 16) & 0xFF);
        message.push_back((val >> 24) & 0xFF);
    };

    append(cmd);
    append(arg0);
    append(arg1);
    append(dataLength);
    append(checksum);
    append(magic);

    message.insert(message.end(), payload.begin(), payload.end());
    return message;
}

std::vector<uint8_t> AdbProtocol::generateConnect() {
    return generateMessage(CMD_CNXN, CONNECT_VERSION, CONNECT_MAXDATA, CONNECT_PAYLOAD);
}

std::vector<uint8_t> AdbProtocol::generateAuth(int type, const std::vector<uint8_t>& data) {
    return generateMessage(CMD_AUTH, type, 0, data);
}

std::vector<uint8_t> AdbProtocol::generateOpen(int localId, const std::string& dest) {
    std::vector<uint8_t> destBytes(dest.begin(), dest.end());
    destBytes.push_back(0); // Null terminator
    return generateMessage(CMD_OPEN, localId, 0, destBytes);
}

std::vector<uint8_t> AdbProtocol::generateWrite(int localId, int remoteId, const std::vector<uint8_t>& data) {
    return generateMessage(CMD_WRTE, localId, remoteId, data);
}

std::vector<uint8_t> AdbProtocol::generateClose(int localId, int remoteId) {
    return generateMessage(CMD_CLSE, localId, remoteId, {});
}

std::vector<uint8_t> AdbProtocol::generateReady(int localId, int remoteId) {
    return generateMessage(CMD_OKAY, localId, remoteId, {});
}

std::optional<AdbMessage> AdbProtocol::parseAdbMessage(const std::vector<uint8_t>& buffer, size_t& outMessageLength) {
    //constexpr size_t ADB_HEADER_LENGTH = 24;

    if (buffer.size() < ADB_HEADER_LENGTH) {
        return std::nullopt;  // 数据太少，等更多
    }

    auto extract = [&](int offset) -> uint32_t {
        return buffer[offset] |
               (buffer[offset + 1] << 8) |
               (buffer[offset + 2] << 16) |
               (buffer[offset + 3] << 24);
    };

    AdbMessage msg;
    msg.command = extract(0);
    msg.arg0 = extract(4);
    msg.arg1 = extract(8);
    msg.payloadLength = extract(12);
    msg.checksum = extract(16);
    msg.magic = extract(20);

    outMessageLength = ADB_HEADER_LENGTH + msg.payloadLength;

    // 数据不够一个完整包
    if (buffer.size() < outMessageLength) {
        return std::nullopt;
    }

    if (msg.payloadLength > 0) {
        msg.payload.assign(buffer.begin() + ADB_HEADER_LENGTH,
                           buffer.begin() + ADB_HEADER_LENGTH + msg.payloadLength);
    }

    return msg;
}


void AdbProtocol::printAdbMessage(const AdbMessage& msg) {
    std::cout << "Command: 0x" << std::hex << std::setw(8) << std::setfill('0') << msg.command << std::dec << std::endl;
    std::cout << "Arg0: 0x" << std::hex << std::setw(8) << std::setfill('0') << msg.arg0 << std::dec << std::endl;
    std::cout << "Arg1: 0x" << std::hex << std::setw(8) << std::setfill('0') << msg.arg1 << std::dec << std::endl;
    std::cout << "Payload Length: " << msg.payloadLength << std::endl;
    std::cout << "Checksum: 0x" << std::hex << std::setw(8) << std::setfill('0') << msg.checksum << std::dec << std::endl;
    std::cout << "Magic: 0x" << std::hex << std::setw(8) << std::setfill('0') << msg.magic << std::dec << std::endl;

    std::cout << "Payload: ";
    for (size_t i = 0; i < msg.payload.size(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(msg.payload[i]) << " ";
    }
    std::cout << std::dec << std::endl;
}


// /// 构造 SEND 命令 payload
// std::vector<uint8_t> AdbProtocol::generateSEND(const std::string& remotePathWithMode) {
//     std::vector<uint8_t> payload;
//     uint32_t pathLen = static_cast<uint32_t>(remotePathWithMode.size());

//     // 构造 SEND header (command + path length)
//     payload.push_back('S'); payload.push_back('E');
//     payload.push_back('N'); payload.push_back('D');

//     payload.push_back(pathLen & 0xFF);
//     payload.push_back((pathLen >> 8) & 0xFF);
//     payload.push_back((pathLen >> 16) & 0xFF);
//     payload.push_back((pathLen >> 24) & 0xFF);

//     // 添加路径和权限（如 "/data/local/tmp/test.txt,33206"）
//     payload.insert(payload.end(), remotePathWithMode.begin(), remotePathWithMode.end());

//     return payload;
// }

// /// 构造 DATA 块 payload（每块最大 64K）
// std::vector<uint8_t> AdbProtocol::generateDATA(const std::vector<uint8_t>& buf, size_t len) {
//     std::vector<uint8_t> payload;

//     // 写入 "DATA" 标识
//     payload.push_back('D'); payload.push_back('A');
//     payload.push_back('T'); payload.push_back('A');

//     // 写入数据长度
//     payload.push_back(len & 0xFF);
//     payload.push_back((len >> 8) & 0xFF);
//     payload.push_back((len >> 16) & 0xFF);
//     payload.push_back((len >> 24) & 0xFF);

//     // 附加实际文件内容
//     payload.insert(payload.end(), buf.begin(), buf.begin() + len);

//     return payload;
// }

// /// 构造 DONE payload（mtime 为最后修改时间）
// std::vector<uint8_t> AdbProtocol::generateDONE(uint32_t mtime) {
//     std::vector<uint8_t> payload;

//     payload.push_back('D'); payload.push_back('O');
//     payload.push_back('N'); payload.push_back('E');

//     payload.push_back(mtime & 0xFF);
//     payload.push_back((mtime >> 8) & 0xFF);
//     payload.push_back((mtime >> 16) & 0xFF);
//     payload.push_back((mtime >> 24) & 0xFF);

//     return payload;
// }
