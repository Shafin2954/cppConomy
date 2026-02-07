#include "CommandParser.h"
#include <algorithm>
#include <cctype>
#include <sstream>

CommandParser::CommandParser()
{
    // Initialize available commands (simplified for v2)
    // Property assignment syntax: entity(filter).property = value
    // Examples:
    //   worker(name='Alice').wage = 1500
    //   market(name='rice').price = 150
    //   system.income_tax_rate = 0.2
    m_commands = {
        // Entity listing commands
        {"persons", "List all persons", {}},
        {"workers", "List all workers", {}},
        {"farmers", "List all farmers", {}},
        {"owners", "List all owners", {}},
        {"markets", "List all markets", {}},

        // Entity detail commands (queries)
        {"person", "Get person details", {{"name", "Person name"}}},
        {"worker", "Get worker details", {{"name", "Worker name"}}},
        {"farmer", "Get farmer details", {{"name", "Farmer name"}}},
        {"owner", "Get owner details", {{"name", "Owner name"}}},
        {"market", "Get market details", {{"name", "Market name"}}},

        // System properties (queries)
        {"system", "Get system property", {}},

        // Entity creation
        {"add_worker", "Add a worker", {{"name", "Worker name"}, {"income", "Monthly income"}, {"skill", "Skill level (0.0-1.0)"}}},
        {"add_farmer", "Add a farmer", {{"name", "Farmer name"}, {"land", "Land size"}, {"crop", "Crop name"}}},
        {"add_owner", "Add a business owner", {{"name", "Owner name"}, {"capital", "Initial capital"}, {"product", "Product type"}, {"monopoly", "true/false"}}},
        {"add_market", "Add a market", {{"product", "Product name"}}},

        {"select", "Select entities to show", {{"worker", "Worker name"}, {"farmer", "Farmer name"}, {"owner", "Owner name"}, {"market", "Market name"}}},
        {"clear_selection", "Clear current selection", {}},

        // Actions
        {"buy", "Worker buys product", {{"worker", "Worker name"}, {"product", "Product name"}, {"quantity", "Quantity"}, {"price", "Price"}}},
        {"harvest", "Harvest crops", {{"farmer", "Farmer name (optional)"}}},

        {"reset", "Reset simulation", {}},
        {"status", "Show simulation status", {}},
        {"help", "Show help for a command", {{"command", "Command name (optional)"}}},
        {"clear", "Clear the event log", {}}};
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

    // Check for assignment syntax: object(params).property = value
    auto findAssignmentEquals = [](const std::string &text) -> size_t
    {
        int parenDepth = 0;
        bool inSingleQuote = false;
        bool inDoubleQuote = false;

        for (size_t i = 0; i < text.size(); ++i)
        {
            char c = text[i];
            if (c == '\'' && !inDoubleQuote)
                inSingleQuote = !inSingleQuote;
            else if (c == '"' && !inSingleQuote)
                inDoubleQuote = !inDoubleQuote;
            else if (!inSingleQuote && !inDoubleQuote)
            {
                if (c == '(')
                    parenDepth++;
                else if (c == ')' && parenDepth > 0)
                    parenDepth--;
                else if (c == '=' && parenDepth == 0)
                    return i;
            }
        }
        return std::string::npos;
    };

    size_t eqPos = findAssignmentEquals(trimmed);
    if (eqPos != std::string::npos)
    {
        // This is an assignment
        cmd.commandType = Command::Type::Assignment;

        // Left side: object(params).property
        std::string leftSide = trim(trimmed.substr(0, eqPos));
        // Right side: value
        std::string rightSide = trim(trimmed.substr(eqPos + 1));

        // Parse the value
        cmd.assignmentValue = parseValue(rightSide);

        // Find the property and object
        size_t dotPos = std::string::npos;
        size_t parenClose = leftSide.rfind(')');
        if (parenClose != std::string::npos)
        {
            dotPos = leftSide.find('.', parenClose);
        }
        else
        {
            dotPos = leftSide.find('.');
        }

        if (dotPos == std::string::npos)
        {
            cmd.errorMessage = "Invalid assignment: missing property";
            return cmd;
        }

        cmd.assignmentProperty = trim(leftSide.substr(dotPos + 1));
        std::string mainPart = leftSide.substr(0, dotPos);

        // Extract object name
        cmd.name = extractName(mainPart);
        if (cmd.name.empty())
        {
            cmd.errorMessage = "Invalid object name in assignment";
            return cmd;
        }

        // Extract parameters
        cmd.params = extractParams(mainPart, cmd.name);
        cmd.valid = true;
        return cmd;
    }

    // Check for property access syntax: system.property or name(params).property (queries)
    size_t dotPos = std::string::npos;
    size_t parenClose = trimmed.rfind(')');
    if (parenClose != std::string::npos)
    {
        // Check for dot after closing parenthesis
        dotPos = trimmed.find('.', parenClose);
    }
    else
    {
        // Simple dot access like system.gdp
        dotPos = trimmed.find('.');
    }

    std::string mainPart = trimmed;
    if (dotPos != std::string::npos)
    {
        cmd.propertyAccess = trimmed.substr(dotPos + 1);
        mainPart = trimmed.substr(0, dotPos);
    }

    // Extract command name
    cmd.name = extractName(mainPart);
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
    cmd.params = extractParams(mainPart, cmd.name);
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
                ss << cmd.parameters[i].first;
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

std::map<std::string, ParamValue> CommandParser::extractParams(const std::string &input,
                                                               const std::string &commandName) const
{
    std::map<std::string, ParamValue> params;

    const CommandInfo *cmdInfo = findCommandInfo(commandName);
    const auto *paramList = cmdInfo ? &cmdInfo->parameters : nullptr;
    size_t positionalIndex = 0;

    size_t startParen = input.find('(');
    size_t endParen = input.rfind(')');

    if (startParen == std::string::npos || endParen == std::string::npos ||
        endParen <= startParen + 1)
    {
        return params; // No parameters
    }

    std::string paramStr = input.substr(startParen + 1, endParen - startParen - 1);

    // Parse key=value pairs or positional values
    std::stringstream ss(paramStr);
    std::string token;

    while (std::getline(ss, token, ','))
    {
        token = trim(token);
        if (token.empty())
            continue;

        size_t eqPos = token.find('=');
        if (eqPos != std::string::npos)
        {
            std::string key = trim(token.substr(0, eqPos));
            std::string value = trim(token.substr(eqPos + 1));
            params[key] = parseValue(value);
        }
        else if (paramList && positionalIndex < paramList->size())
        {
            const std::string &key = (*paramList)[positionalIndex].first;
            params[key] = parseValue(token);
            positionalIndex++;
        }
    }

    return params;
}

const CommandInfo *CommandParser::findCommandInfo(const std::string &commandName) const
{
    for (const auto &cmd : m_commands)
    {
        if (cmd.name == commandName)
            return &cmd;
    }
    return nullptr;
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
            int intVal = std::stoi(value);
            // DEBUG: Return as double so it can be extracted as double
            return static_cast<double>(intVal);
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
