#pragma once
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <iomanip>
#include <type_traits>

#include "consumer.h"
#include "laborer.h"
#include "farmer.h"
#include "firm.h"
#include "market.h"
#include "world.h"
#include "cmd.h"
#include "style.h"

using namespace styledTerminal;
using namespace Box;
class cli; // Forward declaration

class cmdExec
{
public:
    using OutputCallback = std::function<void(const std::string &)>;

    cmdExec(world &simulation, std::function<void(const std::string &)> outputFunc) : simulation(simulation), outputCallback(outputFunc) {};

    // Execute a parsed command
    bool execute(const Command &cmd)
    {
        if (!cmd.valid)
        {
            lastError = cmd.errorMessage;
            output("Error: " + lastError);
            return false;
        }

        try
        {
            // Handle property assignments first
            if (cmd.commandType == Command::Type::Assignment)
            {
                return executeAssignment(cmd);
            }

            if (cmd.name == "consumers")
                cmdConsumers(cmd);
            else if (cmd.name == "laborers")
                cmdLaborers(cmd);
            else if (cmd.name == "farmers")
                cmdFarmers(cmd);
            else if (cmd.name == "firms")
                cmdFirms(cmd);
            else if (cmd.name == "markets")
                cmdMarkets(cmd);
            else if (cmd.name == "products")
                cmdProducts(cmd);
            else if (cmd.name == "add_consumer")
                cmdAddConsumer(cmd);
            else if (cmd.name == "add_laborer")
                cmdAddLaborer(cmd);
            else if (cmd.name == "add_farmer")
                cmdAddFarmer(cmd);
            else if (cmd.name == "add_firm")
                cmdAddFirm(cmd);
            else if (cmd.name == "select_consumer")
                cmdSelectConsumer(cmd);
            else if (cmd.name == "select_laborer")
                cmdSelectLaborer(cmd);
            else if (cmd.name == "select_farmer")
                cmdSelectFarmer(cmd);
            else if (cmd.name == "select_market")
                cmdSelectMarket(cmd);
            else if (cmd.name == "clear_selection")
                cmdClearSelection(cmd);
            else if (cmd.name == "market_equilibrium")
                cmdMarketEquilibrium(cmd);
            else if (cmd.name == "market_demand")
                cmdMarketDemand(cmd);
            else if (cmd.name == "market_supply")
                cmdMarketSupply(cmd);
            else if (cmd.name == "consumer_mu")
                cmdConsumerMU(cmd);
            else if (cmd.name == "consumer_surplus")
                cmdConsumerSurplus(cmd);
            else if (cmd.name == "consumer_details")
                cmdConsumerDetails(cmd);
            else if (cmd.name == "consumer_substitution")
                cmdConsumerSubstitution(cmd);
            else if (cmd.name == "consumer_needs")
                cmdConsumerNeeds(cmd);
            else if (cmd.name == "consumer_demand_curve")
                cmdConsumerDemandCurve(cmd);
            else if (cmd.name == "farmer_supply")
                cmdFarmerSupply(cmd);
            else if (cmd.name == "farmer_details")
                cmdFarmerDetails(cmd);
            else if (cmd.name == "farmer_crops")
                cmdFarmerCrops(cmd);
            else if (cmd.name == "farmer_upgrade")
                cmdFarmerUpgrade(cmd);
            else if (cmd.name == "farmer_weather")
                cmdFarmerWeather(cmd);
            else if (cmd.name == "firm_costs")
                cmdFirmCosts(cmd);
            else if (cmd.name == "firm_output")
                cmdFirmOutput(cmd);
            else if (cmd.name == "firm_mp")
                cmdFirmMP(cmd);
            else if (cmd.name == "firm_efficiency")
                cmdFirmEfficiency(cmd);
            else if (cmd.name == "firm_details")
                cmdFirmDetails(cmd);
            else if (cmd.name == "firm_hire")
                cmdFirmHire(cmd);
            else if (cmd.name == "firm_fire")
                cmdFirmFire(cmd);
            else if (cmd.name == "firm_capital")
                cmdFirmCapital(cmd);
            else if (cmd.name == "pass_day")
                cmdPassDay(cmd);
            else if (cmd.name == "status")
                cmdStatus(cmd);
            else if (cmd.name == "help")
                cmdHelp(cmd);
            else if (cmd.name == "clear")
                cmdClear(cmd);
            else if (cmd.name == "exit")
            {
                output("Exiting simulation...");
                return true;
            }
            else
            {
                lastError = "Unknown command: " + cmd.name;
                output("Error: " + lastError);
                return false;
            }
            return true;
        }
        catch (const std::bad_variant_access &e)
        {
            lastError = std::string("Parameter type mismatch: ") + cmd.name + " " + e.what();
            output("Error: " + lastError);
            return false;
        }
        catch (const std::out_of_range &e)
        {
            lastError = std::string("Parameter access error: ") + e.what();
            output("Error: " + lastError);
            return false;
        }
        catch (const std::exception &e)
        {
            lastError = e.what();
            output("Error: " + lastError);
            return false;
        }
        catch (...)
        {
            lastError = "Unknown error occurred";
            output("Error: " + lastError);
            return false;
        }
    }

    // Execute from std::string input
    bool execute(const std::string &input)
    {
        Command cmd = parser.parse(input);
        return execute(cmd);
    }

    // Set output callback for results
    void setOutputCallback(OutputCallback callback) { outputCallback = callback; }

    // Get last error message
    const std::string &getLastError() const { return lastError; }

    // Get parser for command info
    const CommandParser &getParser() const { return parser; }

    // Property assignment handler
    bool executeAssignment(const Command &cmd)
    {
        // For now, only system properties
        return assignSystemProperty(cmd);
    }

