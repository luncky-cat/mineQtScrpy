#include "shellHandler.h"


#include <regex>
#include <sstream>

#include <qDebug>

#include "protocol/AdbProtocol.h"
#include "context//DeviceContext.h"


shellHandler::Registrar shellHandler::registrar;

shellHandler::shellHandler() {}

bool shellHandler::CommandHandler(ITransPort &transport, DeviceContext &ctx)
{
    qDebug()<<"执行shellHandler";
    qDebug()<<"分配的本地id"<<ctx.shellLocalId;
    if (!ctx.isOpenShell) {
        ctx.isOpenSync=openShell(transport,ctx.shellLocalId,ctx.shellRemoteId,ctx.msg);
        if(!ctx.isOpenSync){
            qDebug()<<"打开shell流失败";
            return false;
        }
    }
    qDebug()<<"打开流成功";
    bool result=execShell(transport,ctx.shellLocalId,ctx.shellRemoteId,ctx.cmd.params[0],ctx.msg);
    if(result){
        qDebug()<<"执行文件成功";
    }
    return true;
}


bool shellHandler::openShell(ITransPort& transport, const int local_id, int& remote_id, AdbMessage& out) {
    // 1. 发送 open shell: 请求
    auto openShell = AdbProtocol::generateOpen(local_id, "shell:");
    transport.sendMsg(openShell);

    // 2. 等待 OKAY 和 WRTE
    bool gotOkay = false;
    bool gotWrte = false;
    std::string wrtePayload;

    while (!(gotOkay && gotWrte)) {
        if (!transport.waitForRecv(out)) {
            qDebug() << "接收失败";
            return false;
        }

        switch (out.command) {
        case AdbProtocol::CMD_OKAY:
            gotOkay = true;
            remote_id = out.arg0;  // 保存 remote_id
            qDebug() << "收到 OKAY，remote_id: " << remote_id;
            break;

        case AdbProtocol::CMD_WRTE:
            gotWrte = true;
            wrtePayload = std::string(out.payload.begin(), out.payload.end());
            qDebug() << "收到 WRTE，内容: " << QString::fromStdString(wrtePayload);
            break;

        default:
            qDebug() << "收到未预期命令：" << out.command;
            break;
        }
    }

    // 3. 回复 OKAY 确认 WRTE
    auto okay = AdbProtocol::generateReady(local_id, remote_id);
    transport.sendMsg(okay);
    qDebug() << "发送 OKAY 响应 WRTE";

    // 4. 检查 shell 提示符是否出现（如 / $ 或 PS 等）
    std::regex pattern(R"(\/\s*\$)");
    if (std::regex_search(wrtePayload, pattern)) {
        qDebug() << "正则找到了回显终端字符" << QString::fromStdString(wrtePayload);
        return true;
    }

    qDebug() << "未识别回显提示符，内容：" << QString::fromStdString(wrtePayload);
    return false;
}


bool shellHandler::execShell(ITransPort& transport, const int local_id, const int remote_id, std::string cmd, AdbMessage& out) {
    qDebug() << "local_id" << local_id << "remote_id" << remote_id;

    std::vector<std::string> wrtePayloads;
    std::regex promptPattern(R"(\/[\s\S]*[\$#]\s*$)");  // 更通用的提示符

    // 添加换行符（shell 通常需要 \n 执行命令）
    if (!cmd.empty() && cmd.back() != '\n') {
        cmd += '\n';
    }

    // 发送 WRTE 命令
    std::vector<uint8_t> dataPayload(cmd.begin(), cmd.end());
    auto wrteData = AdbProtocol::generateWrite(local_id, remote_id, dataPayload);
    transport.sendMsg(wrteData);

    // 等待 OKAY 响应 WRTE（必须）
    if (!transport.waitForCommands({AdbProtocol::CMD_OKAY}, out)) {
        qDebug() << "未收到 WRTE 的 OKAY 响应";
        return false;
    }

    while (true) {
        if (!transport.waitForCommands({AdbProtocol::CMD_WRTE}, out)) {
            qDebug() << "未收到 WRTE 写数据";
            return false;
        }

        std::string result(out.payload.begin(), out.payload.end());
        qDebug() << "payload接收: " << QString::fromStdString(result);
        wrtePayloads.push_back(result);

        // 每个 WRTE 都必须回 OKAY
        auto readyMsg = AdbProtocol::generateReady(local_id, remote_id);
        transport.sendMsg(readyMsg);
        qDebug() << "发送 OKAY 回复 WRTE";

        // 判断是否是命令提示符
        if (std::regex_search(result, promptPattern)) {
            qDebug() << "正则找到了回显终端字符";
            break;
        }
    }

    // 输出完整结果
    for (const auto& str : wrtePayloads) {
        qDebug() << QString::fromStdString(str);
    }

    std::string result = extractShellResult(wrtePayloads);
    qDebug() << "提取到的 shell 结果: " << QString::fromStdString(result);

    return true;
}

std::string shellHandler::extractShellResult(const std::vector<std::string>& payloads) {
    std::string fullOutput;
    for (const auto& line : payloads) {
        fullOutput += line;
    }

    // 分行
    std::istringstream iss(fullOutput);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(iss, line)) {
        // 去除 \r
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        // 忽略空行
        if (!line.empty()) {
            lines.push_back(line);
        }
    }

    // 结果行应在第2行（去除回显和提示符后）
    if (lines.size() >= 2) {
        return lines[1];  // 第 2 行是结果
    }

    return "";  // 提取失败
}
