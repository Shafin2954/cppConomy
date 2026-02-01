#include "CommandParser.h"
#include <algorithm>
#include <cctype>
#include <sstream>

CommandParser::CommandParser()
{
    // Initialize available commands
    m_commands = {
        {"adjust_tax", "Set tax rate for target group", {{"rate", "Tax rate (0.0-1.0)"}, {"target", "\"income\", \"corporate\", or \"all\""}}},

        {"set_interest", "Set central bank interest rate", {{"rate", "Interest rate (0.0-0.25)"}}},

        {"inject_money", "Increase money supply (QE)", {{"amount", "Amount to inject"}}},

        {"grant_stimulus", "Direct payments to citizens/firms", {{"amount", "Total stimulus amount"}, {"sector", "Target sector (optional)"}}},

        {"print_stats", "Display specific statistic", {{"variable", "\"gdp\", \"inflation\", \"unemployment\", \"all\""}}},

        {"simulate", "Run simulation ticks", {{"ticks", "Number of ticks to run"}}},

        {"pause", "Pause simulation", {}},

        {"resume", "Resume simulation", {}},

        {"trigger_shock", "Manually trigger economic shock", {{"type", "\"pandemic\", \"market_crash\", \"tech_boom\", \"oil_shock\""}, {"severity", "Severity multiplier (0.1-2.0)"}}},

        {"add_firms", "Add new firms to economy", {{"count", "Number of firms"}, {"sector", "Sector name"}}},

        {"add_citizens", "Add new citizens to economy", {{"count", "Number of citizens"}}},

        {"export_data", "Export statistics to CSV", {{"filename", "Output filename"}}},

        {"help", "Show help for a command", {{"command", "Command name (optional)"}}},

        {"clear", "Clear the event log", {}},

        {"reset", "Reset the entire simulation", {}},

        {"status", "Show current simulation status", {}}};
}

Command CommandParser::parse(const std::string &input)
{
    Command cmd;

    std::string trimmed = trim(input);
    if (trimmed.empty())
    {
        cmd.errorMessage = "Empty command";
        return cmd;
    }

    // Extract command name
    cmd.name = extractName(trimmed);
    if (cmd.name.empty())
    {
        cmd.errorMessage = "Invalid command format";
        return cmd;
    }

    // Check if command exists
    bool found = false;
    for (const auto &info : m_commands)
    {
        if (info.name == cmd.name)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        cmd.errorMessage = "Unknown command: " + cmd.name;
        return cmd;
    }

    // Extract parameters
    cmd.params = extractParams(trimmed);
    cmd.valid = true;

    return cmd;
}

std::vector<std::string> CommandParser::getSuggestions(const std::string &partial) const
{
    std::vector<std::string> suggestions;
    std::string lower = partial;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    for (const auto &cmd : m_commands)
    {
        if (cmd.name.find(lower) == 0 || lower.empty())
        {
            suggestions.push_back(cmd.name);
        }
    }

    return suggestions;
}

std::string CommandParser::getParameterHints(const std::string &commandName) const
{
    for (const auto &cmd : m_commands)
    {
        if (cmd.name == commandName)
        {
            if (cmd.parameters.empty())
            {
                return commandName + "()";
            }

            std::stringstream ss;
            ss << commandName << "(";
            for (size_t i = 0; i < cmd.parameters.size(); ++i)
            {
                ss << cmd.parameters[i].first << "=...";
                if (i < cmd.parameters.size() - 1)
                {
                    ss << ", ";
                }
            }
            ss << ")";
            return ss.str();
        }
    }
    return "";
}

bool CommandParser::validateCommand(const Command &cmd) const
{
    // Basic validation - could be expanded
    return cmd.valid && !cmd.name.empty();
}

std::string CommandParser::extractName(const std::string &input) const
{
    size_t parenPos = input.find('(');
    std::string name;

    if (parenPos != std::string::npos)
    {
        name = input.substr(0, parenPos);
    }
    else
    {
        // Command without parentheses
        size_t spacePos = input.find(' ');
        if (spacePos != std::string::npos)
        {
            name = input.substr(0, spacePos);
        }
        else
        {
            name = input;
        }
    }

    return trim(name);
}

std::map<std::string, ParamValue> CommandParser::extractParams(const std::string &input) const
{
    std::map<std::string, ParamValue> params;

    size_t startParen = input.find('(');
    size_t endParen = input.rfind(')');

    if (startParen == std::string::npos || endParen == std::string::npos ||
        endParen <= startParen + 1)
    {
        return params; // No parameters
    }

    std::string paramStr = input.substr(startParen + 1, endParen - startParen - 1);

    // Parse key=value pairs
    std::stringstream ss(paramStr);
    std::string token;

    while (std::getline(ss, token, ','))
    {
        token = trim(token);
        if (token.empty())
            continue;

        size_t eqPos = token.find('=');
        if (eqPos == std::string::npos)
            continue;

        std::string key = trim(token.substr(0, eqPos));
        std::string value = trim(token.substr(eqPos + 1));

        params[key] = parseValue(value);
    }

    return params;
}

ParamValue CommandParser::parseValue(const std::string &value) const
{
    if (value.empty())
    {
        return std::string("");
    }

    // Check for quoted string
    if ((value.front() == '"' && value.back() == '"') ||
        (value.front() == '\'' && value.back() == '\''))
    {
        return value.substr(1, value.size() - 2);
    }

    // Check for boolean
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower == "true")
        return true;
    if (lower == "false")
        return false;

    // Check for integer
    bool isInt = true;
    bool hasDecimal = false;
    for (size_t i = 0; i < value.size(); ++i)
    {
        char c = value[i];
        if (i == 0 && c == '-')
            continue;
        if (c == '.')
        {
            hasDecimal = true;
            isInt = false;
            continue;
        }
        if (!std::isdigit(c))
        {
            isInt = false;
            hasDecimal = false;
            break;
        }
    }

    if (isInt && !hasDecimal)
    {
        try
        {
            return std::stoi(value);
        }
        catch (...)
        {
        }
    }

    // Check for double
    if (hasDecimal || isInt)
    {
        try
        {
            return std::stod(value);
        }
        catch (...)
        {
        }
    }

    // Default to string
    return value;
}

std::string CommandParser::trim(const std::string &str) const
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}
