#include "CommandExecutor.h"
#include "../utils/Logger.h"
#include <iomanip>
#include <fstream>

CommandExecutor::CommandExecutor(Economy &economy)
    : m_economy(economy)
{
}

bool CommandExecutor::execute(const Command &cmd)
{
    if (!cmd.valid)
    {
        m_lastError = cmd.errorMessage;
        output("Error: " + m_lastError);
        return false;
    }

    try
    {
        if (cmd.name == "adjust_tax")
            cmdAdjustTax(cmd);
        else if (cmd.name == "set_interest")
            cmdSetInterest(cmd);
        else if (cmd.name == "inject_money")
            cmdInjectMoney(cmd);
        else if (cmd.name == "grant_stimulus")
            cmdGrantStimulus(cmd);
        else if (cmd.name == "print_stats")
            cmdPrintStats(cmd);
        else if (cmd.name == "simulate")
            cmdSimulate(cmd);
        else if (cmd.name == "pause")
            cmdPause(cmd);
        else if (cmd.name == "resume")
            cmdResume(cmd);
        else if (cmd.name == "trigger_shock")
            cmdTriggerShock(cmd);
        else if (cmd.name == "add_firms")
            cmdAddFirms(cmd);
        else if (cmd.name == "add_citizens")
            cmdAddCitizens(cmd);
        else if (cmd.name == "export_data")
            cmdExportData(cmd);
        else if (cmd.name == "help")
            cmdHelp(cmd);
        else if (cmd.name == "clear")
            cmdClear(cmd);
        else if (cmd.name == "reset")
            cmdReset(cmd);
        else if (cmd.name == "status")
            cmdStatus(cmd);
        else
        {
            m_lastError = "Unknown command: " + cmd.name;
            output("Error: " + m_lastError);
            return false;
        }
        return true;
    }
    catch (const std::exception &e)
    {
        m_lastError = e.what();
        output("Error: " + m_lastError);
        return false;
    }
}

bool CommandExecutor::execute(const std::string &input)
{
    Command cmd = m_parser.parse(input);
    return execute(cmd);
}

void CommandExecutor::cmdAdjustTax(const Command &cmd)
{
    double rate = getParam<double>(cmd, "rate", -1.0);
    std::string target = getParam<std::string>(cmd, "target", "all");

    if (rate < 0 || rate > 1)
    {
        output("Error: rate must be between 0.0 and 1.0");
        return;
    }

    m_economy.getGovernment().adjustTax(rate, target);
}

void CommandExecutor::cmdSetInterest(const Command &cmd)
{
    double rate = getParam<double>(cmd, "rate", -1.0);

    if (rate < 0 || rate > 0.25)
    {
        output("Error: rate must be between 0.0 and 0.25 (0-25%)");
        return;
    }

    m_economy.getGovernment().setInterestRate(rate);
}

void CommandExecutor::cmdInjectMoney(const Command &cmd)
{
    double amount = getParam<double>(cmd, "amount", 0.0);

    if (amount <= 0)
    {
        output("Error: amount must be positive");
        return;
    }

    m_economy.getGovernment().injectMoney(amount);
}

void CommandExecutor::cmdGrantStimulus(const Command &cmd)
{
    double amount = getParam<double>(cmd, "amount", 0.0);
    std::string sector = getParam<std::string>(cmd, "sector", "all");

    if (amount <= 0)
    {
        output("Error: amount must be positive");
        return;
    }

    m_economy.getGovernment().grantStimulus(amount, sector);
}

void CommandExecutor::cmdPrintStats(const Command &cmd)
{
    std::string variable = getParam<std::string>(cmd, "variable", "all");
    const auto &stats = m_economy.getStats();

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    if (variable == "gdp" || variable == "all")
    {
        ss << "GDP: $" << (stats.gdp / 1e9) << "B";
        if (stats.gdpGrowth != 0)
        {
            ss << " (" << (stats.gdpGrowth >= 0 ? "+" : "")
               << (stats.gdpGrowth * 100) << "%)";
        }
        ss << "\n";
    }

    if (variable == "inflation" || variable == "all")
    {
        ss << "Inflation: " << (stats.inflation * 100) << "% (annual)\n";
        ss << "CPI: " << stats.cpi << "\n";
    }

    if (variable == "unemployment" || variable == "all")
    {
        ss << "Unemployment: " << (stats.unemployment * 100) << "%\n";
        ss << "Employed: " << stats.employed << " / " << stats.population << "\n";
    }

    if (variable == "money" || variable == "all")
    {
        ss << "Money Supply: $" << (stats.moneySupply / 1e12) << "T\n";
        ss << "Interest Rate: " << (stats.interestRate * 100) << "%\n";
    }

    if (variable == "government" || variable == "all")
    {
        ss << "Gov Budget: $" << (stats.budget / 1e9) << "B\n";
        ss << "Gov Debt: $" << (stats.debt / 1e9) << "B\n";
    }

    if (variable == "distribution" || variable == "all")
    {
        ss << "Gini Coefficient: " << stats.giniCoefficient << "\n";
    }

    output(ss.str());
}

void CommandExecutor::cmdSimulate(const Command &cmd)
{
    int ticks = getParam<int>(cmd, "ticks", 1);

    if (ticks <= 0)
    {
        output("Error: ticks must be positive");
        return;
    }

    bool wasRunning = m_economy.isRunning();
    m_economy.setRunning(true);

    for (int i = 0; i < ticks; ++i)
    {
        m_economy.tick();
    }

    if (!wasRunning)
    {
        m_economy.setRunning(false);
    }

    output("Simulated " + std::to_string(ticks) + " tick(s)");
}

