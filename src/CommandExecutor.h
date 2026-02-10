#pragma once

#include "CommandParser.h"
#include "Simulation.h"
#include <functional>
#include <sstream>

class CommandExecutor
{
public:
    using OutputCallback = std::function<void(const std::string &)>;

    CommandExecutor(Simulation &simulation);

    // Execute a parsed command
    bool execute(const Command &cmd);

    // Execute from std::string input
    bool execute(const std::string &input);

    // Set output callback for results
    void setOutputCallback(OutputCallback callback) { outputCallback = callback; }

    // Get last error message
    const std::string &getLastError() const { return lastError; }

    // Get parser for command info
    const CommandParser &getParser() const { return parser; }

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

    // ========== ECONOMIC ANALYSIS COMMANDS ==========

    // Market mechanisms and equilibrium
    void cmdMarketEquilibrium(const Command &cmd);
    void cmdMarketElasticity(const Command &cmd);
    void cmdMarketWelfare(const Command &cmd);
    void cmdMarketSupplyShock(const Command &cmd);
    void cmdMarketTax(const Command &cmd);
    void cmdMarketSubsidy(const Command &cmd);
    void cmdMarketPriceControl(const Command &cmd);

    // Consumer behavior
    void cmdConsumerOptimize(const Command &cmd);
    void cmdConsumerSubstitute(const Command &cmd);

    // Firm analysis
    void cmdFirmCostAnalysis(const Command &cmd);
    void cmdFirmAddWorker(const Command &cmd);
    void cmdFirmShutdown(const Command &cmd);

    // Macroeconomics
    void cmdGDPCalculate(const Command &cmd);
    void cmdCPIAnalysis(const Command &cmd);
    void cmdInflationAnalysis(const Command &cmd);
    void cmdMonetaryPolicy(const Command &cmd);
    void cmdFiscalPolicy(const Command &cmd);

    // PPF and growth
    void cmdPPFAnalysis(const Command &cmd);
    void cmdTechUpgrade(const Command &cmd);

    // System analysis
    void cmdShowVariableChanges(const Command &cmd);
    void cmdDependencyChain(const Command &cmd);

    // ========== PROPAGATION SYSTEM COMMANDS ==========

    // Advance simulation by one tick (runs full Step())
    void cmdStep(const Command &cmd);

    // Show propagation event history and dependency chains
    void cmdPropagationShow(const Command &cmd);
    void cmdPropagationChain(const Command &cmd);
    void cmdPropagationGraph(const Command &cmd);

    // Helper to get parameter values
    template <typename T>
    T getParam(const Command &cmd, const std::string &name, T defaultValue) const;

    void output(const std::string &message);

    Simulation &simulation;
    CommandParser parser;
    OutputCallback outputCallback;
    std::string lastError;
};
