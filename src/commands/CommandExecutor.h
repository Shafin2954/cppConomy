#pragma once

#include "CommandParser.h"
#include "../core/Simulation.h"
#include <functional>
#include <sstream>

class CommandExecutor
{
public:
    using OutputCallback = std::function<void(const std::string &)>;

    CommandExecutor(Simulation &simulation);

    // Execute a parsed command
    bool execute(const Command &cmd);

    // Execute from string input
    bool execute(const std::string &input);

    // Set output callback for results
    void setOutputCallback(OutputCallback callback) { m_outputCallback = callback; }

    // Get last error message
    const std::string &getLastError() const { return m_lastError; }

    // Get parser for command info
    const CommandParser &getParser() const { return m_parser; }

private:
    // Property assignment handler
    bool executeAssignment(const Command &cmd);

    // Assignment handlers for different entity types
    bool assignWorkerProperty(const Command &cmd, Worker *worker);
    bool assignFarmerProperty(const Command &cmd, Farmer *farmer);
    bool assignOwnerProperty(const Command &cmd, Owner *owner);
    bool assignMarketProperty(const Command &cmd, Market *market);
    bool assignSystemProperty(const Command &cmd);
    // Command handlers
    void cmdAddWorker(const Command &cmd);
    void cmdAddFarmer(const Command &cmd);
    void cmdAddOwner(const Command &cmd);
    void cmdAddMarket(const Command &cmd);
    void cmdSelect(const Command &cmd);
    void cmdClearSelection(const Command &cmd);
    void cmdBuy(const Command &cmd);
    void cmdHarvest(const Command &cmd);
    void cmdReset(const Command &cmd);
    void cmdStatus(const Command &cmd);
    void cmdHelp(const Command &cmd);
    void cmdClear(const Command &cmd);

    // Listing commands
    void cmdPersons(const Command &cmd);
    void cmdWorkers(const Command &cmd);
    void cmdFarmers(const Command &cmd);
    void cmdOwners(const Command &cmd);
    void cmdMarkets(const Command &cmd);

    // Query commands
    void cmdPerson(const Command &cmd);
    void cmdWorker(const Command &cmd);
    void cmdFarmer(const Command &cmd);
    void cmdOwner(const Command &cmd);
    void cmdMarket(const Command &cmd);
    void cmdSystem(const Command &cmd);

    // Helper to get parameter values
    template <typename T>
    T getParam(const Command &cmd, const std::string &name, T defaultValue) const;

    void output(const std::string &message);

    Simulation &m_simulation;
    CommandParser m_parser;
    OutputCallback m_outputCallback;
    std::string m_lastError;
};