    // Assignment handlers
    bool assignSystemProperty(const Command &cmd)
    {
        // Simple assignments like "gdp = 1000"
        if (cmd.assignmentProperty == "gdp")
        {
            double gdpValue = 0.0;
            bool ok = true;
            if (auto pv = std::get_if<double>(&cmd.assignmentValue))
                gdpValue = *pv;
            else if (auto pi = std::get_if<int>(&cmd.assignmentValue))
                gdpValue = static_cast<double>(*pi);
            else if (auto ps = std::get_if<std::string>(&cmd.assignmentValue))
            {
                try
                {
                    gdpValue = std::stod(*ps);
                }
                catch (...)
                {
                    ok = false;
                }
            }
            else if (auto pb = std::get_if<bool>(&cmd.assignmentValue))
                gdpValue = *pb ? 1.0 : 0.0;
            else
                ok = false;

            if (!ok)
            {
                output("Error: Invalid gdp value");
                return false;
            }

            simulation.currentStats.gdp = gdpValue;
            output("GDP set to " + std::to_string(simulation.currentStats.gdp));
            return true;
        }
        // Add more as needed
        output("Unknown property: " + cmd.assignmentProperty);
        return false;
    }
    // Listing commands
    void cmdConsumers(const Command &cmd);
    void cmdLaborers(const Command &cmd);
    void cmdFarmers(const Command &cmd);
    void cmdFirms(const Command &cmd);
    void cmdMarkets(const Command &cmd);
    void cmdProducts(const Command &cmd);

    // Creation commands
    void cmdAddConsumer(const Command &cmd);
    void cmdAddLaborer(const Command &cmd);
    void cmdAddFarmer(const Command &cmd);
    void cmdAddFirm(const Command &cmd);

    // Selection commands
    void cmdSelectConsumer(const Command &cmd);
    void cmdSelectLaborer(const Command &cmd);
    void cmdSelectFarmer(const Command &cmd);
    void cmdSelectMarket(const Command &cmd);
    void cmdClearSelection(const Command &cmd);

    // Market analysis
    void cmdMarketEquilibrium(const Command &cmd);
    void cmdMarketDemand(const Command &cmd);
    void cmdMarketSupply(const Command &cmd);

    // Consumer analysis
    void cmdConsumerMU(const Command &cmd);
    void cmdConsumerSurplus(const Command &cmd);
    void cmdConsumerSubstitution(const Command &cmd);
    void cmdConsumerNeeds(const Command &cmd);
    void cmdConsumerDetails(const Command &cmd);
    void cmdConsumerDemandCurve(const Command &cmd);

    // Farmer analysis
    void cmdFarmerSupply(const Command &cmd);
    void cmdFarmerCrops(const Command &cmd);
    void cmdFarmerUpgrade(const Command &cmd);
    void cmdFarmerWeather(const Command &cmd);
    void cmdFarmerDetails(const Command &cmd);
    void cmdFarmerSupplyCurve(const Command &cmd);

    // Firm analysis
    void cmdFirmCosts(const Command &cmd);
    void cmdFirmOutput(const Command &cmd);
    void cmdFirmMP(const Command &cmd);
    void cmdFirmEfficiency(const Command &cmd);
    void cmdFirmHire(const Command &cmd);
    void cmdFirmCapital(const Command &cmd);
    void cmdFirmDetails(const Command &cmd);
    void cmdFirmFire(const Command &cmd);

    // Simulation
    void cmdPassDay(const Command &cmd);
    void cmdStatus(const Command &cmd);
    void cmdHelp(const Command &cmd);
    void cmdClear(const Command &cmd);

    // Helper to get parameter values
    template <typename T>
    T getParam(const Command &cmd, const std::string &name, T defaultValue) const;

    // Helper to check if parameter exists
    bool hasParam(const Command &cmd, const std::string &name) const
    {
        return cmd.params.find(name) != cmd.params.end();
    }

    // Helper functions
    market *getMarketFromCmd(const Command &cmd);
    product *getProductByName(const std::string &name);
    market *getMarketByProduct(product *p);
    void updateStats();

    void output(const std::string &message)
    {
        if (outputCallback)
            outputCallback(message);
    }

    world &simulation;
    CommandParser parser;
    OutputCallback outputCallback;
    std::string lastError;
};

// Template implementation for parameter retrieval (robust conversion)
template <typename T>
T cmdExec::getParam(const Command &cmd, const std::string &name, T defaultValue) const
{
    auto it = cmd.params.find(name);
    if (it == cmd.params.end())
        return defaultValue;

    const ParamValue &v = it->second;

    // double
    if constexpr (std::is_same_v<T, double>)
    {
        if (auto pv = std::get_if<double>(&v))
            return *pv;
        if (auto pi = std::get_if<int>(&v))
            return static_cast<double>(*pi);
        if (auto ps = std::get_if<std::string>(&v))
        {
            try
            {
                return std::stod(*ps);
            }
            catch (...)
            {
                return defaultValue;
            }
        }
        if (auto pb = std::get_if<bool>(&v))
            return *pb ? 1.0 : 0.0;
    }

    // int
    if constexpr (std::is_same_v<T, int>)
    {
        if (auto pi = std::get_if<int>(&v))
            return *pi;
        if (auto pd = std::get_if<double>(&v))
            return static_cast<int>(*pd);
        if (auto ps = std::get_if<std::string>(&v))
        {
            try
            {
                return static_cast<int>(std::stoi(*ps));
            }
            catch (...)
            {
                return defaultValue;
            }
        }
        if (auto pb = std::get_if<bool>(&v))
            return *pb ? 1 : 0;
    }

    // string
    if constexpr (std::is_same_v<T, std::string>)
    {
        if (auto ps = std::get_if<std::string>(&v))
            return *ps;
        if (auto pi = std::get_if<int>(&v))
            return std::to_string(*pi);
        if (auto pd = std::get_if<double>(&v))
            return std::to_string(*pd);
        if (auto pb = std::get_if<bool>(&v))
            return *pb ? "true" : "false";
    }

    // bool
    if constexpr (std::is_same_v<T, bool>)
    {
        if (auto pb = std::get_if<bool>(&v))
            return *pb;
        if (auto pi = std::get_if<int>(&v))
            return *pi != 0;
        if (auto pd = std::get_if<double>(&v))
            return *pd != 0.0;
        if (auto ps = std::get_if<std::string>(&v))
        {
            return (*ps == "true" || *ps == "1");
        }
    }

    return defaultValue;
}
// ========== COMMAND IMPLEMENTATIONS ==========

