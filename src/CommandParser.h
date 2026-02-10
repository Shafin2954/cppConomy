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

    string name;
    map<string, ParamValue> params;
    string propertyAccess;          // For syntax like person.age("x") or system.gdp
    Type commandType = Type::Query; // Query or Assignment
    string assignmentProperty;      // Property being assigned to (for assignments)
    ParamValue assignmentValue;     // Value being assigned
    bool valid = false;
    string errorMessage;
};

struct CommandInfo
{
    string name;
    string description;
    vector<pair<string, string>> parameters; // name, description
};

class CommandParser
{
public:
    CommandParser();

    // Parse input string into Command struct
    Command parse(const string &input);

    // Get autocomplete suggestions
    vector<string> getSuggestions(const string &partial) const;

    // Get parameter hints for a command
    string getParameterHints(const string &commandName) const;

    // Get all available commands
    const vector<CommandInfo> &getAvailableCommands() const { return commands; }

    // Validate command parameters
    bool validateCommand(const Command &cmd) const;

private:
    string extractName(const string &input) const;
    map<string, ParamValue> extractParams(const string &input,
                                          const string &commandName) const;
    ParamValue parseValue(const string &value) const;
    string trim(const string &str) const;

    const CommandInfo *findCommandInfo(const string &commandName) const;

    vector<CommandInfo> commands;
};
