#pragma once

#include <string>
#include <map>
#include <variant>
#include <vector>
#include <optional>

using ParamValue = std::variant<int, double, std::string, bool>;

struct Command
{
    enum class Type
    {
        Query,
        Assignment
    };

    std::string name;
    std::map<std::string, ParamValue> params;
    std::string propertyAccess;     // For syntax like person(name='x').age or system.gdp
    Type commandType = Type::Query; // Query or Assignment
    std::string assignmentProperty; // Property being assigned to (for assignments)
    ParamValue assignmentValue;     // Value being assigned
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
    std::map<std::string, ParamValue> extractParams(const std::string &input,
                                                    const std::string &commandName) const;
    ParamValue parseValue(const std::string &value) const;
    std::string trim(const std::string &str) const;

    const CommandInfo *findCommandInfo(const std::string &commandName) const;

    std::vector<CommandInfo> m_commands;
};