// ========== COMMAND IMPLEMENTATIONS ==========

// Listing commands
void cmdExec::cmdConsumers(const Command &cmd)
{
    std::cout << Styled("CONSUMERS (" + std::to_string(simulation.consumers.size()) + ")\n", Theme::Primary);

    for (auto &c : simulation.consumers)
    {
        std::cout << "\n"
                  << c.name << ":\n\t" << Vertical << "ID: " << std::to_string(c.id) << "\n\t" << Vertical << Styled("Age: ", Color::Bold) << std::to_string(c.ageInDays / 365) << "y" << "\n\t" << Vertical << Styled("Savings: ", Color::Bold) << "$" << std::to_string(twoDecimal(c.savings)) << "\n\t" << Vertical << Styled("Alive: ", Color::Bold) << (c.isAlive ? "Yes" : "No") << "\n";
    }
}

void cmdExec::cmdLaborers(const Command &cmd)
{
    std::cout << Styled("LABORERS (" + std::to_string(simulation.laborers.size()) + ")\n", Theme::Primary);

    for (auto &l : simulation.laborers)
    {
        std::cout << "\n"
                  << l.name << ":\n\t" << Vertical << "ID: " << std::to_string(l.id) << "\n\t" << Vertical << Styled("Skill: ", Color::Bold) << std::to_string((int)(l.skillLevel * 100)) << "%" << "\n\t" << Vertical << Styled("Min Wage: ", Color::Bold) << "$" << std::to_string(twoDecimal(l.minWage)) << "/day\n";
    }
}

void cmdExec::cmdFarmers(const Command &cmd)
{
    std::cout << Styled("FARMERS (" + std::to_string(simulation.farmers.size()) + ")\n", Theme::Primary);

    for (auto &f : simulation.farmers)
    {
        std::cout << "\n"
                  << f.name << ":\n\t" << Vertical << "ID: " << std::to_string(f.id) << "\n\t" << Vertical << Styled("Land: ", Color::Bold) << std::to_string(twoDecimal(f.land)) << " acres" << "\n\t" << Vertical << Styled("Tech: ", Color::Bold) << std::to_string(f.techLevel) << "\n\t" << Vertical << Styled("Crops: ", Color::Bold) << std::to_string(twoDecimal(f.crops.size())) << "\n";
    }
}

void cmdExec::cmdFirms(const Command &cmd)
{
    std::cout << Styled("FIRMS (" + std::to_string(simulation.firms.size()) + ")\n", Theme::Primary);

    for (auto &f : simulation.firms)
    {
        std::cout << "\nOwner ID " << std::to_string(f.ownerId) << ":\n\t" << Vertical << Styled("Cash: ", Color::Bold) << "$" << std::to_string(twoDecimal(f.cash)) << "\n\t" << Vertical << Styled("Workers: ", Color::Bold) << std::to_string(f.workers.size()) << "\n\t" << Vertical << Styled("Capital: ", Color::Bold) << std::to_string(f.capitals.size()) << "\n\t" << Vertical << Styled("Output: ", Color::Bold) << std::to_string(twoDecimal(f.currentOutput)) << "\n";
    }
}

void cmdExec::cmdMarkets(const Command &cmd)
{
    std::cout << Styled("MARKETS (" + std::to_string(simulation.markets.size()) + ")\n", Theme::Primary);

    for (auto &m : simulation.markets)
    {
        std::cout << "\n"
                  << m.prod->name << ":\n\t"
                  << Vertical << Styled("Price: ", Color::Bold) << "$" << std::to_string(twoDecimal(m.price)) << "\n\t"
                  << Vertical << Styled("Demand: ", Color::Bold) << "p = " << std::to_string(twoDecimal(m.aggregateDemand.c)) << " - " << std::to_string(twoDecimal(m.aggregateDemand.m)) << "Q" << "\n\t"
                  << Vertical << Styled("Supply: ", Color::Bold) << "p = " << std::to_string(twoDecimal(m.aggregateSupply.c)) << " + " << std::to_string(twoDecimal(m.aggregateSupply.m)) << "Q\n";
    }
}

void cmdExec::cmdProducts(const Command &cmd)
{
    std::cout << Styled("PRODUCTS\n", Theme::Primary);

    std::vector<product *> prods = {&rice, &cloth, &computer, &phone, &car, &steel};
    for (auto *p : prods)
    {
        std::cout << "\n"
                  << p->name << ":\n\t" << Vertical << Styled("Decay Rate: ", Color::Bold) << std::to_string(twoDecimal(p->decayRate)) << "/day\n";
    }
}

// Creation commands
void cmdExec::cmdAddConsumer(const Command &cmd)
{
    if (!hasParam(cmd, "name") || !hasParam(cmd, "age"))
    {
        output("Error: Missing parameters. Usage: add_consumer <name> <age>");
        return;
    }
    std::string name = getParam<std::string>(cmd, "name", std::string());
    int age = getParam<int>(cmd, "age", 0);
    if (name.empty())
    {
        output("Error: Invalid name");
        return;
    }
    simulation.addConsumer(name, age);
    output("Added consumer: " + name);
}

