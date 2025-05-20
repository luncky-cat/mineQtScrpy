#include "shellHandler.h"

#include <qDebug>

#include "protocol/AdbProtocol.h"
#include "context//DeviceContext.h"

shellHandler::shellHandler() {}

bool shellHandler::CommandHandler(ITransPort &transport, DeviceContext &ctx)
{
    if (!ctx.isOpenShell) {
        ctx.isOpenSync=openShell(transport,ctx.local_id,ctx.remote_id,ctx.msg);
        if(!ctx.isOpenSync){
            qDebug()<<"打开shell流失败";
            return false;
        }
    }
    bool result=execShell(transport,ctx.local_id,ctx.remote_id,ctx.cmd.params[0],ctx.msg);
    if(result){
        qDebug()<<"推送文件成功";
    }
    return true;
}

bool shellHandler::openShell(ITransPort &transport, const int local_id, int &remote_id, AdbMessage &out)
{
    auto openShell = AdbProtocol::generateOpen(local_id,"shell:");
    transport.sendMsg(openShell);   //发送连接请求

    if (!transport.waitForCommand(AdbProtocol::CMD_OKAY,out)) {
        qDebug() << "未收到 ok回复";
        return false;
    }

    remote_id = out.arg0;

    if (!transport.waitForCommand(AdbProtocol::CMD_WRTE,out)) {    //等待写
        qDebug() << "未收到回写";
        return false;
    }

    auto okay = AdbProtocol::generateReady(local_id,remote_id);
    transport.sendMsg(okay);
    std::string resultStr(out.payload.begin(),out.payload.end());
    if(resultStr.find("/ $")!=std::string::npos){
        qDebug()<<"找到了回显终端字符"<<resultStr;
        return true;
    }
    return false;
}

bool shellHandler::execShell(ITransPort &transport, const int local_id,const int remote_id,std::string cmd,AdbMessage &out){
    std::vector<std::string> wrtePayloads;
    std::string shellPromptSuffix = "/ $ ";  // 更通用的 shell 提示符
    std::vector<uint8_t> dataPayload(cmd.begin(), cmd.end());  // 转换为字节数组
    auto wrteData = AdbProtocol::generateWrite(local_id,remote_id, dataPayload);
    transport.sendMsg(wrteData); // 发送 WRTE（命令）
    if (!transport.waitForCommand(AdbProtocol::CMD_OKAY,out)) {    //接收ok
        qDebug() << "未收到ok";
        return false;
    }

    while (true) {     //循环接收写

        if (!transport.waitForCommand(AdbProtocol::CMD_OKAY,out)) {    //等待写
            qDebug() << "未收到回写";
            return false;
        }

        auto readyMsg = AdbProtocol::generateReady(local_id,remote_id);
        transport.sendMsg(readyMsg);
        qDebug() << "收到回写 发送 OKAY";
        if(out.payload.empty()){
            qDebug()<<"payload为空,跳过数据后续处理";
            continue;
        }
        // 提取数据
        std::string result(out.payload.begin(), out.payload.end());
        wrtePayloads.push_back(result);
        qDebug() << "payload接收: " << QString::fromStdString(result);

        // 判断接收是否结束
        if (result.find(shellPromptSuffix) != std::string::npos) {
            qDebug() << "检测到提示符，命令执行结束";
            break;
        }
    }


    // std::string serialno = extractShellResult(wrtePayloads,cmd);
    // if (!serialno.empty()) {
    //     qDebug() << "提取执行结果: " << QString::fromStdString(serialno);
    // } else {
    //     qDebug() << "未能提取到执行结果";
    // }

    // QString str=QString::fromStdString(serialno);
    return true;
}
