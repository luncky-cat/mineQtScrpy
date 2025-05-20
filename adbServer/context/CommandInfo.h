#ifndef COMMANDINFO_H
#define COMMANDINFO_H

#include <string>
#include <vector>

enum class CmdType {
    Unknown,
    Push,
    Shell,
    Pull
};

struct CommandInfo {
    CmdType type = CmdType::Unknown;
    std::vector<std::string> params;
};

#endif // COMMANDINFO_H