void cmdExec::cmdAddLaborer(const Command &cmd)
{
    if (!hasParam(cmd, "name") || !hasParam(cmd, "age") || !hasParam(cmd, "skill") || !hasParam(cmd, "minwage"))
    {
        output("Error: Missing parameters. Usage: add_laborer <name> <age> <skill> <minwage>");
        return;
    }
    std::string name = getParam<std::string>(cmd, "name", std::string());
    int age = getParam<int>(cmd, "age", 0);
    double skill = getParam<double>(cmd, "skill", 0.0);
    double minwage = getParam<double>(cmd, "minwage", 0.0);
    if (name.empty())
    {
        output("Error: Invalid name");
        return;
    }
    simulation.addlaborer(name, age, skill, minwage);
    output("Added laborer: " + name);
}

void cmdExec::cmdAddFarmer(const Command &cmd)
{
    if (!hasParam(cmd, "name") || !hasParam(cmd, "age") || !hasParam(cmd, "land") || !hasParam(cmd, "tech"))
    {
        output("Error: Missing parameters. Usage: add_farmer <name> <age> <land> <tech>");
        return;
    }
    std::string name = getParam<std::string>(cmd, "name", std::string());
    int age = getParam<int>(cmd, "age", 0);
    double land = getParam<double>(cmd, "land", 0.0);
    double tech = getParam<double>(cmd, "tech", 0.0);
    if (name.empty())
    {
        output("Error: Invalid name");
        return;
    }
    simulation.addFarmer(name, age, land, tech);
    output("Added farmer: " + name);
}

void cmdExec::cmdAddFirm(const Command &cmd)
{
    if (!hasParam(cmd, "ownerid") || !hasParam(cmd, "cash") || !hasParam(cmd, "alpha") || !hasParam(cmd, "beta"))
    {
        output("Error: Missing parameters. Usage: add_firm <ownerid> <cash> <alpha> <beta>");
        return;
    }
    int ownerId = getParam<int>(cmd, "ownerid", 0);
    double cash = getParam<double>(cmd, "cash", 0.0);
    double alpha = getParam<double>(cmd, "alpha", 0.0);
    double beta = getParam<double>(cmd, "beta", 0.0);
    simulation.addFirm(ownerId, cash, cobbDouglas(alpha, beta, 1.0));
    output("Added firm for owner ID: " + std::to_string(ownerId));
}

// Selection commands
void cmdExec::cmdSelectConsumer(const Command &cmd)
{
    if (!hasParam(cmd, "name"))
    {
        output("Error: Missing parameter. Usage: select_consumer(name)");
        return;
    }
    std::string name = getParam<std::string>(cmd, "name", std::string());
    if (name.empty())
    {
        output("Error: Invalid name");
        return;
    }
    for (auto &c : simulation.consumers)
    {
        if (c.name == name)
        {
            simulation.selected_consumer = &c;
            output("Selected consumer: " + name);
            return;
        }
    }
    output("Error: Consumer not found: " + name);
}

void cmdExec::cmdSelectLaborer(const Command &cmd)
{
    if (!hasParam(cmd, "name"))
    {
        output("Error: Missing parameter. Usage: select_laborer <name>");
        return;
    }
    std::string name = getParam<std::string>(cmd, "name", std::string());
    if (name.empty())
    {
        output("Error: Invalid name");
        return;
    }
    for (auto &l : simulation.laborers)
    {
        if (l.name == name)
        {
            simulation.selected_laborer = &l;
            output("Selected laborer: " + name);
            return;
        }
    }
    output("Error: Laborer not found: " + name);
}

void cmdExec::cmdSelectFarmer(const Command &cmd)
{
    if (!hasParam(cmd, "name"))
    {
        output("Error: Missing parameter. Usage: select_farmer <name>");
        return;
    }
    std::string name = getParam<std::string>(cmd, "name", std::string());
    if (name.empty())
    {
        output("Error: Invalid name");
        return;
    }
    for (auto &f : simulation.farmers)
    {
        if (f.name == name)
        {
            simulation.selected_farmer = &f;
            output("Selected farmer: " + name);
            return;
        }
    }
    output("Error: Farmer not found: " + name);
}

void cmdExec::cmdSelectMarket(const Command &cmd)
{
    if (!hasParam(cmd, "product"))
    {
        output("Error: Missing parameter. Usage: select_market <product>");
        return;
    }
    std::string prodName = getParam<std::string>(cmd, "product", std::string());
    if (prodName.empty())
    {
        output("Error: Invalid product name");
        return;
    }
    for (auto &m : simulation.markets)
    {
        if (m.prod->name == prodName)
        {
            simulation.selected_market = &m;
            output("Selected market: " + prodName);
            return;
        }
    }
    output("Error: Market not found: " + prodName);
}

void cmdExec::cmdClearSelection(const Command &cmd)
{
    simulation.selected_consumer = nullptr;
    simulation.selected_laborer = nullptr;
    simulation.selected_farmer = nullptr;
    simulation.selected_market = nullptr;
    output("Selections cleared");
}

// Market analysis
void cmdExec::cmdMarketEquilibrium(const Command &cmd)
{
    market *m = getMarketFromCmd(cmd);
    if (!m)
    {
        output("Error: No market selected");
        return;
    }

    m->calculateAggregateDemand(simulation.consumers);
    m->calculateAggregateSupply(simulation.farmers);

    auto eq = m->findEquilibrium();

    std::stringstream ss;
    ss << "MARKET EQUILIBRIUM: " << m->prod->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Demand Curve: P = " << m->aggregateDemand.c << " - " << m->aggregateDemand.m << "Q\n";
    ss << "Supply Curve: P = " << m->aggregateSupply.c << " + " << m->aggregateSupply.m << "Q\n";
    ss << std::string(40, '-') << "\n";
    ss << "Equilibrium Price:    $" << std::fixed << std::setprecision(2) << eq.price << "\n";
    ss << "Equilibrium Quantity: " << std::fixed << std::setprecision(2) << eq.quantity << " units\n";

    m->price = eq.price;
    output(ss.str());
}

