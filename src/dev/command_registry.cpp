#include "dev/command_registry.h"
#include <sstream>

void CommandRegistry::registerCommand(const std::string& name, CmdHandler handler) {
    commands_[name] = std::move(handler);
}

ConsoleCmd CommandRegistry::parse(const std::string& line) {
    ConsoleCmd cmd;
    std::istringstream ss(line);
    std::string token;
    while (ss >> token) {
        cmd.argv.push_back(std::move(token));
    }
    return cmd;
}

std::vector<std::string> CommandRegistry::dispatch(const std::string& line) const {
    if (line.empty()) return {};

    ConsoleCmd cmd = parse(line);
    if (cmd.argv.empty()) return {};

    auto it = commands_.find(cmd.argv[0]);
    if (it == commands_.end()) {
        return {"Unknown command: " + cmd.argv[0]};
    }

    std::vector<std::string> out;
    it->second(cmd, out);
    return out;
}
