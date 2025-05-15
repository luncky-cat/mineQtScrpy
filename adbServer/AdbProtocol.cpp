#include "AdbProtocol.h"

#include <iomanip>
#include <iostream>

std::vector<uint8_t> AdbProtocol::CONNECT_PAYLOAD = {'h', 'o', 's', 't', ':', ':', '\0'};

unsigned AdbProtocol::getPayloadChecksum(const std::vector<uint8_t>& payload, unsigned offset) {
    unsigned checksum = 0;
    for (size_t i = offset; i < payload.size(); ++i) {
        checksum += payload[i];
    }
    return checksum;
}

std::vector<uint8_t> AdbProtocol::generateMessage(unsigned cmd, unsigned arg0, unsigned arg1, const std::vector<uint8_t>& payload) {
    std::vector<uint8_t> message(ADB_HEADER_LENGTH + payload.size());
    message[0] = cmd & 0xFF;
    message[1] = (cmd >> 8) & 0xFF;
    message[2] = (cmd >> 16) & 0xFF;
    message[3] = (cmd >> 24) & 0xFF;

    message[4] = arg0 & 0xFF;
    message[5] = (arg0 >> 8) & 0xFF;
    message[6] = (arg0 >> 16) & 0xFF;
    message[7] = (arg0 >> 24) & 0xFF;

    message[8] = arg1 & 0xFF;
    message[9] = (arg1 >> 8) & 0xFF;
    message[10] = (arg1 >> 16) & 0xFF;
    message[11] = (arg1 >> 24) & 0xFF;

    unsigned payloadLength = payload.size();
    message[12] = payloadLength & 0xFF;
    message[13] = (payloadLength >> 8) & 0xFF;
    message[14] = (payloadLength >> 16) & 0xFF;
    message[15] = (payloadLength >> 24) & 0xFF;

    message[16] = getPayloadChecksum(payload, 0) & 0xFF;
    message[17] = (getPayloadChecksum(payload, 0) >> 8) & 0xFF;
    message[18] = (getPayloadChecksum(payload, 0) >> 16) & 0xFF;
    message[19] = (getPayloadChecksum(payload, 0) >> 24) & 0xFF;

    message[20] = 0x50;
    message[21] = 0x4E;
    message[22] = 0x4F;
    message[23] = 0x53;

    std::copy(payload.begin(), payload.end(), message.begin() + ADB_HEADER_LENGTH);

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
    destBytes.push_back(0);
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

AdbProtocol::AdbMessage AdbProtocol::parseAdbMessage(std::vector<uint8_t>& data) {
    if (data.size() < ADB_HEADER_LENGTH) {
        throw std::runtime_error("Data too short to be a valid ADB message");
    }

    AdbMessage msg;
    msg.command = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    msg.arg0 = data[4] | (data[5] << 8) | (data[6] << 16) | (data[7] << 24);
    msg.arg1 = data[8] | (data[9] << 8) | (data[10] << 16) | (data[11] << 24);
    msg.payloadLength = data[12] | (data[13] << 8) | (data[14] << 16) | (data[15] << 24);
    msg.checksum = data[16] | (data[17] << 8) | (data[18] << 16) | (data[19] << 24);
    msg.magic = data[20] | (data[21] << 8) | (data[22] << 16) | (data[23] << 24);

    if (msg.payloadLength > 0) {
        msg.payload.assign(data.begin() + ADB_HEADER_LENGTH, data.begin() + ADB_HEADER_LENGTH + msg.payloadLength);
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
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)msg.payload[i] << " ";
    }
    std::cout << std::dec << std::endl;
}