void cmdExec::cmdMarketDemand(const Command &cmd)
{
    market *m = getMarketFromCmd(cmd);
    if (!m)
    {
        output("Error: No market selected");
        return;
    }

    m->calculateAggregateDemand(simulation.consumers);

    std::stringstream ss;
    ss << "AGGREGATE DEMAND: " << m->prod->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Curve: P = " << m->aggregateDemand.c << " - " << m->aggregateDemand.m << "Q\n";
    ss << "\nIndividual Demands:\n";
    for (auto &c : simulation.consumers)
    {
        if (c.dd.find(m->prod) != c.dd.end())
        {
            ss << "  " << c.name << ": P = " << c.dd[m->prod].c
               << " - " << c.dd[m->prod].m << "Q\n";
        }
    }
    output(ss.str());
}

void cmdExec::cmdMarketSupply(const Command &cmd)
{
    market *m = getMarketFromCmd(cmd);
    if (!m)
    {
        output("Error: No market selected");
        return;
    }

    m->calculateAggregateSupply(simulation.farmers);

    std::stringstream ss;
    ss << "AGGREGATE SUPPLY: " << m->prod->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Curve: P = " << m->aggregateSupply.c << " + " << m->aggregateSupply.m << "Q\n";
    ss << "\nIndividual Supplies:\n";
    for (auto &f : simulation.farmers)
    {
        if (f.ss.find(m->prod) != f.ss.end())
        {
            ss << "  " << f.name << ": P = " << f.ss[m->prod].c
               << " + " << f.ss[m->prod].m << "Q"
               << " | Max: " << f.maxOutput[m->prod] << "\n";
        }
    }
    output(ss.str());
}

// Consumer analysis
void cmdExec::cmdConsumerMU(const Command &cmd)
{
    consumer *c = simulation.selected_consumer;
    if (!c)
    {
        output("Error: No consumer selected");
        return;
    }

    if (!hasParam(cmd, "product"))
    {
        output("Error: Missing parameter. Usage: consumer_mu <product>");
        return;
    }
    std::string prodName = getParam<std::string>(cmd, "product", std::string());
    if (prodName.empty())
    {
        output("Error: Invalid product name");
        return;
    }
    product *p = getProductByName(prodName);
    if (!p)
    {
        output("Error: Unknown product");
        return;
    }

    double mu = c->getMarginalUtility(*p);
    double muPerDollar = c->getMUperDollar();

    std::stringstream ss;
    ss << "MARGINAL UTILITY: " << c->name << " → " << prodName << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "MU per Dollar: " << std::fixed << std::setprecision(4) << muPerDollar << "\n";
    ss << "WTP (Willingness to Pay): $" << (c->dd[p].c - c->dd[p].m * c->consumed[p]) << "\n";
    ss << "Consumed so far: " << c->consumed[p] << "\n";
    ss << "Marginal Utility: " << std::fixed << std::setprecision(4) << mu << "\n";
    output(ss.str());
}

void cmdExec::cmdConsumerSurplus(const Command &cmd)
{
    consumer *c = simulation.selected_consumer;
    if (!c)
    {
        output("Error: No consumer selected");
        return;
    }

    if (!hasParam(cmd, "product"))
    {
        output("Error: Missing parameter. Usage: consumer_surplus <product>");
        return;
    }
    std::string prodName = getParam<std::string>(cmd, "product", std::string());
    if (prodName.empty())
    {
        output("Error: Invalid product name");
        return;
    }
    product *p = getProductByName(prodName);
    market *m = getMarketByProduct(p);
    if (!p || !m)
    {
        output("Error: Unknown product/market");
        return;
    }

    double surplus = c->consumerSurplus(*p, m->price);

    std::stringstream ss;
    ss << "CONSUMER SURPLUS: " << c->name << " → " << prodName << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Market Price: $" << m->price << "\n";
    ss << "Quantity Consumed: " << c->consumed[p] << "\n";
    ss << "Consumer Surplus: $" << std::fixed << std::setprecision(2) << surplus << "\n";
    output(ss.str());
}

void cmdExec::cmdConsumerSubstitution(const Command &cmd)
{
    consumer *c = simulation.selected_consumer;
    if (!c)
    {
        output("Error: No consumer selected");
        return;
    }

    std::stringstream ss;
    ss << "SUBSTITUTION RATIOS: " << c->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "(Relative to Rice)\n\n";

    for (auto &need : c->needs)
    {
        double ratio = c->updateSubRatio(need);
        ss << need.name << ": " << std::fixed << std::setprecision(3) << ratio << "\n";
    }
    output(ss.str());
}

void cmdExec::cmdConsumerNeeds(const Command &cmd)
{
    consumer *c = simulation.selected_consumer;
    if (!c)
    {
        output("Error: No consumer selected");
        return;
    }

    std::stringstream ss;
    ss << "NEEDS & CONSUMPTION: " << c->name << "\n";
    ss << std::string(40, '-') << "\n";
    for (auto &need : c->needs)
    {
        product *p = &need;
        ss << need.name
           << " | Demand: P = " << c->dd[p].c << " - " << c->dd[p].m << "Q"
           << " | Consumed: " << c->consumed[p] << "\n";
    }
    output(ss.str());
}

void cmdExec::cmdConsumerDetails(const Command &cmd)
{
    consumer *c = simulation.selected_consumer;
    if (!c)
    {
        output("Error: No consumer selected");
        return;
    }

    std::stringstream ss;
    ss << "CONSUMER DETAILS: " << c->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "ID: " << c->id << "\n";
    ss << "Age: " << (c->ageInDays / 365) << " years\n";
    ss << "Savings: $" << std::fixed << std::setprecision(2) << c->savings << "\n";
    ss << "Consumed:\n";
    for (auto &p : c->consumed)
    {
        if (p.first)
            ss << "  " << p.first->name << ": " << p.second << "\n";
    }
    ss << "\nNeeds and Demand Curves:\n";
    for (auto &need : c->needs)
    {
        product *p = &need;
        ss << "  " << p->name << " -> P = " << c->dd[p].c << " - " << c->dd[p].m << "Q\n";
    }

    output(ss.str());
}

