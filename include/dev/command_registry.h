#pragma once
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

// Parsed command: argv[0] is the command name, argv[1..] are arguments.
struct ConsoleCmd {
    std::vector<std::string> argv;
};

// Handler receives parsed args and appends output lines to `out`.
using CmdHandler = std::function<void(const ConsoleCmd&, std::vector<std::string>& out)>;

class CommandRegistry {
public:
    void registerCommand(const std::string& name, CmdHandler handler);

    // Tokenizes `line` and dispatches to the matching handler.
    // Returns the output lines produced (may include error messages).
    std::vector<std::string> dispatch(const std::string& line) const;

private:
    std::unordered_map<std::string, CmdHandler> commands_;

    static ConsoleCmd parse(const std::string& line);
};
