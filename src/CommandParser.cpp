#include "CommandParser.h"
#include <algorithm>
#include <cctype>
#include <sstream>
using namespace std;

CommandParser::CommandParser()
{
    // Initialize available commands (simplified for v2)
    // Property assignment syntax: entity.property(filter) = value
    // Quotes are OPTIONAL for string parameters
    // Examples:
    //   worker.wage(Alice) = 1500
    //   market.price(rice) = 150
    //   farmer.land(Shafin)
    //   system.income_tax_rate = 0.2
    commands = {
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
        {"clear", "Clear the event log", {}},

        // ========== ECONOMIC ANALYSIS COMMANDS ==========
        // Market Analysis
        {"market.equilibrium", "Find market equilibrium", {{"product", "Product name"}}},
        {"market.elasticity", "Calculate price elasticity of demand", {{"product", "Product name"}}},
        {"market.welfare", "Analyze consumer/producer surplus and welfare", {{"product", "Product name"}}},
        {"market.supply_shock", "Simulate supply shock", {{"product", "Product name"}, {"shock_type", "bumper_harvest or natural_disaster"}}},
        {"market.tax", "Apply excise tax", {{"product", "Product name"}, {"tax_rate", "Tax rate (0.0-1.0)"}}},
        {"market.subsidy", "Apply subsidy", {{"product", "Product name"}, {"subsidy_rate", "Subsidy rate (0.0-1.0)"}}},
        {"market.price_control", "Enforce price ceiling or floor", {{"product", "Product name"}, {"control_type", "ceiling or floor"}, {"control_price", "Price level"}}},

        // Consumer Behavior
        {"consumer.optimize_bundle", "Find optimal consumption bundle", {{"budget", "Consumer budget"}}},
        {"consumer.substitute", "Analyze substitute goods", {{"product1", "First product"}, {"product2", "Second product"}}},

        // Firm Production
        {"firm.cost_analysis", "Analyze firm costs", {{"firname", "Firm name"}, {"quantity", "Production quantity"}}},
        {"firm.add_worker", "Add worker and show diminishing returns", {{"firname", "Firm name"}}},
        {"firm.check_shutdown", "Check shutdown rule", {{"firname", "Firm name"}}},

        // Macroeconomic Analysis
        {"gov.calculate_gdp", "Calculate GDP", {{"method", "Expenditure or Income"}}},
        {"stats.cpi", "Analyze Consumer Price Index", {}},
        {"stats.inflation", "Calculate inflation rate", {}},
        {"centralBank.monetary_policy", "Adjust interest rate", {{"new_rate", "New interest rate (0.0-1.0)"}}},
        {"gov.set_policy", "Set fiscal policy", {{"policy_type", "tax or spending"}, {"change", "Change amount"}}},

        // Production Possibility Frontier
        {"economy.ppf", "Analyze production possibilities", {}},
        {"tech.upgrade", "Simulate technological upgrade", {{"sector", "Sector name (e.g., Agriculture)"}}},

        // Variable Tracking System
        {"system.show_changes", "Show recent variable changes", {}},
        {"system.dependency_chain", "Show variable dependency chain", {{"variable", "Variable name"}}}};
}