void cmdExec::cmdConsumerDemandCurve(const Command &cmd)
{
    consumer *c = simulation.selected_consumer;
    if (!c)
    {
        output("Error: No consumer selected");
        return;
    }
    if (!hasParam(cmd, "product"))
    {
        output("Error: Missing parameter. Usage: consumer_demand_curve <product>");
        return;
    }
    std::string prodName = getParam<std::string>(cmd, "product", std::string());
    product *p = getProductByName(prodName);
    if (!p)
    {
        output("Error: Unknown product");
        return;
    }

    std::stringstream ss;
    ss << "CONSUMER DEMAND CURVE: " << c->name << " -> " << p->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "P = " << c->dd[p].c << " - " << c->dd[p].m << "Q\n";
    output(ss.str());
}

// Farmer analysis
void cmdExec::cmdFarmerSupply(const Command &cmd)
{
    farmer *f = simulation.selected_farmer;
    if (!f)
    {
        output("Error: No farmer selected");
        return;
    }

    if (!hasParam(cmd, "product") || !hasParam(cmd, "price"))
    {
        output("Error: Missing parameters. Usage: farmer_supply <product> <price>");
        return;
    }
    std::string prodName = getParam<std::string>(cmd, "product", std::string());
    double price = getParam<double>(cmd, "price", 0.0);
    product *p = getProductByName(prodName);
    if (!p)
    {
        output("Error: Unknown product");
        return;
    }

    double qty = f->calculateSupply(p, price);

    std::stringstream ss;
    ss << "SUPPLY CALCULATION: " << f->name << " → " << prodName << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Market Price: $" << price << "\n";
    ss << "Supply Curve: P = " << f->ss[p].c << " + " << f->ss[p].m << "Q\n";
    ss << " | Tax: $" << f->tax << "\n";
    ss << "Tech Level: " << f->techLevel << " | Weather: " << std::fixed << std::setprecision(2) << f->weather << "\n";
    ss << "Max Output: " << f->maxOutput[p] << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Quantity Supplied: " << std::fixed << std::setprecision(2) << qty << " units\n";
    output(ss.str());
}

void cmdExec::cmdFarmerCrops(const Command &cmd)
{
    farmer *f = simulation.selected_farmer;
    if (!f)
    {
        output("Error: No farmer selected");
        return;
    }

    std::stringstream ss;
    ss << "CROPS: " << f->name << "\n";
    ss << std::string(50, '-') << "\n";
    for (auto &crop : f->crops)
    {
        product *p = &crop;
        ss << crop.name << "\n";
        ss << "  Supply: P = " << f->ss[p].c << " + " << f->ss[p].m << "Q\n";
        ss << "  Growth: " << f->growthRate[p] << "/day | Decay: " << f->decay[p] << "/day\n";
        ss << "  Max Output: " << f->maxOutput[p] << " units\n\n";
    }
    output(ss.str());
}

void cmdExec::cmdFarmerUpgrade(const Command &cmd)
{
    farmer *f = simulation.selected_farmer;
    if (!f)
    {
        output("Error: No farmer selected");
        return;
    }

    if (!hasParam(cmd, "level"))
    {
        output("Error: Missing parameter. Usage: farmer_upgrade <level>");
        return;
    }
    double level = getParam<double>(cmd, "level", f->techLevel);
    double oldLevel = f->techLevel;
    f->upgradeTech(level);

    std::stringstream ss;
    ss << "TECH UPGRADE: " << f->name << "\n";
    ss << "Tech Level: " << oldLevel << " → " << level << "\n";
    output(ss.str());
}

void cmdExec::cmdFarmerWeather(const Command &cmd)
{
    farmer *f = simulation.selected_farmer;
    if (!f)
    {
        output("Error: No farmer selected");
        return;
    }

    std::stringstream ss;
    ss << "WEATHER: " << f->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Current Weather Factor: " << std::fixed << std::setprecision(2) << f->weather << "\n";
    ss << "(0 = Perfect, 1 = Severe conditions)\n";
    ss << "Effect: Slope multiplier on supply curve\n";
    output(ss.str());
}

void cmdExec::cmdFarmerDetails(const Command &cmd)
{
    farmer *f = simulation.selected_farmer;
    if (!f)
    {
        output("Error: No farmer selected");
        return;
    }

    std::stringstream ss;
    ss << "FARMER DETAILS: " << f->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "ID: " << f->id << "\n";
    ss << "Land: " << std::fixed << std::setprecision(2) << f->land << " acres\n";
    ss << "Tech Level: " << f->techLevel << "\n";
    ss << "Crops:\n";
    for (auto &c : f->crops)
    {
        ss << "  " << c.name << " (Max: " << f->maxOutput[&c] << ")\n";
    }
    output(ss.str());
}

void cmdExec::cmdFarmerSupplyCurve(const Command &cmd)
{
    farmer *f = simulation.selected_farmer;
    if (!f)
    {
        output("Error: No farmer selected");
        return;
    }
    if (!hasParam(cmd, "product"))
    {
        output("Error: Missing parameter. Usage: farmer_supply_curve <product>");
        return;
    }
    std::string prodName = getParam<std::string>(cmd, "product", std::string());
    product *p = getProductByName(prodName);
    if (!p)
    {
        output("Error: Unknown product");
        return;
    }

    std::stringstream ss;
    ss << "FARMER SUPPLY CURVE: " << f->name << " -> " << p->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "P = " << f->ss[p].c << " + " << f->ss[p].m << "Q\n";
    ss << "Max Output: " << f->maxOutput[p] << "\n";
    output(ss.str());
}

