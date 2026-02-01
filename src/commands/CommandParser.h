#pragma once

#include <string>
#include <map>
#include <variant>
#include <vector>
#include <optional>

using ParamValue = std::variant<int, double, std::string, bool>;

struct Command
{
    std::string name;
    std::map<std::string, ParamValue> params;
    bool valid = false;
    std::string errorMessage;
};

struct CommandInfo
{
    std::string name;
    std::string description;
    std::vector<std::pair<std::string, std::string>> parameters; // name, description
};

class CommandParser
{
public:
    CommandParser();

    // Parse input string into Command struct
    Command parse(const std::string &input);

    // Get autocomplete suggestions
    std::vector<std::string> getSuggestions(const std::string &partial) const;

    // Get parameter hints for a command
    std::string getParameterHints(const std::string &commandName) const;

    // Get all available commands
    const std::vector<CommandInfo> &getAvailableCommands() const { return m_commands; }

    // Validate command parameters
    bool validateCommand(const Command &cmd) const;

private:
    std::string extractName(const std::string &input) const;
    std::map<std::string, ParamValue> extractParams(const std::string &input) const;
    ParamValue parseValue(const std::string &value) const;
    std::string trim(const std::string &str) const;

    std::vector<CommandInfo> m_commands;
};