void CommandExecutor::cmdPause(const Command &cmd)
{
    m_economy.setRunning(false);
    LOG_INFO("Simulation paused");
}

void CommandExecutor::cmdResume(const Command &cmd)
{
    m_economy.setRunning(true);
    LOG_INFO("Simulation resumed");
}

void CommandExecutor::cmdTriggerShock(const Command &cmd)
{
    std::string type = getParam<std::string>(cmd, "type", "");
    double severity = getParam<double>(cmd, "severity", 1.0);

    if (type.empty())
    {
        output("Error: type is required (pandemic, market_crash, tech_boom, oil_shock)");
        return;
    }

    m_economy.triggerShock(type, severity);
}

void CommandExecutor::cmdAddFirms(const Command &cmd)
{
    int count = getParam<int>(cmd, "count", 1);
    std::string sectorStr = getParam<std::string>(cmd, "sector", "services");

    if (count <= 0)
    {
        output("Error: count must be positive");
        return;
    }

    Sector sector = stringToSector(sectorStr);
    m_economy.addFirm(count, sector);
}

void CommandExecutor::cmdAddCitizens(const Command &cmd)
{
    int count = getParam<int>(cmd, "count", 1);

    if (count <= 0)
    {
        output("Error: count must be positive");
        return;
    }

    m_economy.addCitizen(count);
}

void CommandExecutor::cmdExportData(const Command &cmd)
{
    std::string filename = getParam<std::string>(cmd, "filename", "economy_data.csv");

    const auto &stats = m_economy.getStats();

    std::ofstream file(filename);
    if (!file.is_open())
    {
        output("Error: Could not open file: " + filename);
        return;
    }

    // Write header
    file << "Tick,GDP,Inflation,Unemployment,CPI,MoneySupply,InterestRate,"
         << "Population,Employed,Firms,GovBudget,GovDebt,Gini\n";

    // Write current data
    file << stats.currentTick << ","
         << stats.gdp << ","
         << stats.inflation << ","
         << stats.unemployment << ","
         << stats.cpi << ","
         << stats.moneySupply << ","
         << stats.interestRate << ","
         << stats.population << ","
         << stats.employed << ","
         << stats.firms << ","
         << stats.budget << ","
         << stats.debt << ","
         << stats.giniCoefficient << "\n";

    file.close();
    LOG_SUCCESS("Data exported to " + filename);
}

void CommandExecutor::cmdHelp(const Command &cmd)
{
    std::string cmdName = getParam<std::string>(cmd, "command", "");

    std::stringstream ss;

    if (cmdName.empty())
    {
        ss << "Available commands:\n";
        ss << "--------------------------------------\n";
        for (const auto &info : m_parser.getAvailableCommands())
        {
            ss << "  " << info.name << " - " << info.description << "\n";
        }
        ss << "\nUse help(command=\"name\") for details.";
    }
    else
    {
        for (const auto &info : m_parser.getAvailableCommands())
        {
            if (info.name == cmdName)
            {
                ss << info.name << "\n";
                ss << "  " << info.description << "\n\n";
                if (!info.parameters.empty())
                {
                    ss << "  Parameters:\n";
                    for (const auto &param : info.parameters)
                    {
                        ss << "    " << param.first << ": " << param.second << "\n";
                    }
                }
                ss << "\n  Example: " << m_parser.getParameterHints(cmdName);
                break;
            }
        }
    }

    output(ss.str());
}

void CommandExecutor::cmdClear(const Command &cmd)
{
    Logger::getInstance().clear();
    output("Log cleared");
}

void CommandExecutor::cmdReset(const Command &cmd)
{
    m_economy.reset();
    m_economy.initialize(Config::INITIAL_POPULATION, Config::INITIAL_FIRMS);
    LOG_SUCCESS("Economy reset and reinitialized");
}

void CommandExecutor::cmdStatus(const Command &cmd)
{
    std::stringstream ss;
    ss << "Simulation Status\n";
    ss << "──────────────────────────────────────\n";
    ss << "  Running: " << (m_economy.isRunning() ? "Yes" : "No") << "\n";
    ss << "  Current Tick: " << m_economy.getCurrentTick() << "\n";
    ss << "  Year: " << (m_economy.getCurrentTick() / Config::TICKS_PER_YEAR) << "\n";
    ss << "  Month: " << (m_economy.getCurrentTick() % Config::TICKS_PER_YEAR + 1) << "\n";
    ss << "  Population: " << m_economy.getStats().population << "\n";
    ss << "  Active Firms: " << m_economy.getStats().firms << "\n";
    output(ss.str());
}

template <typename T>
T CommandExecutor::getParam(const Command &cmd, const std::string &name, T defaultValue) const
{
    auto it = cmd.params.find(name);
    if (it == cmd.params.end())
    {
        return defaultValue;
    }

    try
    {
        if constexpr (std::is_same_v<T, int>)
        {
            if (std::holds_alternative<int>(it->second))
            {
                return std::get<int>(it->second);
            }
            else if (std::holds_alternative<double>(it->second))
            {
                return static_cast<int>(std::get<double>(it->second));
            }
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            if (std::holds_alternative<double>(it->second))
            {
                return std::get<double>(it->second);
            }
            else if (std::holds_alternative<int>(it->second))
            {
                return static_cast<double>(std::get<int>(it->second));
            }
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            if (std::holds_alternative<std::string>(it->second))
            {
                return std::get<std::string>(it->second);
            }
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            if (std::holds_alternative<bool>(it->second))
            {
                return std::get<bool>(it->second);
            }
        }
    }
    catch (...)
    {
    }

    return defaultValue;
}

void CommandExecutor::output(const std::string &message)
{
    if (m_outputCallback)
    {
        m_outputCallback(message);
    }
}