// Firm analysis
void cmdExec::cmdFirmCosts(const Command &cmd)
{
    if (simulation.firms.empty())
    {
        output("Error: No firms");
        return;
    }
    firm *f = &simulation.firms[0];

    f->calculateCosts();

    std::stringstream ss;
    ss << "FIRM COST ANALYSIS (Owner ID: " << f->ownerId << ")\n";
    ss << std::string(50, '-') << "\n";
    ss << "Workers: " << f->workers.size() << " | Capital: " << f->capitals.size() << "\n";
    ss << "Current Output (Q): " << std::fixed << std::setprecision(2) << f->currentOutput << "\n\n";
    ss << "COSTS:\n";
    ss << "  Total Fixed Cost (TFC):     $" << f->totalFixedCost << "\n";
    ss << "  Total Variable Cost (TVC):  $" << f->totalVariableCost << "\n";
    ss << "  Total Cost (TC):            $" << f->totalCost << "\n\n";
    ss << "AVERAGES:\n";
    ss << "  Average Fixed Cost (AFC):   $" << f->averageFixedCost << "\n";
    ss << "  Average Variable Cost (AVC):$" << f->averageVariableCost << "\n";
    ss << "  Average Cost (AC):          $" << f->averageCost << "\n\n";
    ss << "MARGINAL:\n";
    ss << "  Marginal Cost (MC):         $" << f->marginalCost << "\n";

    if (f->marginalCost < f->averageCost)
    {
        ss << "\n→ Economies of Scale (MC < AC)\n";
    }
    else
    {
        ss << "\n→ Diminishing Returns (MC > AC)\n";
    }
    output(ss.str());
}

void cmdExec::cmdFirmOutput(const Command &cmd)
{
    if (simulation.firms.empty())
    {
        output("Error: No firms");
        return;
    }
    firm *f = &simulation.firms[0];

    double L = f->workers.size();
    double K = f->capitals.size();
    double Q = f->prodFunc->output(L, K);

    std::stringstream ss;
    ss << "FIRM OUTPUT (Owner ID: " << f->ownerId << ")\n";
    ss << std::string(40, '-') << "\n";
    ss << "Labor (L): " << (int)L << " workers\n";
    ss << "Capital (K): " << (int)K << " units\n";
    ss << "Production Function: Cobb-Douglas\n";
    ss << "  α = " << f->cdProd.alpha << ", β = " << f->cdProd.beta << ", A = " << f->cdProd.tech << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Output (Q): " << std::fixed << std::setprecision(2) << Q << " units\n";
    output(ss.str());
}

void cmdExec::cmdFirmMP(const Command &cmd)
{
    if (simulation.firms.empty())
    {
        output("Error: No firms");
        return;
    }
    firm *f = &simulation.firms[0];

    double mpL = f->MPofLabor();
    double mpK = f->MPofCapital();

    std::stringstream ss;
    ss << "MARGINAL PRODUCTS (Owner ID: " << f->ownerId << ")\n";
    ss << std::string(40, '-') << "\n";
    ss << "Marginal Product of Labor (MPL):   " << std::fixed << std::setprecision(4) << mpL << "\n";
    ss << "  → Adding 1 worker increases output by " << mpL << " units\n\n";
    ss << "Marginal Product of Capital (MPK): " << std::fixed << std::setprecision(4) << mpK << "\n";
    ss << "  → Adding 1 machine increases output by " << mpK << " units\n";
    output(ss.str());
}

void cmdExec::cmdFirmEfficiency(const Command &cmd)
{
    if (simulation.firms.empty())
    {
        output("Error: No firms");
        return;
    }
    firm *f = &simulation.firms[0];

    auto ratios = f->marginalCosts();

    std::stringstream ss;
    ss << "FACTOR EFFICIENCY (Owner ID: " << f->ownerId << ")\n";
    ss << std::string(40, '-') << "\n";
    ss << "Labor Efficiency (MPL/w):   " << std::fixed << std::setprecision(4) << ratios[0] << "\n";
    ss << "Capital Efficiency (MPK/r): " << std::fixed << std::setprecision(4) << ratios[1] << "\n\n";

    double diff = std::abs(ratios[0] - ratios[1]);
    if (diff < 0.05)
    {
        ss << "STATUS: Optimal Mix (Isoquant tangent to Isocost)\n";
    }
    else if (ratios[0] > ratios[1])
    {
        ss << "RECOMMENDATION: Labor is more efficient. HIRE WORKER.\n";
    }
    else
    {
        ss << "RECOMMENDATION: Capital is more efficient. BUY MACHINE.\n";
    }
    output(ss.str());
}

void cmdExec::cmdFirmDetails(const Command &cmd)
{
    if (simulation.firms.empty())
    {
        output("Error: No firms");
        return;
    }
    firm *f = &simulation.firms[0];

    std::stringstream ss;
    ss << "FIRM DETAILS (Owner ID: " << f->ownerId << ")\n";
    ss << std::string(40, '-') << "\n";
    ss << "Cash: $" << std::fixed << std::setprecision(2) << f->cash << "\n";
    ss << "Workers: " << f->workers.size() << "\n";
    ss << "Capital units: " << f->capitals.size() << "\n";
    ss << "Current Output: " << std::fixed << std::setprecision(2) << f->currentOutput << "\n";
    output(ss.str());
}

void cmdExec::cmdFirmFire(const Command &cmd)
{
    if (simulation.firms.empty())
    {
        output("Error: No firms");
        return;
    }
    firm *f = &simulation.firms[0];

    if (!hasParam(cmd, "laborer"))
    {
        output("Error: Missing parameter. Usage: firm_fire <laborer>");
        return;
    }
    std::string name = getParam<std::string>(cmd, "laborer", std::string());
    if (name.empty())
    {
        output("Error: Invalid laborer name");
        return;
    }

    for (auto it = f->workers.begin(); it != f->workers.end(); ++it)
    {
        if (it->name == name)
        {
            f->workers.erase(it);
            f->calculateCosts();
            output("Fired " + name + ". New output: " + std::to_string(f->currentOutput));
            return;
        }
    }
    output("Error: Laborer not found in firm: " + name);
}

