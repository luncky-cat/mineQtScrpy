#ifndef LOG_H
#define LOG_H

#include <QString>
#include<QDebug>

class Logger {
public:
    // 打印日志的方法，接受一个布尔参数来控制是否输出日志
    void log(const QString& message, bool enabled) {
        if (enabled) {
            qDebug()<< "Log:" << message;
        }
    }
};

#endif // LOG_H