Command CommandParser::parse(const string &input)
{
    Command cmd;

    string trimmed = trim(input);
    if (trimmed.empty())
    {
        cmd.errorMessage = "Empty command";
        return cmd;
    }

    // Check for assignment syntax: object(params).property = value
    auto findAssignmentEquals = [](const string &text) -> size_t
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
        return string::npos;
    };

    size_t eqPos = findAssignmentEquals(trimmed);
    if (eqPos != string::npos)
    {
        // This is an assignment
        cmd.commandType = Command::Type::Assignment;

        // Left side: object(params).property
        string leftSide = trim(trimmed.substr(0, eqPos));
        // Right side: value
        string rightSide = trim(trimmed.substr(eqPos + 1));

        // Parse the value
        cmd.assignmentValue = parseValue(rightSide);

        // Find the property and object
        size_t dotPos = string::npos;
        size_t parenOpen = leftSide.find('(');
        size_t parenClose = leftSide.rfind(')');

        // New syntax: entity.property(params) = value
        if (parenOpen != string::npos)
        {
            size_t dotBefore = leftSide.find('.');
            if (dotBefore != string::npos && dotBefore < parenOpen)
            {
                dotPos = dotBefore;
                cmd.assignmentProperty = trim(leftSide.substr(dotPos + 1, parenOpen - dotPos - 1));
                string mainPart = leftSide.substr(0, dotPos) + leftSide.substr(parenOpen);

                cmd.name = extractName(mainPart);
                if (cmd.name.empty())
                {
                    cmd.errorMessage = "Invalid object name in assignment";
                    return cmd;
                }

                cmd.params = extractParams(mainPart, cmd.name);
                cmd.valid = true;
                return cmd;
            }
        }

        // Legacy syntax: entity(params).property = value
        if (parenClose != string::npos)
        {
            dotPos = leftSide.find('.', parenClose);
        }
        else
        {
            dotPos = leftSide.find('.');
        }

        if (dotPos == string::npos)
        {
            cmd.errorMessage = "Invalid assignment: missing property";
            return cmd;
        }

        cmd.assignmentProperty = trim(leftSide.substr(dotPos + 1));
        string mainPart = leftSide.substr(0, dotPos);

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

    // Check for property access syntax:
    // New: entity.property(params)
    // Legacy: entity(params).property
    // Simple: system.property
    size_t dotPos = string::npos;
    string mainPart = trimmed;
    size_t parenOpen = trimmed.find('(');
    if (parenOpen != string::npos)
    {
        size_t dotBefore = trimmed.find('.');
        if (dotBefore != string::npos && dotBefore < parenOpen)
        {
            dotPos = dotBefore;
            cmd.propertyAccess = trim(trimmed.substr(dotPos + 1, parenOpen - dotPos - 1));
            mainPart = trimmed.substr(0, dotPos) + trimmed.substr(parenOpen);
        }
    }

    if (dotPos == string::npos)
    {
        size_t parenClose = trimmed.rfind(')');
        if (parenClose != string::npos)
        {
            dotPos = trimmed.find('.', parenClose);
        }
        else
        {
            dotPos = trimmed.find('.');
        }

        if (dotPos != string::npos)
        {
            cmd.propertyAccess = trimmed.substr(dotPos + 1);
            mainPart = trimmed.substr(0, dotPos);
        }
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
    for (const auto &info : commands)
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

vector<string> CommandParser::getSuggestions(const string &partial) const
{
    vector<string> suggestions;
    string lower = partial;
    transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    for (const auto &cmd : commands)
    {
        if (cmd.name.find(lower) == 0 || lower.empty())
        {
            suggestions.push_back(cmd.name);
        }
    }

    return suggestions;
}

string CommandParser::getParameterHints(const string &commandName) const
{
    for (const auto &cmd : commands)
    {
        if (cmd.name == commandName)
        {
            if (cmd.parameters.empty())
            {
                return commandName + "()";
            }

            stringstream ss;
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

string CommandParser::extractName(const string &input) const
{
    size_t parenPos = input.find('(');
    string name;

    if (parenPos != string::npos)
    {
        name = input.substr(0, parenPos);
    }
    else
    {
        // Command without parentheses
        size_t spacePos = input.find(' ');
        if (spacePos != string::npos)
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

map<string, ParamValue> CommandParser::extractParams(const string &input,
                                                     const string &commandName) const
{
    map<string, ParamValue> params;

    const CommandInfo *cmdInfo = findCommandInfo(commandName);
    const auto *paramList = cmdInfo ? &cmdInfo->parameters : nullptr;
    size_t positionalIndex = 0;

    size_t startParen = input.find('(');
    size_t endParen = input.rfind(')');

    if (startParen == string::npos || endParen == string::npos ||
        endParen <= startParen + 1)
    {
        return params; // No parameters
    }

    string paramStr = input.substr(startParen + 1, endParen - startParen - 1);

    // Parse key=value pairs or positional values
    stringstream ss(paramStr);
    string token;

    while (getline(ss, token, ','))
    {
        token = trim(token);
        if (token.empty())
            continue;

        size_t eqPos = token.find('=');
        if (eqPos != string::npos)
        {
            string key = trim(token.substr(0, eqPos));
            string value = trim(token.substr(eqPos + 1));
            params[key] = parseValue(value);
        }
        else if (paramList && positionalIndex < paramList->size())
        {
            const string &key = (*paramList)[positionalIndex].first;
            params[key] = parseValue(token);
            positionalIndex++;
        }
    }

    return params;
}

const CommandInfo *CommandParser::findCommandInfo(const string &commandName) const
{
    for (const auto &cmd : commands)
    {
        if (cmd.name == commandName)
            return &cmd;
    }
    return nullptr;
}

ParamValue CommandParser::parseValue(const string &value) const
{
    if (value.empty())
    {
        return string("");
    }

    // Check for quoted string (quotes are optional, but supported for compatibility)
    if ((value.front() == '"' && value.back() == '"') ||
        (value.front() == '\'' && value.back() == '\''))
    {
        return value.substr(1, value.size() - 2);
    }

    // Check for boolean
    string lower = value;
    transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
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
        if (!isdigit(c))
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
            int intVal = stoi(value);
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
            return stod(value);
        }
        catch (...)
        {
        }
    }

    // Default to unquoted string (quotes are optional!)
    // This allows: farmer.land(Shafin) instead of farmer.land("Shafin")
    return value;
}

string CommandParser::trim(const string &str) const
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}
