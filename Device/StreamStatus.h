#ifndef STREAMSTATUS_H
#define STREAMSTATUS_H

//流信息
struct StreamStatus {
    bool isStreamable = false;  // 设备是否允许推流
    bool isStreaming = false;   // 设备是否正在推流
};


#endif // STREAMSTATUS_H