#pragma once

#include <string>
#include <map>
#include <variant>
#include <vector>
#include <optional>
#include <sstream>

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
    std::string propertyAccess;     // For syntax like person.age("x") or system.gdp
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
    CommandParser()
    {
        commands = {
            // list commands
            {"consumers", "List all consumers", {}},
            {"laborers", "List all laborers", {}},
            {"farmers", "List all farmers", {}},
            {"firms", "List all firms", {}},
            {"markets", "List all markets", {}},
            {"products", "List all products", {}},

            // add entities
            {"add_consumer(name, age)", "Add a consumer", {{"name", "Name"}, {"age", "Age in years"}}},
            {"add_laborer(name, age, skill, minwage)", "Add a laborer", {{"name", "Name"}, {"age", "Age"}, {"skill", "Skill 0-1"}, {"minwage", "Min wage"}}},
            {"add_farmer(name, age, land, tech)", "Add a farmer", {{"name", "Name"}, {"age", "Age"}, {"land", "Land acres"}, {"tech", "Tech level 0-1"}}},
            {"add_firm(ownerid, cash, alpha, beta)", "Add a firm", {{"ownerid", "Owner consumer ID"}, {"cash", "Initial cash"}, {"alpha", "Cobb-Douglas α"}, {"beta", "Cobb-Douglas β"}}},

            // selection commands
            {"select_consumer(name)", "Select a consumer", {{"name", "Consumer name"}}},
            {"select_laborer(name)", "Select a laborer", {{"name", "Laborer name"}}},
            {"select_farmer(name)", "Select a farmer", {{"name", "Farmer name"}}},
            {"select_market(product)", "Select a market", {{"product", "Product name"}}},
            {"clear_selection", "Clear all selections", {}},

            // query commands -- remove
            {"market_details", "Show market details", {}},

            //
            {"consumer_details", "Show consumer details", {}},
            {"consumer_mu(product)", "Show marginal utility for a product", {{"product", "Product name"}}},
            {"consumer_surplus(product)", "Calculate consumer surplus", {{"product", "Product name"}}},
            {"consumer_substitution", "Show substitution ratios", {}},
            {"consumer_needs", "Show consumer needs and consumption", {}},
            {"consumer_demand_curve(product)", "Show consumer demand curve for a product", {{"product", "Product name"}}},


            {"farmer_details", "Show farmer details", {}},
            {"farmer_supply(product, price)", "Calculate supply at a price", {{"product", "Crop name"}, {"price", "Market price"}}},
            {"farmer_crops", "Show farmer crops and supply curves", {}},
            {"farmer_upgrade(level)", "Upgrade farmer tech level", {{"level", "New tech level 0-1"}}},
            {"farmer_weather", "Show current weather effect", {}},
            {"farmer_supply_curve(product)", "Show farmer's supply curve for a crop", {{"product", "Crop name"}}},


            {"laborer_details", "Show laborer details", {}},

            {"firm_details", "Show firm details", {}},
            {"firm_costs", "Calculate all cost metrics", {}},
            {"firm_output", "Show current production output", {}},
            {"firm_mp", "Show marginal products of L and K", {}},
            {"firm_efficiency", "Show labor vs capital efficiency", {}},
            {"firm_hire(laborer)", "Add a laborer to the firm", {{"laborer", "Laborer name"}}},
            {"firm_fire(laborer)", "Remove a laborer from the firm", {{"laborer", "Laborer name"}}},
            {"firm_capital(rental, eff)", "Add capital to the firm", {{"rental", "Rental rate"}, {"eff", "Efficiency"}}},

            {"pass_day", "Advance simulation by one day", {}},
            {"status", "Show economic statistics", {}},
            {"help", "Show available commands", {}},
            {"clear", "Clear screen", {}},
            {"exit", "Exit simulation", {}}};
    };

    // Parse input std::string into Command struct
    Command parse(const std::string &input)
    {
        Command cmd;
        try
        {
            std::string trimmed = trim(input);
            if (trimmed.empty())
            {
                cmd.valid = false;
                cmd.errorMessage = "Empty input";
                return cmd;
            }

            // Check for assignment syntax: property = value
            size_t eqPos = trimmed.find('=');
            if (eqPos != std::string::npos)
            {
                cmd.commandType = Command::Type::Assignment;
                std::string left = trim(trimmed.substr(0, eqPos));
                std::string right = trim(trimmed.substr(eqPos + 1));

                // Parse assignment value
                cmd.assignmentValue = parseValue(right);

                // For now, simple assignment like "gdp = 1000"
                cmd.assignmentProperty = left;
                cmd.name = "system"; // Generic assignment
                cmd.valid = true;
                return cmd;
            }

            // Regular command parsing
            std::string name = extractName(trimmed);
            cmd.name = name;
            cmd.params = extractParams(trimmed, name);

            // Validate
            cmd.valid = validateCommand(cmd);
            if (!cmd.valid)
            {
                cmd.errorMessage = "Unknown command: "+ trimmed + name;
            }
        }
        catch (const std::exception &e)
        {
            cmd.valid = false;
            cmd.errorMessage = std::string("Parse error: ") + e.what();
        }

        return cmd;
    }

    // Get autocomplete suggestions
    std::vector<std::string> getSuggestions(const std::string &partial) const;

    // Get parameter hints for a command
    std::string getParameterHints(const std::string &commandName) const;

    // Get all available commands
    const std::vector<CommandInfo> &getAvailableCommands() const { return commands; }

    // Validate command parameters
    bool validateCommand(const Command &cmd) const
    {
        // Find command info
        const CommandInfo *info = findCommandInfo(cmd.name);
        if (!info)
        {
            return false; // Unknown command
        }

        // Don't enforce required parameters - let command implementations handle validation
        // This allows for optional parameters

        return true;
    }

    std::vector<std::string> split(const std::string &s, char delimiter) const
    {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter))
        {
            tokens.push_back(token);
        }
        return tokens;
    }

    std::string extractName(const std::string &input) const
    {
        size_t parenPos = input.find('(');

        // If there's a parenthesis, extract name up to it
        if (parenPos != std::string::npos)
        {
            return trim(input.substr(0, parenPos));
        }

        // Otherwise, the whole trimmed input is the command name (no params)
        return trim(input);
    }

    std::map<std::string, ParamValue> extractParams(const std::string &input, const std::string &commandName) const
    {
        std::map<std::string, ParamValue> params;
        const CommandInfo *info = findCommandInfo(commandName);
        if (!info)
            return params;

        std::vector<std::string> values;

        // Check if input has parentheses: command(arg1, arg2)
        size_t parenStart = input.find('(');
        size_t parenEnd = input.find_last_of(')');

        if (parenStart != std::string::npos && parenEnd != std::string::npos && parenEnd > parenStart)
        {
            // Extract content within parentheses
            std::string paramsStr = input.substr(parenStart + 1, parenEnd - parenStart - 1);
            paramsStr = trim(paramsStr);

            // Empty parentheses - no params
            if (paramsStr.empty())
            {
                return params;
            }

            // Split by comma, handling quoted strings
            std::stringstream ss(paramsStr);
            std::string value;
            while (std::getline(ss, value, ','))
            {
                values.push_back(trim(value));
            }
        }
        // No parentheses - command has no parameters
        else
        {
            return params;
        }

        // Map values to parameter names
        size_t paramIndex = 0;
        for (size_t i = 0; i < values.size() && paramIndex < info->parameters.size(); ++i)
        {
            std::string paramName = info->parameters[paramIndex].first;
            params[paramName] = parseValue(values[i]);
            paramIndex++;
        }

        return params;
    }

    ParamValue parseValue(const std::string &value) const
    {
        std::string trimmedValue = trim(value);

        // Remove quotes if present
        if (trimmedValue.size() >= 2 &&
            ((trimmedValue.front() == '"' && trimmedValue.back() == '"') ||
             (trimmedValue.front() == '\'' && trimmedValue.back() == '\'')))
        {
            trimmedValue = trimmedValue.substr(1, trimmedValue.size() - 2);
            return trimmedValue; // Quoted string
        }

        // Try int
        try
        {
            size_t pos;
            int intVal = std::stoi(trimmedValue, &pos);
            if (pos == trimmedValue.size())
                return intVal;
        }
        catch (...)
        {
        }

        // Try double
        try
        {
            size_t pos;
            double doubleVal = std::stod(trimmedValue, &pos);
            if (pos == trimmedValue.size())
                return doubleVal;
        }
        catch (...)
        {
        }

        // Try bool
        if (trimmedValue == "true" || trimmedValue == "1")
            return true;
        if (trimmedValue == "false" || trimmedValue == "0")
            return false;

        // Default to string (unquoted identifiers like "Cauchy")
        return trimmedValue;
    }

    std::string trim(const std::string &str) const
    {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos)
            return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }

    const CommandInfo *findCommandInfo(const std::string &commandName) const
    {
        for (const auto &cmd : commands)
        {
            // Extract base name from registered command (e.g., "firm_hire(laborer)" -> "firm_hire")
            std::string baseName = cmd.name;
            size_t parenPos = baseName.find('(');
            if (parenPos != std::string::npos)
            {
                baseName = baseName.substr(0, parenPos);
            }

            if (baseName == commandName)
                return &cmd;
        }
        return nullptr;
    }

    std::vector<CommandInfo> commands;
};