void cmdExec::cmdFirmHire(const Command &cmd)
{
    if (simulation.firms.empty())
    {
        output("Error: No firms");
        return;
    }
    firm *f = &simulation.firms[0];

    if (!hasParam(cmd, "laborer"))
    {
        output("Error: Missing parameter. Usage: firm_hire <laborer>");
        return;
    }
    std::string name = getParam<std::string>(cmd, "laborer", std::string());
    if (name.empty())
    {
        output("Error: Invalid laborer name");
        return;
    }
    for (auto &l : simulation.laborers)
    {

        if (l.name == name)
        {
            f->workers.push_back(l);
            f->calculateCosts();
            output("Hired " + name + ". New output: " + std::to_string(f->currentOutput));
            return;
        }
    }
    output("Error: Laborer not found: " + name);
}

void cmdExec::cmdFirmCapital(const Command &cmd)
{
    if (simulation.firms.empty())
    {
        output("Error: No firms");
        return;
    }
    firm *f = &simulation.firms[0];

    if (!hasParam(cmd, "rental") || !hasParam(cmd, "eff"))
    {
        output("Error: Missing parameters. Usage: firm_capital <rental> <eff>");
        return;
    }
    double rental = getParam<double>(cmd, "rental", 0.0);
    double eff = getParam<double>(cmd, "eff", 0.0);

    f->capitals.emplace_back(rental, eff);
    f->calculateCosts();
    output("Added capital. New output: " + std::to_string(f->currentOutput));
}

void cmdExec::cmdHelp(const Command &cmd)
{
    std::stringstream ss;
    ss << "AVAILABLE COMMANDS\n";
    ss << std::string(50, '=') << "\n\n";

    const auto &commands = parser.getAvailableCommands();
    for (const auto &cmdInfo : commands)
    {
        ss << styledTerminal::Styled(cmdInfo.name, styledTerminal::Theme::Highlight) << "\n";
        ss << "  " << cmdInfo.description << "\n";

        if (!cmdInfo.parameters.empty())
        {
            ss << "  Parameters:\n";
            for (const auto &param : cmdInfo.parameters)
            {
                ss << "    " << param.first << " - " << param.second << "\n";
            }
        }
        ss << "\n";
    }

    ss << "Use 'help <command>' for detailed help on a specific command.\n";
    output(ss.str());
}

void cmdExec::cmdPassDay(const Command &cmd)
{
    double gdp = 0.0; // reset GDP for the new day
    for (auto &c : simulation.consumers)
    {
        gdp += c.incomePerDay * 365;
    }
    for (auto &l : simulation.laborers)
        gdp += l.incomePerDay * 365;
    for (auto &f : simulation.farmers)
        gdp += f.incomePerDay * 365;

    simulation.currentStats.gdp = gdp;
    double perCapita = gdp / simulation.getPopulation();

    for (auto &c : simulation.consumers)
    {
        c.pass_day(perCapita);
    }
    simulation.currentStats.gdp = gdp;
    for (auto &l : simulation.laborers)
        l.pass_day(perCapita);
    for (auto &f : simulation.farmers)
        f.pass_day(perCapita);

    for (auto &m : simulation.markets)
    {
        m.calculateAggregateDemand(simulation.consumers);
        m.calculateAggregateSupply(simulation.farmers);
        auto eq = m.findEquilibrium();
        m.price = eq.price / eq.quantity; // price per unit
    }

    output("Day passed. All entities updated.");
}

void cmdExec::cmdClear(const Command &cmd)
{
    output("Screen cleared");
}

// Helper functions
market *cmdExec::getMarketFromCmd(const Command &cmd)
{
    if (cmd.params.find("product") != cmd.params.end())
    {
        std::string name = getParam<std::string>(cmd, "product", std::string());
        if (name.empty())
            return simulation.selected_market;
        for (auto &m : simulation.markets)
        {
            if (m.prod->name == name)
                return &m;
        }
    }
    return simulation.selected_market;
}

product *cmdExec::getProductByName(const std::string &name)
{
    std::vector<product *> prods = {&rice, &cloth, &computer, &phone, &car, &steel};
    for (auto *p : prods)
    {
        if (p->name == name)
            return p;
    }
    return nullptr;
}

market *cmdExec::getMarketByProduct(product *p)
{
    for (auto &m : simulation.markets)
    {
        if (m.prod == p)
            return &m;
    }
    return nullptr;
}

void cmdExec::cmdStatus(const Command &cmd)
{
    auto stats = simulation.getStats();

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    // Economic Indicators
    ss << "ECONOMIC INDICATORS\n";
    ss << std::string(50, '-') << "\n";
    ss << "GDP: $" << (int)stats.gdp << "\n";
    ss << "GDP Growth: " << stats.gdpGrowth << "%\n";
    ss << "Inflation: " << stats.inflation << "%\n";
    ss << "CPI: " << stats.cpi << "\n";
    ss << "\n";

    // Labor Market
    ss << "LABOR MARKET\n";
    ss << std::string(50, '-') << "\n";
    ss << "Population: " << stats.population << "\n";
    ss << "Employed: " << stats.employed << "\n";
    ss << "Unemployment Rate: " << stats.unemployment << "%\n";
    ss << "\n";

    // Government & Finance
    ss << "GOVERNMENT & FINANCE\n";
    ss << std::string(50, '-') << "\n";
    ss << "Money Supply: $" << (int)stats.moneySupply << "\n";
    ss << "Interest Rate: " << stats.interestRate << "%\n";
    ss << "Budget: $" << (int)stats.budget << "\n";
    ss << "Public Debt: $" << (int)stats.debt << "\n";
    ss << "\n";

    // Inequality
    ss << "INEQUALITY\n";
    ss << std::string(50, '-') << "\n";
    ss << "Gini Coefficient: " << stats.giniCoefficient << "\n";
    ss << "Number of Firms: " << stats.firms << "\n";

    output(ss.str());
}