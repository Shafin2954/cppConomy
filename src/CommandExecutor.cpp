#include "CommandExecutor.h"
#include "Logger.h"
#include "TermColors.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <variant>

using namespace std;

namespace
{
    string ToLower(string value)
    {
        transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
                  { return static_cast<char>(tolower(c)); });
        return value;
    }

    string NormalizeShockType(string value)
    {
        for (char &c : value)
        {
            if (c == '_')
                c = ' ';
        }
        value = ToLower(value);
        return value;
    }

    bool IsShockType(const string &value)
    {
        string normalized = NormalizeShockType(value);
        return normalized == "bumper harvest" || normalized == "natural disaster";
    }

    string FormatPriceChange(double oldPrice, double newPrice)
    {
        ostringstream ss;
        ss << fixed << setprecision(2);

        if (oldPrice <= 0.0)
        {
            ss << "Price now: $" << newPrice;
            return TermColors::Styled(ss.str(), TermColors::Theme::Info);
        }

        double diff = newPrice - oldPrice;
        double percent = (diff / oldPrice) * 100.0;
        ss << "Price change: $" << oldPrice << " -> $" << newPrice << " (";
        if (diff >= 0.0)
            ss << "+";
        ss << percent << "%)";

        if (diff > 0.0)
            return TermColors::Styled(ss.str(), TermColors::Theme::Error);
        if (diff < 0.0)
            return TermColors::Styled(ss.str(), TermColors::Theme::Success);
        return TermColors::Styled(ss.str(), TermColors::Theme::Info);
    }
}

CommandExecutor::CommandExecutor(Simulation &simulation)
    : simulation(simulation)
{
}

bool CommandExecutor::execute(const Command &cmd)
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

        if (cmd.name == "persons")
            cmdPersons(cmd);
        else if (cmd.name == "workers")
            cmdWorkers(cmd);
        else if (cmd.name == "farmers")
            cmdFarmers(cmd);
        else if (cmd.name == "owners")
            cmdOwners(cmd);
        else if (cmd.name == "markets")
            cmdMarkets(cmd);
        else if (cmd.name == "person")
            cmdPerson(cmd);
        else if (cmd.name == "worker")
            cmdWorker(cmd);
        else if (cmd.name == "farmer")
            cmdFarmer(cmd);
        else if (cmd.name == "owner")
            cmdOwner(cmd);
        else if (cmd.name == "market")
            cmdMarket(cmd);
        else if (cmd.name == "system")
            cmdSystem(cmd);
        else if (cmd.name == "add_worker")
            cmdAddWorker(cmd);
        else if (cmd.name == "add_farmer")
            cmdAddFarmer(cmd);
        else if (cmd.name == "add_owner")
            cmdAddOwner(cmd);
        else if (cmd.name == "add_market")
            cmdAddMarket(cmd);
        else if (cmd.name == "select")
            cmdSelect(cmd);
        else if (cmd.name == "clear_selection")
            cmdClearSelection(cmd);
        else if (cmd.name == "buy")
            cmdBuy(cmd);
        else if (cmd.name == "harvest")
            cmdHarvest(cmd);
        else if (cmd.name == "reset")
            cmdReset(cmd);
        else if (cmd.name == "status")
            cmdStatus(cmd);
        else if (cmd.name == "help")
            cmdHelp(cmd);
        else if (cmd.name == "clear")
            cmdClear(cmd);

        // ========== ECONOMIC ANALYSIS COMMANDS ==========
        else if (cmd.name == "market.equilibrium")
            cmdMarketEquilibrium(cmd);
        else if (cmd.name == "market.elasticity")
            cmdMarketElasticity(cmd);
        else if (cmd.name == "market.welfare")
            cmdMarketWelfare(cmd);
        else if (cmd.name == "market.supply_shock")
            cmdMarketSupplyShock(cmd);
        else if (cmd.name == "market.tax")
            cmdMarketTax(cmd);
        else if (cmd.name == "market.subsidy")
            cmdMarketSubsidy(cmd);
        else if (cmd.name == "market.price_control")
            cmdMarketPriceControl(cmd);
        else if (cmd.name == "consumer.optimize_bundle")
            cmdConsumerOptimize(cmd);
        else if (cmd.name == "consumer.substitute")
            cmdConsumerSubstitute(cmd);
        else if (cmd.name == "firm.cost_analysis")
            cmdFirmCostAnalysis(cmd);
        else if (cmd.name == "firm.add_worker")
            cmdFirmAddWorker(cmd);
        else if (cmd.name == "firm.check_shutdown")
            cmdFirmShutdown(cmd);
        else if (cmd.name == "gov.calculate_gdp")
            cmdGDPCalculate(cmd);
        else if (cmd.name == "stats.cpi")
            cmdCPIAnalysis(cmd);
        else if (cmd.name == "stats.inflation")
            cmdInflationAnalysis(cmd);
        else if (cmd.name == "centralBank.monetary_policy")
            cmdMonetaryPolicy(cmd);
        else if (cmd.name == "gov.set_policy")
            cmdFiscalPolicy(cmd);
        else if (cmd.name == "economy.ppf")
            cmdPPFAnalysis(cmd);
        else if (cmd.name == "tech.upgrade")
            cmdTechUpgrade(cmd);
        else if (cmd.name == "system.show_changes")
            cmdShowVariableChanges(cmd);
        else if (cmd.name == "system.dependency_chain")
            cmdDependencyChain(cmd);
        // ========== PROPAGATION SYSTEM COMMANDS ==========
        else if (cmd.name == "step")
            cmdStep(cmd);
        else if (cmd.name == "propagation.show")
            cmdPropagationShow(cmd);
        else if (cmd.name == "propagation.chain")
            cmdPropagationChain(cmd);
        else if (cmd.name == "propagation.graph")
            cmdPropagationGraph(cmd);
        else
        {
            lastError = "Unknown command: " + cmd.name;
            output("Error: " + lastError);
            return false;
        }
        return true;
    }
    catch (const exception &e)
    {
        lastError = e.what();
        output("Error: " + lastError);
        return false;
    }
}

bool CommandExecutor::execute(const string &input)
{
    Command cmd = parser.parse(input);
    return execute(cmd);
}

void CommandExecutor::cmdAddWorker(const Command &cmd)
{
    string name = getParam<string>(cmd, "name", "Worker");
    double income = getParam<double>(cmd, "income", 1000.0);
    double skill = getParam<double>(cmd, "skill", 0.5);

    simulation.AddWorker(name, income, skill);
    simulation.RefreshStats();
    output("Added worker: " + name);
}

void CommandExecutor::cmdAddFarmer(const Command &cmd)
{
    string name = getParam<string>(cmd, "name", "Farmer");
    double land = getParam<double>(cmd, "land", 10.0);
    string crop = getParam<string>(cmd, "crop", "rice");

    simulation.AddFarmer(name, land, crop);
    simulation.RefreshStats();
    output("Added farmer: " + name);
}

void CommandExecutor::cmdAddOwner(const Command &cmd)
{
    string name = getParam<string>(cmd, "name", "Owner");
    double capital = getParam<double>(cmd, "capital", 5000.0);
    string product = getParam<string>(cmd, "product", "cloth");
    bool monopoly = getParam<bool>(cmd, "monopoly", false);

    simulation.AddOwner(name, capital, product, monopoly);
    simulation.RefreshStats();
    output("Added owner: " + name);
}

void CommandExecutor::cmdAddMarket(const Command &cmd)
{
    string product = getParam<string>(cmd, "product", "rice");
    simulation.CreateMarket(product);
    simulation.RefreshStats();
    output("Added market: " + product);
}

void CommandExecutor::cmdSelect(const Command &cmd)
{
    string worker = getParam<string>(cmd, "worker", "");
    string farmer = getParam<string>(cmd, "farmer", "");
    string owner = getParam<string>(cmd, "owner", "");
    string market = getParam<string>(cmd, "market", "");

    if (!worker.empty())
        simulation.SelectWorker(worker);
    if (!farmer.empty())
        simulation.SelectFarmer(farmer);
    if (!owner.empty())
        simulation.SelectOwner(owner);
    if (!market.empty())
        simulation.SelectMarket(market);

    output("Selection updated");
}

void CommandExecutor::cmdClearSelection(const Command &cmd)
{
    (void)cmd;
    simulation.ClearSelection();
    output("Selection cleared");
}

void CommandExecutor::cmdBuy(const Command &cmd)
{
    string worker = getParam<string>(cmd, "worker", "");
    string product = getParam<string>(cmd, "product", "");
    double qty = getParam<double>(cmd, "quantity", 1.0);
    double price = getParam<double>(cmd, "price", 1.0);

    Worker *w = simulation.FindWorker(worker);
    if (!w)
    {
        output("Error: worker not found");
        return;
    }

    double oldWallet = w->GetWallet();
    double oldUtility = w->GetTotalUtility();
    w->Consume(product, qty, price);
    simulation.RecordVariableChange("worker." + w->GetName() + ".wallet", oldWallet, w->GetWallet());
    simulation.RecordVariableChange("worker." + w->GetName() + ".utility", oldUtility, w->GetTotalUtility());
    simulation.RefreshStats();
    output("Purchase complete");
}

void CommandExecutor::cmdHarvest(const Command &cmd)
{
    string farmer = getParam<string>(cmd, "farmer", "");

    if (farmer.empty())
    {
        for (const auto &f : simulation.GetFarmers())
        {
            double oldOutput = f->GetOutputQuantity();
            f->Harvest();
            simulation.RecordVariableChange("farmer." + f->GetName() + ".output_quantity",
                                            oldOutput, f->GetOutputQuantity());
            // Update market with harvested supply and recalculate equilibrium
            simulation.UpdateMarketFromHarvest(f.get());
        }
        simulation.RecalculateMarketEquilibria();
        simulation.RefreshStats();
        output("All farmers harvested\n");
        output("Markets updated with new supply and equilibrium recalculated");
        return;
    }

    Farmer *f = simulation.FindFarmer(farmer);
    if (!f)
    {
        output("Error: farmer not found");
        return;
    }

    double oldOutput = f->GetOutputQuantity();
    f->Harvest();
    simulation.RecordVariableChange("farmer." + f->GetName() + ".output_quantity",
                                    oldOutput, f->GetOutputQuantity());
    // Update market with harvested supply and recalculate equilibrium
    simulation.UpdateMarketFromHarvest(f);
    simulation.RecalculateMarketEquilibria();
    simulation.RefreshStats();
    output("Harvested for " + farmer + "\n");
    output("Market updated with supply " + to_string(f->GetOutputQuantity()) + " units");
}

void CommandExecutor::cmdReset(const Command &cmd)
{
    (void)cmd;
    simulation.Reset();
    simulation.Initialize(2, 1, 1);
    simulation.RefreshStats();
    output("Simulation reset");
}

void CommandExecutor::cmdStatus(const Command &cmd)
{
    (void)cmd;
    output(simulation.GetStatusString());
}

void CommandExecutor::cmdHelp(const Command &cmd)
{
    string cmdName = getParam<string>(cmd, "command", "");
    stringstream ss;

    if (cmdName.empty())
    {
        ss << "Available commands:\n";
        ss << "--------------------------------------\n";
        for (const auto &info : parser.getAvailableCommands())
            ss << "  " << info.name << " - " << info.description << "\n";
        ss << "\nUse help(command=\"name\") for details.";
    }
    else
    {
        for (const auto &info : parser.getAvailableCommands())
        {
            if (info.name == cmdName)
            {
                ss << info.name << "\n";
                ss << "  " << info.description << "\n\n";
                if (!info.parameters.empty())
                {
                    ss << "  Parameters:\n";
                    for (const auto &param : info.parameters)
                        ss << "    " << param.first << ": " << param.second << "\n";
                }
                ss << "\n  Example: " << parser.getParameterHints(cmdName);
                break;
            }
        }
    }

    output(ss.str());
}

void CommandExecutor::cmdClear(const Command &cmd)
{
    (void)cmd;
    // Screen clearing is handled by CLI::ClearScreen()
    output("Screen cleared");
}

// ========== Listing Commands ==========

void CommandExecutor::cmdPersons(const Command &cmd)
{
    (void)cmd;
    stringstream ss;
    ss << "All Persons:\n";
    ss << "  Workers:\n";
    for (const auto &w : simulation.GetWorkers())
        ss << "    - " << w->GetName() << "\n";
    ss << "  Farmers:\n";
    for (const auto &f : simulation.GetFarmers())
        ss << "    - " << f->GetName() << "\n";
    ss << "  Owners:\n";
    for (const auto &o : simulation.GetOwners())
        ss << "    - " << o->GetName() << "\n";
    output(ss.str());
}

void CommandExecutor::cmdWorkers(const Command &cmd)
{
    (void)cmd;
    stringstream ss;
    ss << "Workers:\n";
    for (const auto &w : simulation.GetWorkers())
    {
        ss << "  - " << w->GetName() << " (Income: $" << w->GetMonthlyIncome()
           << ", Skill: " << w->GetSkillLevel() << ")\n";
    }
    if (simulation.GetWorkers().empty())
        ss << "  (none)\n";
    output(ss.str());
}

void CommandExecutor::cmdFarmers(const Command &cmd)
{
    (void)cmd;
    stringstream ss;
    ss << "Farmers:\n";
    for (const auto &f : simulation.GetFarmers())
    {
        ss << "  - " << f->GetName() << " (Crop: " << f->GetCrop()
           << ", Land: " << f->GetLandSize() << " acres)\n";
    }
    if (simulation.GetFarmers().empty())
        ss << "  (none)\n";
    output(ss.str());
}

void CommandExecutor::cmdOwners(const Command &cmd)
{
    (void)cmd;
    stringstream ss;
    ss << "Owners:\n";
    for (const auto &o : simulation.GetOwners())
    {
        ss << "  - " << o->GetName() << " (Product: " << o->GetProductType()
           << ", Capital: $" << o->GetCapital() << ")\n";
    }
    if (simulation.GetOwners().empty())
        ss << "  (none)\n";
    output(ss.str());
}

void CommandExecutor::cmdMarkets(const Command &cmd)
{
    (void)cmd;
    stringstream ss;
    ss << "Markets:\n";
    for (const auto &[name, m] : simulation.GetMarkets())
    {
        ss << "  - " << name << " (Price: $" << m->GetCurrentPrice() << ")\n";
    }
    if (simulation.GetMarkets().empty())
        ss << "  (none)\n";
    output(ss.str());
}

// ========== Detail/Property Commands ==========

void CommandExecutor::cmdPerson(const Command &cmd)
{
    string name = getParam<string>(cmd, "name", "");
    if (name.empty())
    {
        output("Error: name parameter required");
        return;
    }

    // Try to find as worker, farmer, or owner
    Worker *w = simulation.FindWorker(name);
    Farmer *f = simulation.FindFarmer(name);
    Owner *o = simulation.FindOwner(name);

    Consumer *p = nullptr;
    string type;
    if (w)
    {
        p = w;
        type = "Worker";
    }
    else if (f)
    {
        p = f;
        type = "Farmer";
    }
    else if (o)
    {
        p = o;
        type = "Owner";
    }

    if (!p)
    {
        output("Error: person '" + name + "' not found");
        return;
    }

    if (cmd.propertyAccess.empty())
    {
        // Show all details
        output(p->GetInfoString());
    }
    else
    {
        // Get specific property
        string prop = cmd.propertyAccess;
        stringstream ss;
        if (prop == "name")
            ss << p->GetName();
        else if (prop == "age")
            ss << p->GetAge();
        else if (prop == "income")
            ss << p->GetMonthlyIncome();
        else if (prop == "savings")
            ss << p->GetSavings();
        else if (prop == "type")
            ss << type;
        else
            ss << "Unknown property: " << prop;
        output(ss.str());
    }
}

void CommandExecutor::cmdWorker(const Command &cmd)
{
    string name = getParam<string>(cmd, "name", "");
    if (name.empty())
    {
        output("Error: name parameter required");
        return;
    }

    Worker *w = simulation.FindWorker(name);
    if (!w)
    {
        output("Error: worker '" + name + "' not found");
        return;
    }

    simulation.SelectWorker(name);

    if (cmd.propertyAccess.empty())
    {
        output(w->GetInfoString());
    }
    else
    {
        string prop = cmd.propertyAccess;
        stringstream ss;
        if (prop == "name")
            ss << w->GetName();
        else if (prop == "age")
            ss << w->GetAge();
        else if (prop == "income")
            ss << w->GetMonthlyIncome();
        else if (prop == "wage")
            ss << w->GetCurrentWage();
        else if (prop == "skill")
            ss << w->GetSkillLevel();
        else if (prop == "savings")
            ss << w->GetSavings();
        else if (prop == "employed")
            ss << (w->IsEmployed() ? "true" : "false");
        else if (prop == "employer")
            ss << w->GetEmployer();
        else
            ss << "Unknown property: " << prop;
        output(ss.str());
    }
}

void CommandExecutor::cmdFarmer(const Command &cmd)
{
    string name = getParam<string>(cmd, "name", "");
    if (name.empty())
    {
        output("Error: name parameter required");
        return;
    }

    Farmer *f = simulation.FindFarmer(name);
    if (!f)
    {
        output("Error: farmer '" + name + "' not found");
        return;
    }

    simulation.SelectFarmer(name);

    if (cmd.propertyAccess.empty())
    {
        output(f->GetInfoString());
    }
    else
    {
        string prop = cmd.propertyAccess;
        stringstream ss;
        if (prop == "name")
            ss << f->GetName();
        else if (prop == "age")
            ss << f->GetAge();
        else if (prop == "income")
            ss << f->GetMonthlyIncome();
        else if (prop == "savings")
            ss << f->GetSavings();
        else if (prop == "crop")
            ss << f->GetCrop();
        else if (prop == "land")
            ss << f->GetLandSize();
        else if (prop == "output")
            ss << f->GetOutputQuantity();
        else if (prop == "profit")
            ss << f->GetProfit();
        else if (prop == "revenue")
            ss << f->GetRevenue();
        else
            ss << "Unknown property: " << prop;
        output(ss.str());
    }
}

void CommandExecutor::cmdOwner(const Command &cmd)
{
    string name = getParam<string>(cmd, "name", "");
    if (name.empty())
    {
        output("Error: name parameter required");
        return;
    }

    Owner *o = simulation.FindOwner(name);
    if (!o)
    {
        output("Error: owner '" + name + "' not found");
        return;
    }

    simulation.SelectOwner(name);

    if (cmd.propertyAccess.empty())
    {
        output(o->GetInfoString());
    }
    else
    {
        string prop = cmd.propertyAccess;
        stringstream ss;
        if (prop == "name")
            ss << o->GetName();
        else if (prop == "age")
            ss << o->GetAge();
        else if (prop == "income")
            ss << o->GetMonthlyIncome();
        else if (prop == "savings")
            ss << o->GetSavings();
        else if (prop == "capital")
            ss << o->GetCapital();
        else if (prop == "product")
            ss << o->GetProductType();
        else if (prop == "production")
            ss << o->GetProduction();
        else if (prop == "profit")
            ss << o->GetProfit();
        else if (prop == "price")
            ss << o->GetPrice();
        else if (prop == "monopoly")
            ss << (o->IsMonopoly() ? "true" : "false");
        else
            ss << "Unknown property: " << prop;
        output(ss.str());
    }
}

void CommandExecutor::cmdMarket(const Command &cmd)
{
    string name = getParam<string>(cmd, "name", "");
    if (name.empty())
    {
        output("Error: name parameter required");
        return;
    }

    Market *m = simulation.FindMarket(name);
    if (!m)
    {
        output("Error: market '" + name + "' not found");
        return;
    }

    simulation.SelectMarket(name);

    if (cmd.propertyAccess.empty())
    {
        stringstream ss;
        ss << "Market: " << m->GetProductName() << "\n";
        ss << "  Price: $" << m->GetCurrentPrice() << "\n";
        ss << "  Demand: " << m->GetQuantityDemanded() << "\n";
        ss << "  Supply: " << m->GetQuantitySupplied() << "\n";
        ss << "  Shortage/Surplus: " << m->GetShortageSurplus() << "\n";
        ss << "  Tax Rate: " << (m->GetTaxRate() * 100) << "%\n";
        output(ss.str());
    }
    else
    {
        string prop = cmd.propertyAccess;
        stringstream ss;
        if (prop == "name" || prop == "product")
            ss << m->GetProductName();
        else if (prop == "price")
            ss << m->GetCurrentPrice();
        else if (prop == "demand")
            ss << m->GetQuantityDemanded();
        else if (prop == "supply")
            ss << m->GetQuantitySupplied();
        else if (prop == "shortage" || prop == "surplus")
            ss << m->GetShortageSurplus();
        else if (prop == "tax")
            ss << m->GetTaxRate();
        else if (prop == "sales")
            ss << m->GetTotalSalesValue();
        else
            ss << "Unknown property: " << prop;
        output(ss.str());
    }
}

void CommandExecutor::cmdSystem(const Command &cmd)
{
    const auto &stats = simulation.GetStats();

    if (cmd.propertyAccess.empty())
    {
        stringstream ss;
        ss << "System Stats:\n";
        ss << "  GDP: $" << stats.gdp << "\n";
        ss << "  GDP Growth: " << (stats.gdpGrowth * 100) << "%\n";
        ss << "  Inflation: " << (stats.inflation * 100) << "%\n";
        ss << "  CPI: " << stats.cpi << "\n";
        ss << "  Unemployment: " << (stats.unemployment * 100) << "%\n";
        ss << "  Employed: " << stats.employed << "/" << stats.population << "\n";
        ss << "  Money Supply: $" << stats.moneySupply << "\n";
        ss << "  Interest Rate: " << (stats.interestRate * 100) << "%\n";
        ss << "  Gov Budget: $" << stats.budget << "\n";
        ss << "  Gov Debt: $" << stats.debt << "\n";
        ss << "  Gini: " << stats.giniCoefficient << "\n";
        ss << "  Population: " << stats.population << "\n";
        ss << "  Firms: " << stats.firms << "\n";
        output(ss.str());
    }
    else
    {
        string prop = cmd.propertyAccess;
        stringstream ss;
        if (prop == "gdp")
            ss << stats.gdp;
        else if (prop == "growth" || prop == "gdp_growth")
            ss << stats.gdpGrowth;
        else if (prop == "inflation")
            ss << stats.inflation;
        else if (prop == "cpi")
            ss << stats.cpi;
        else if (prop == "unemployment")
            ss << stats.unemployment;
        else if (prop == "employed")
            ss << stats.employed;
        else if (prop == "money_supply" || prop == "money")
            ss << stats.moneySupply;
        else if (prop == "interest" || prop == "interest_rate")
            ss << stats.interestRate;
        else if (prop == "budget")
            ss << stats.budget;
        else if (prop == "debt")
            ss << stats.debt;
        else if (prop == "gini")
            ss << stats.giniCoefficient;
        else if (prop == "population")
            ss << stats.population;
        else if (prop == "firms")
            ss << stats.firms;
        else
            ss << "Unknown property: " << prop;
        output(ss.str());
    }
}

template <typename T>
T CommandExecutor::getParam(const Command &cmd, const string &name, T defaultValue) const
{
    auto it = cmd.params.find(name);
    if (it == cmd.params.end())
        return defaultValue;

    if (auto val = get_if<T>(&it->second))
        return *val;

    return defaultValue;
}

void CommandExecutor::output(const string &message)
{
    if (outputCallback)
        outputCallback(message);
}
bool CommandExecutor::executeAssignment(const Command &cmd)
{
    if (cmd.name == "system")
    {
        return assignSystemProperty(cmd);
    }
    else if (cmd.name == "worker")
    {
        string name = getParam<string>(cmd, "name", "");
        Worker *w = simulation.FindWorker(name);
        if (!w)
        {
            output("Error: worker '" + name + "' not found");
            return false;
        }
        return assignWorkerProperty(cmd, w);
    }
    else if (cmd.name == "farmer")
    {
        string name = getParam<string>(cmd, "name", "");
        Farmer *f = simulation.FindFarmer(name);
        if (!f)
        {
            output("Error: farmer '" + name + "' not found");
            return false;
        }
        return assignFarmerProperty(cmd, f);
    }
    else if (cmd.name == "owner")
    {
        string name = getParam<string>(cmd, "name", "");
        Owner *o = simulation.FindOwner(name);
        if (!o)
        {
            output("Error: owner '" + name + "' not found");
            return false;
        }
        return assignOwnerProperty(cmd, o);
    }
    else if (cmd.name == "market")
    {
        string name = getParam<string>(cmd, "name", "");
        Market *m = simulation.FindMarket(name);
        if (!m)
        {
            output("Error: market '" + name + "' not found");
            return false;
        }
        return assignMarketProperty(cmd, m);
    }

    output("Error: cannot assign property to '" + cmd.name + "'");
    return false;
}

bool CommandExecutor::assignWorkerProperty(const Command &cmd, Worker *worker)
{
    const string &prop = cmd.assignmentProperty;
    double doubleVal = 0.0;
    string stringVal;

    // Try to extract double value
    if (auto val = get_if<double>(&cmd.assignmentValue))
    {
        doubleVal = *val;
    }
    else if (auto val = get_if<string>(&cmd.assignmentValue))
    {
        stringVal = *val;
    }

    if (prop == "wage")
    {
        double oldWage = worker->GetCurrentWage();
        worker->UpdateLaborSupply(doubleVal);
        simulation.RecordVariableChange("worker." + worker->GetName() + ".wage", oldWage, doubleVal);
    }
    else if (prop == "income")
    {
        double oldIncome = worker->GetMonthlyIncome();
        worker->SetMonthlyIncome(doubleVal);
        simulation.RecordVariableChange("worker." + worker->GetName() + ".income", oldIncome, doubleVal);
    }
    else if (prop == "skill")
    {
        double oldSkill = worker->GetSkillLevel();
        worker->SetSkillLevel(doubleVal);
        simulation.RecordVariableChange("worker." + worker->GetName() + ".skill", oldSkill, doubleVal);
    }
    else if (prop == "min_wage")
    {
        double oldMin = worker->GetMinAcceptableWage();
        worker->SetMinAcceptableWage(doubleVal);
        simulation.RecordVariableChange("worker." + worker->GetName() + ".min_wage", oldMin, doubleVal);
    }
    else
    {
        output("Error: unknown worker property '" + prop + "'");
        return false;
    }

    simulation.RefreshStats();
    output("Worker " + worker->GetName() + "." + prop + " = " + to_string(doubleVal));
    return true;
}

bool CommandExecutor::assignFarmerProperty(const Command &cmd, Farmer *farmer)
{
    const string &prop = cmd.assignmentProperty;
    double doubleVal = 0.0;

    if (auto val = get_if<double>(&cmd.assignmentValue))
    {
        doubleVal = *val;
    }

    if (prop == "land")
    {
        double oldLand = farmer->GetLandSize();
        // No setter for land, would need to add one
        output("Error: cannot directly modify land (would require redesign)");
        return false;
    }
    else if (prop == "fertilizer")
    {
        double oldFert = farmer->GetFertilizerUnits();
        farmer->AddFertilizer(doubleVal - oldFert);
        simulation.RecordVariableChange("farmer." + farmer->GetName() + ".fertilizer", oldFert, doubleVal);
    }
    else if (prop == "technology")
    {
        double oldTech = farmer->GetTechnologyLevel();
        // No direct setter, would need to add
        output("Error: cannot directly modify technology level");
        return false;
    }
    else
    {
        output("Error: unknown farmer property '" + prop + "'");
        return false;
    }

    simulation.RefreshStats();
    output("Farmer " + farmer->GetName() + "." + prop + " = " + to_string(doubleVal));
    return true;
}

bool CommandExecutor::assignOwnerProperty(const Command &cmd, Owner *owner)
{
    const string &prop = cmd.assignmentProperty;
    double doubleVal = 0.0;

    if (auto val = get_if<double>(&cmd.assignmentValue))
    {
        doubleVal = *val;
    }

    if (prop == "price")
    {
        double oldPrice = owner->GetPrice();
        owner->SetPrice(doubleVal);
        simulation.RecordVariableChange("owner." + owner->GetName() + ".price", oldPrice, doubleVal);
    }
    else
    {
        output("Error: unknown owner property '" + prop + "'");
        return false;
    }

    simulation.RefreshStats();
    output("Owner " + owner->GetName() + "." + prop + " = " + to_string(doubleVal));
    return true;
}

bool CommandExecutor::assignMarketProperty(const Command &cmd, Market *market)
{
    const string &prop = cmd.assignmentProperty;
    double doubleVal = 0.0;

    if (auto val = get_if<double>(&cmd.assignmentValue))
    {
        doubleVal = *val;
    }

    if (prop == "price")
    {
        double oldPrice = market->GetCurrentPrice();
        market->SetPrice(doubleVal);
        simulation.RecordVariableChange("market." + market->GetProductName() + ".price", oldPrice, doubleVal);
    }
    else if (prop == "demand")
    {
        double oldDemand = market->GetQuantityDemanded();
        market->SetDemand(doubleVal);
        simulation.RecordVariableChange("market." + market->GetProductName() + ".demand", oldDemand, doubleVal);
    }
    else if (prop == "supply")
    {
        double oldSupply = market->GetQuantitySupplied();
        market->SetSupply(doubleVal);
        simulation.RecordVariableChange("market." + market->GetProductName() + ".supply", oldSupply, doubleVal);
    }
    else if (prop == "tax")
    {
        double oldTax = market->GetTaxRate();
        market->SetTaxRate(doubleVal);
        simulation.RecordVariableChange("market." + market->GetProductName() + ".tax", oldTax, doubleVal);
    }
    else if (prop == "subsidy")
    {
        double oldSubsidy = market->GetSubsidyRate();
        market->SetSubsidyRate(doubleVal);
        simulation.RecordVariableChange("market." + market->GetProductName() + ".subsidy", oldSubsidy, doubleVal);
    }
    else
    {
        output("Error: unknown market property '" + prop + "'");
        return false;
    }

    simulation.RefreshStats();
    output("Market " + market->GetProductName() + "." + prop + " = " + to_string(doubleVal));
    return true;
}

bool CommandExecutor::assignSystemProperty(const Command &cmd)
{
    const string &prop = cmd.assignmentProperty;
    double doubleVal = 0.0;

    if (auto val = get_if<double>(&cmd.assignmentValue))
    {
        doubleVal = *val;
    }

    Government *gov = simulation.GetGovernment();
    if (!gov)
    {
        output("Error: government not initialized");
        return false;
    }

    if (prop == "income_tax_rate" || prop == "income_tax")
    {
        double oldRate = gov->GetIncomeTaxRate();
        gov->SetIncomeTaxRate(doubleVal);
        simulation.RecordVariableChange("system.income_tax_rate", oldRate, doubleVal);
    }
    else if (prop == "corporate_tax_rate" || prop == "corporate_tax")
    {
        double oldRate = gov->GetCorporateTaxRate();
        gov->SetCorporateTaxRate(doubleVal);
        simulation.RecordVariableChange("system.corporate_tax_rate", oldRate, doubleVal);
    }
    else if (prop == "min_wage" || prop == "minimuwage")
    {
        double oldWage = gov->GetMinimumWage();
        gov->SetMinimumWage(doubleVal);
        simulation.RecordVariableChange("system.minimuwage", oldWage, doubleVal);
    }
    else if (prop == "spending" || prop == "government_spending")
    {
        double oldSpending = gov->GetGovernmentSpending();
        gov->SetGovernmentSpending(doubleVal);
        simulation.RecordVariableChange("system.government_spending", oldSpending, doubleVal);
    }
    else if (prop == "money_supply" || prop == "money")
    {
        double oldSupply = gov->GetMoneySupply();
        gov->SetMoneySupply(doubleVal);
        simulation.RecordVariableChange("system.money_supply", oldSupply, doubleVal);
    }
    else if (prop == "interest_rate" || prop == "interest")
    {
        double oldRate = gov->GetInterestRate();
        gov->SetInterestRate(doubleVal);
        simulation.RecordVariableChange("system.interest_rate", oldRate, doubleVal);
    }
    else
    {
        output("Error: unknown system property '" + prop + "'");
        return false;
    }

    simulation.RefreshStats();
    output("System." + prop + " = " + to_string(doubleVal));
    return true;
}
// ============================================================================
// NEW ECONOMIC ANALYSIS COMMAND HANDLERS
// ============================================================================

// ========== MARKET MECHANISMS ==========

void CommandExecutor::cmdMarketEquilibrium(const Command &cmd)
{
    string productName = getParam<string>(cmd, "product", "Rice");
    Market *market = simulation.FindMarket(productName);

    if (!market)
    {
        output("Error: Market '" + productName + "' not found");
        return;
    }

    market->FindEquilibrium();
    output(market->GetEquilibriumAnalysis());
    simulation.RefreshStats();
}

void CommandExecutor::cmdMarketElasticity(const Command &cmd)
{
    string productName = getParam<string>(cmd, "product", "IceCream");
    Market *market = simulation.FindMarket(productName);

    if (!market)
    {
        output("Error: Market '" + productName + "' not found");
        return;
    }

    market->CalculatePriceElasticity();
    output(market->GetElasticityAnalysis());
    simulation.RefreshStats();
}

void CommandExecutor::cmdMarketWelfare(const Command &cmd)
{
    string productName = getParam<string>(cmd, "product", "Rice");
    Market *market = simulation.FindMarket(productName);

    if (!market)
    {
        output("Error: Market '" + productName + "' not found");
        return;
    }

    market->CalculateWelfare();
    output(market->GetWelfareAnalysis());
    simulation.RefreshStats();
}

void CommandExecutor::cmdMarketSupplyShock(const Command &cmd)
{
    string shockType = getParam<string>(cmd, "shock_type", "Bumper Harvest");
    if (shockType.empty())
        shockType = getParam<string>(cmd, "type", "Bumper Harvest");

    string productName = getParam<string>(cmd, "product", "Rice");

    // If args are reversed (type first), swap when it looks like a shock type.
    if (!IsShockType(shockType) && IsShockType(productName))
    {
        swap(shockType, productName);
    }

    Market *market = simulation.FindMarket(productName);
    if (!market)
    {
        output("Error: Market '" + productName + "' not found");
        return;
    }

    double oldPrice = market->GetCurrentPrice();
    string normalized = NormalizeShockType(shockType);

    if (normalized == "bumper harvest")
    {
        output("\n*** BUMPER HARVEST EVENT ***\n");
        market->TriggerBumperHarvest();
        output("Supply increased 30%\n");
    }
    else if (normalized == "natural disaster")
    {
        output("\n*** NATURAL DISASTER EVENT ***\n");
        market->TriggerNaturalDisaster();
        output("Supply decreased 50%\n");
    }
    else
    {
        output("Error: Unknown shock type '" + shockType + "'");
        return;
    }

    output(FormatPriceChange(oldPrice, market->GetCurrentPrice()));
    output(market->GetEquilibriumAnalysis());
    simulation.RefreshStats();
}

void CommandExecutor::cmdMarketTax(const Command &cmd)
{
    string productName = getParam<string>(cmd, "product", "Rice");
    double taxRate = getParam<double>(cmd, "tax_rate", 0.15);
    if (taxRate == 0.15)
        taxRate = getParam<double>(cmd, "rate", 0.15);

    Market *market = simulation.FindMarket(productName);
    if (!market)
    {
        output("Error: Market '" + productName + "' not found");
        return;
    }

    double oldPrice = market->GetCurrentPrice();
    market->SetTaxRate(taxRate);
    market->ApplyTaxAndSubsidy();
    output("Tax of " + to_string(taxRate * 100) + "% applied to " + productName + "\n");
    output(FormatPriceChange(oldPrice, market->GetCurrentPrice()));
    output(market->GetEquilibriumAnalysis());
    simulation.RefreshStats();
}

void CommandExecutor::cmdMarketSubsidy(const Command &cmd)
{
    string productName = getParam<string>(cmd, "product", "IceCream");
    double subsidyRate = getParam<double>(cmd, "subsidy_rate", 0.20);
    if (subsidyRate == 0.20)
        subsidyRate = getParam<double>(cmd, "rate", 0.20);

    Market *market = simulation.FindMarket(productName);
    if (!market)
    {
        output("Error: Market '" + productName + "' not found");
        return;
    }

    double oldPrice = market->GetCurrentPrice();
    market->SetSubsidyRate(subsidyRate);
    market->ApplyTaxAndSubsidy();
    output("Subsidy of " + to_string(subsidyRate * 100) + "% applied to " + productName + "\n");
    output(FormatPriceChange(oldPrice, market->GetCurrentPrice()));
    output(market->GetEquilibriumAnalysis());
    simulation.RefreshStats();
}

void CommandExecutor::cmdMarketPriceControl(const Command &cmd)
{
    string controlType = getParam<string>(cmd, "control_type", "ceiling");
    if (controlType == "ceiling")
        controlType = getParam<string>(cmd, "type", "ceiling");
    string productName = getParam<string>(cmd, "product", "Rice");
    double price = getParam<double>(cmd, "control_price", 25.0);
    if (price == 25.0)
        price = getParam<double>(cmd, "price", 25.0);

    Market *market = simulation.FindMarket(productName);
    if (!market)
    {
        output("Error: Market '" + productName + "' not found");
        return;
    }

    double oldPrice = market->GetCurrentPrice();
    if (controlType == "ceiling")
    {
        market->SetPriceCeiling(price);
        market->EnforcePriceCeiling();
        output("Price ceiling of $" + to_string(price) + " set for " + productName + "\n");
    }
    else if (controlType == "floor")
    {
        market->SetPriceFloor(price);
        market->EnforcePriceFloor();
        output("Price floor of $" + to_string(price) + " set for " + productName + "\n");
    }
    else
    {
        output("Error: Unknown control type. Use 'ceiling' or 'floor'");
        return;
    }

    output(FormatPriceChange(oldPrice, market->GetCurrentPrice()));
    output(market->GetWelfareAnalysis());
    simulation.RefreshStats();
}

// ========== CONSUMER BEHAVIOR ==========

void CommandExecutor::cmdConsumerOptimize(const Command &cmd)
{
    double budget = getParam<double>(cmd, "budget", 100.0);

    output("\n╔════════════════════════════════════════════════╗\n");
    output("║   OPTIMAL CONSUMPTION BUNDLE (Equi-Marginal)   ║\n");
    output("║             Budget: $" + to_string(budget) + "              ║\n");
    output("╚════════════════════════════════════════════════╝\n\n");

    // Simplified example with Rice and Cloth
    double riceMU = 50, ricePrice = 5;
    double clothMU = 20, clothPrice = 4;

    output("RICE:\n");
    output("  Marginal Utility: " + to_string(riceMU) + "\n");
    output("  Price: $" + to_string(ricePrice) + "\n");
    output("  MU/Price: " + to_string(riceMU / ricePrice) + "\n\n");

    output("CLOTH:\n");
    output("  Marginal Utility: " + to_string(clothMU) + "\n");
    output("  Price: $" + to_string(clothPrice) + "\n");
    output("  MU/Price: " + to_string(clothMU / clothPrice) + "\n\n");

    output("ANALYSIS:\n");
    if (riceMU / ricePrice > clothMU / clothPrice)
    {
        output("  Rice has HIGHER MU/Price ratio!\n");
        output("  Decision: Buy more Rice until ratios equalize\n\n");
    }

    double riceQty = (budget * 0.6) / ricePrice;
    double clothQty = (budget * 0.4) / clothPrice;

    output("OPTIMAL CHOICE:\n");
    output("  Buy " + to_string(riceQty) + " Rice ($" + to_string(riceQty * ricePrice) + ")\n");
    output("  Buy " + to_string(clothQty) + " Cloth ($" + to_string(clothQty * clothPrice) + ")\n");
    output("  Total spending: $" + to_string(budget) + "\n\n");

    output("ECONOMIC PRINCIPLE: Equi-Marginal Principle\n");
    output("  Consumer maximizes utility when last dollar spent\n");
    output("  on each good gives equal satisfaction (MU/P ratios equal)\n");
}

void CommandExecutor::cmdConsumerSubstitute(const Command &cmd)
{
    string good1 = getParam<string>(cmd, "good1", "Curd");
    string good2 = getParam<string>(cmd, "good2", "IceCream");
    double priceChange = getParam<double>(cmd, "price_change", 0.25);

    output("\n╔════════════════════════════════════════╗\n");
    output("║    SUBSTITUTE GOODS ANALYSIS           ║\n");
    output("╚════════════════════════════════════════╝\n\n");

    output("EVENT: Price of " + good1 + " increased by " + to_string(priceChange * 100) + "%\n\n");

    output("CROSS-PRICE ELASTICITY: +0.60 (Substitutes)\n");
    output("  * Positive means they're substitutes\n");
    output("  * When one gets expensive, demand shifts to other\n\n");

    double oldQty1 = 50, oldQty2 = 40;
    double newQty1 = oldQty1 * (1 - priceChange * 0.6);
    double newQty2 = oldQty2 * (1 + priceChange * 0.75);

    output("DEMAND CHANGES:\n");
    output("  " + good1 + ": " + to_string(oldQty1) + " → " + to_string(newQty1) + " units (↓ -30%)\n");
    output("    * Law of Demand: Higher price → Lower quantity\n");
    output("  " + good2 + ": " + to_string(oldQty2) + " → " + to_string(newQty2) + " units (↑ +45%)\n");
    output("    * Substitute effect: Demand switches to alternative\n");
}

// ========== FIRM ANALYSIS ==========

void CommandExecutor::cmdFirmCostAnalysis(const Command &cmd)
{
    string firmName = getParam<string>(cmd, "firm", "RiceFarm");
    double quantity = getParam<double>(cmd, "quantity", 100.0);
    double price = getParam<double>(cmd, "price", 15.0);

    double TFC = 500.0;
    double TVC = quantity * 8.0;

    EconomicEquations::CostAnalysis ca = EconomicEquations::CostAnalysis::Calculate(TFC, TVC, quantity);
    output(ca.ToString(price));
    simulation.RefreshStats();
}

void CommandExecutor::cmdFirmAddWorker(const Command &cmd)
{
    output("\n╔════════════════════════════════════════════════╗\n");
    output("║      DIMINISHING RETURNS IN ACTION             ║\n");
    output("╚════════════════════════════════════════════════╝\n\n");

    vector<int> mpList = {10, 12, 8, 3};
    int totalOutput = 0;

    for (size_t i = 0; i < mpList.size(); i++)
    {
        int oldOutput = totalOutput;
        int mp = mpList[i];
        totalOutput += mp;

        output("Worker " + to_string(i + 1) + " added:\n");
        output("  Output: " + to_string(oldOutput) + " → " + to_string(totalOutput) + " units\n");
        output("  Marginal Product: " + to_string(mp) + " units\n");

        if (i == 0)
            output("  Status: INCREASING RETURNS\n");
        else if (i == 1)
            output("  Status: INCREASING RETURNS (Peak)\n");
        else if (i == 2)
            output("  Status: DIMINISHING RETURNS STARTED\n");
        else
            output("  Status: SEVERE DIMINISHING RETURNS\n");
        output("\n");
    }

    output("CONCLUSION:\n");
    output("  Optimal workers to hire: 2\n");
    output("  At 3+ workers, would be better to hire capital equipment\n");
}

void CommandExecutor::cmdFirmShutdown(const Command &cmd)
{
    double currentPrice = 4.0;
    double AVC = 5.0;
    double AFC = 2.0;
    double ATC = AVC + AFC;
    double quantity = 100.0;

    output("\n╔════════════════════════════════════════════════╗\n");
    output("║           SHUTDOWN ANALYSIS                    ║\n");
    output("╚════════════════════════════════════════════════╝\n\n");

    output("Current Market Price: $" + to_string(currentPrice) + "\n");
    output("Average Variable Cost: $" + to_string(AVC) + "\n");
    output("Average Total Cost: $" + to_string(ATC) + "\n\n");

    if (currentPrice < AVC)
    {
        output("RESULT: SHUT DOWN IMMEDIATELY ✗\n\n");
        output("Reasoning:\n");
        output("  * Price ($" + to_string(currentPrice) + ") < AVC ($" + to_string(AVC) + ")\n");
        output("  * You lose $" + to_string(AVC - currentPrice) + " per unit produced\n");
        output("  * Better to close and pay only fixed costs\n\n");

        double operatingLoss = (AVC - currentPrice) * quantity;
        double fixedCost = AFC * quantity;
        output("If you operated:\n");
        output("  Loss per period: $" + to_string(operatingLoss) + "\n");
        output("By shutting down:\n");
        output("  Fixed costs still paid: $" + to_string(fixedCost) + "\n");
        output("  Better outcome by $" + to_string(operatingLoss - fixedCost) + "\n");
    }
    else
    {
        output("RESULT: CONTINUE OPERATING ✓\n\n");
        output("Price ($" + to_string(currentPrice) + ") > AVC ($" + to_string(AVC) + ")\n");
        output("Firm is covering variable costs and contributing to fixed costs.\n");
    }
}

// ========== MACROECONOMICS ==========

void CommandExecutor::cmdGDPCalculate(const Command &cmd)
{
    double C = 500.0;
    double I = 200.0;
    double G = 300.0;
    double X = 100.0;
    double M = 150.0;

    double GDP = EconomicEquations::CalculateGDP(C, I, G, X, M);

    output("\n╔════════════════════════════════════════════════╗\n");
    output("║         GDP CALCULATION                        ║\n");
    output("║      (Expenditure Approach)                    ║\n");
    output("╚════════════════════════════════════════════════╝\n\n");

    output("Consumption (C)           : $" + to_string(C) + "\n");
    output("  * Household spending on goods/services\n\n");
    output("Investment (I)            : $" + to_string(I) + "\n");
    output("  * Business investment, home construction\n\n");
    output("Government Spending (G)   : $" + to_string(G) + "\n");
    output("  * Government purchases, services\n\n");
    output("Exports (X)               : $" + to_string(X) + "\n");
    output("  * Goods sold to foreign countries\n\n");
    output("Imports (M)               : $" + to_string(M) + "\n");
    output("  * Goods bought from foreign countries\n\n");
    output("Net Exports (X - M)       : $" + to_string(X - M) + "\n\n");

    output("================================\n");
    output("GDP = C + I + G + (X-M)\n");
    output("GDP = $" + to_string(C) + " + $" + to_string(I) + " + $" + to_string(G) + " + ($" + to_string(X - M) + ")\n");
    output("GDP = $" + to_string(GDP) + "\n");
    output("================================\n");
}

void CommandExecutor::cmdCPIAnalysis(const Command &cmd)
{
    output("\n╔════════════════════════════════════════════════╗\n");
    output("║      CONSUMER PRICE INDEX (CPI)                ║\n");
    output("╚════════════════════════════════════════════════╝\n\n");

    output("Basket of Goods (Base Year):\n");
    output("  Rice (5 lbs)    : $10.00\n");
    output("  Bread (2 loaves): $4.00\n");
    output("  Milk (1 gallon) : $3.50\n");
    output("  Eggs (1 dozen)  : $2.50\n");
    output("  Total: $20.00\n\n");

    output("Current Year:\n");
    output("  Rice (5 lbs)    : $11.00 (+10%)\n");
    output("  Bread (2 loaves): $4.20 (+5%)\n");
    output("  Milk (1 gallon) : $3.85 (+10%)\n");
    output("  Eggs (1 dozen)  : $2.40 (-4%)\n");
    output("  Total: $21.45\n\n");

    output("CPI = (Current / Base) × 100\n");
    output("CPI = ($21.45 / $20.00) × 100 = 107.25\n\n");

    output("Purchasing Power Impact:\n");
    output("  $1,000 last year → Buys only $931.84 today\n");
    output("  Savers lost 6.76% of purchasing power\n");
}

void CommandExecutor::cmdInflationAnalysis(const Command &cmd)
{
    double lastCPI = 100.0;
    double currentCPI = 107.25;
    double inflation = EconomicEquations::CalculateInflation(currentCPI, lastCPI);

    output("\n╔════════════════════════════════════════════════╗\n");
    output("║         INFLATION ANALYSIS                     ║\n");
    output("╚════════════════════════════════════════════════╝\n\n");

    output("Last Period CPI: " + to_string(lastCPI) + "\n");
    output("Current Period CPI: " + to_string(currentCPI) + "\n\n");

    output("Inflation Rate = (Current CPI - Last CPI) / Last CPI × 100%\n");
    output("Inflation Rate = (" + to_string(currentCPI) + " - " + to_string(lastCPI) + ") / " + to_string(lastCPI) + " × 100%\n");
    output("Inflation Rate = " + to_string(inflation) + "%\n\n");

    output("Economic Effects:\n");
    if (inflation > 0)
    {
        output("  ✗ Prices RISING (Inflation detected)\n");
        output("  ✗ Purchasing power falling\n");
        output("  ✓ Borrowers benefit (repay with cheaper dollars)\n");
        output("  ✗ Savers lose (savings worth less)\n");
    }
    else
    {
        output("  ✓ Prices FALLING (Deflation detected)\n");
        output("  ✓ Purchasing power rising\n");
    }
}

void CommandExecutor::cmdMonetaryPolicy(const Command &cmd)
{
    string policyType = getParam<string>(cmd, "type", "Expansionary");

    output("\n╔════════════════════════════════════════════════╗\n");
    output("║     " + policyType + " MONETARY POLICY            ║\n");
    output("╚════════════════════════════════════════════════╝\n\n");

    if (policyType == "Expansionary")
    {
        output("Money Supply Increase: +20%\n");
        output("Interest Rate Change: 5% → 2%\n\n");
        output("CHAIN REACTION:\n");
        output("  1. Interest Rates Down → Borrowing cheaper\n");
        output("  2. Investment Up → Businesses expand\n");
        output("  3. Consumption Up → Households spend more\n");
        output("  4. Aggregate Demand Up → Total spending increases\n");
        output("  5. OUTPUT EXPANSION → GDP grows, employment rises\n");
        output("  6. INFLATION PRESSURE → Prices start rising over time\n\n");
        output("SHORT TERM: More jobs, higher output\n");
        output("LONG TERM: Rising inflation\n");
    }
    else if (policyType == "Contractionary")
    {
        output("Money Supply Decrease: -15%\n");
        output("Interest Rate Change: 5% → 8%\n\n");
        output("CHAIN REACTION:\n");
        output("  1. Interest Rates Up → Borrowing expensive\n");
        output("  2. Investment Down → Businesses pull back\n");
        output("  3. Consumption Down → Households spend less\n");
        output("  4. Aggregate Demand Down → Total spending falls\n");
        output("  5. OUTPUT CONTRACTION → GDP falls, unemployment rises\n");
        output("  6. INFLATION DROPS → Prices stabilize\n\n");
        output("SHORT TERM: Job losses, recession risk\n");
        output("LONG TERM: Lower inflation\n");
    }
}

void CommandExecutor::cmdFiscalPolicy(const Command &cmd)
{
    string policyName = getParam<string>(cmd, "policy", "spending");
    double value = getParam<double>(cmd, "value", 500.0);

    output("\n╔════════════════════════════════════════════════╗\n");
    output("║         FISCAL POLICY EFFECT                   ║\n");
    output("╚════════════════════════════════════════════════╝\n\n");

    if (policyName == "GovernmentSpending" || policyName == "spending")
    {
        output("Government Spending Increase: +67%\n");
        output("New Government Spending: $" + to_string(value) + "\n\n");
        output("MULTIPLIER EFFECT:\n");
        output("  Initial spending increase: $200\n");
        output("  Multiplier = 2.5x\n");
        output("  Total economic impact: $500\n\n");
        output("AGGREGATE DEMAND EFFECT:\n");
        output("  GDP increases, employment rises\n");
        output("  Deficit increases (spending > tax revenue)\n");
    }
    else if (policyName == "IncomeTaxRate" || policyName == "tax")
    {
        output("Income Tax Increase: " + to_string(value * 100) + "%\n\n");
        output("EFFECT:\n");
        output("  Workers have less disposable income\n");
        output("  Consumption falls → Aggregate Demand falls\n");
        output("  GDP falls → Unemployment rises\n");
    }
}

// ========== PPF AND GROWTH ==========

void CommandExecutor::cmdPPFAnalysis(const Command &cmd)
{
    string good1 = getParam<string>(cmd, "good1", "Rice");
    string good2 = getParam<string>(cmd, "good2", "Cloth");

    output("\n╔════════════════════════════════════════════════╗\n");
    output("║  PRODUCTION POSSIBILITY FRONTIER               ║\n");
    output("╚════════════════════════════════════════════════╝\n\n");

    output("Resources Available:\n");
    output("  * 100 workers\n");
    output("  * 1,000 acres of land\n");
    output("  * Technology Level: 1.0\n\n");

    output("PRODUCTION POSSIBILITIES:\n\n");
    output("100% " + good1 + ", 0% " + good2 + ":\n  " + good1 + ": 500 units,  " + good2 + ": 0 units\n\n");
    output("75% " + good1 + ", 25% " + good2 + ":\n  " + good1 + ": 375 units,  " + good2 + ": 100 units\n\n");
    output("50% " + good1 + ", 50% " + good2 + ":\n  " + good1 + ": 250 units,  " + good2 + ": 200 units\n\n");
    output("25% " + good1 + ", 75% " + good2 + ":\n  " + good1 + ": 125 units,  " + good2 + ": 300 units\n\n");
    output("0% " + good1 + ", 100% " + good2 + ":\n  " + good1 + ": 0 units,  " + good2 + ": 400 units\n\n");

    output("OPPORTUNITY COST:\n");
    output("  To produce 1 more unit of " + good2 + "\n");
    output("  Must give up: 1.25 units of " + good1 + "\n\n");

    output("CURRENT PRODUCTION:\n");
    output("  " + good1 + ": 300 units\n");
    output("  " + good2 + ": 150 units\n");
    output("  Status: INSIDE frontier (Inefficient!)\n");
    output("  * Could produce more with same resources\n");
    output("  * 50 workers are unemployed\n\n");

    output("RECOMMENDATION:\n");
    output("  Move to frontier to be productively efficient\n");
}

void CommandExecutor::cmdTechUpgrade(const Command &cmd)
{
    string sector = getParam<string>(cmd, "sector", "Agriculture");

    output("\n╔════════════════════════════════════════════════╗\n");
    output("║  TECHNOLOGICAL UPGRADE: " + sector + "          ║\n");
    output("╚════════════════════════════════════════════════╝\n\n");

    output("Technology Level: 1.0 → 1.5 (+50%)\n\n");

    output("PPF SHIFT:\n");
    output("  Old Maximum Production:\n");
    output("    Max Rice: 500 units\n");
    output("    Max Cloth: 400 units\n\n");
    output("  New Maximum Production:\n");
    output("    Max Rice: 750 units (+50%)\n");
    output("    Max Cloth: 400 units (unchanged)\n\n");

    output("EFFECT:\n");
    output("  * Farmers now produce 50% more " + sector + " with same effort\n");
    output("  * Economy richer - can have more goods\n");
    output("  * Living standards up across board\n\n");

    output("OUTPUT GROWTH:\n");
    output("  Old: Rice 300, Cloth 150 (Total Value: $4,050)\n");
    output("  New: Rice 450, Cloth 150 (Total Value: $5,850)\n");
    output("  Growth: +44%\n");
}

// ========== SYSTEM ANALYSIS ==========

void CommandExecutor::cmdShowVariableChanges(const Command &cmd)
{
    output("\n╔════════════════════════════════════════════════╗\n");
    output("║        RECENT VARIABLE CHANGES                 ║\n");
    output("║   (Variable Tracker - Audit Log)               ║\n");
    output("╚════════════════════════════════════════════════╝\n\n");

    output("Tracked Changes:\n");
    output("  1. Price of Rice: $36.00 → $40.20 (↑ +12%)\n");
    output("     Affects: Qty_Demanded, Consumer_Surplus, Revenue\n\n");
    output("  2. Quantity Demanded: 82 → 72 units (↓ -12%)\n");
    output("     Caused by: Price increase\n");
    output("     Affects: Total_Sales, GDP\n\n");
    output("  3. Consumer Surplus: $1,681 → $1,521 (↓ -9%)\n");
    output("     Caused by: Price increase\n");
    output("     Affects: Total_Welfare\n\n");

    output("(Track system changes with VariableTracker)\n");
}

void CommandExecutor::cmdDependencyChain(const Command &cmd)
{
    string variable = getParam<string>(cmd, "variable", "Price of Rice");

    output("\n╔════════════════════════════════════════════════╗\n");
    output("║     DEPENDENCY CHAIN: " + variable + "  ║\n");
    output("║     (How changes cascade through economy)      ║\n");
    output("╚════════════════════════════════════════════════╝\n\n");

    output(variable + " Changed\n");
    output("  ↓\n");
    output("Quantity Demanded falls (Law of Demand)\n");
    output("  ↓\n");
    output("Total Revenue might change (depends on elasticity)\n");
    output("  ↓ (if demand is inelastic)\n");
    output("Revenue up - helps farmers\n");
    output("  ↓\n");
    output("Farm profits increase\n");
    output("  ↓\n");
    output("Farmers hire more workers\n");
    output("  ↓\n");
    output("Employment rises, unemployment falls\n");
    output("  ↓\n");
    output("New workers earn income\n");
    output("  ↓\n");
    output("They spend on other goods\n");
    output("  ↓\n");
    output("Those industries expand\n");
    output("  ↓\n");
    output("Overall economic activity increases (GDP grows)\n");
}

// ============================================================================
// PROPAGATION SYSTEM COMMANDS
// ============================================================================

void CommandExecutor::cmdStep(const Command &cmd)
{
    // Run one tick of the simulation
    // This orchestrates: agents act → markets clear → propagation → government → statistics

    output("\n╔══════════════════════════════════════════════════╗\n");
    output("║         Running Simulation Step                  ║\n");
    output("╚══════════════════════════════════════════════════╝\n\n");

    int ticksBefore = simulation.GetCurrentTick();

    output("Phase 1: Agent actions (harvests, production)...\n");
    output("Phase 2: Market clearing (equilibrium prices)...\n");
    output("Phase 3: Propagation (cascading effects)...\n");
    output("Phase 4: Government accounting...\n");
    output("Phase 5: Statistics refresh...\n\n");

    // Execute the step
    simulation.Step();

    int ticksAfter = simulation.GetCurrentTick();

    output(TermColors::Styled("✓ Step complete. ", TermColors::Theme::Success));
    output("Advanced from tick " + to_string(ticksBefore) +
           " to tick " + to_string(ticksAfter) + "\n\n");

    // Show brief summary
    const auto &stats = simulation.GetStats();
    ostringstream summary;
    summary << fixed << setprecision(1);
    summary << "Economic Snapshot:\n";
    summary << "  GDP: $" << stats.gdp << " (" << (stats.gdpGrowth * 100) << "% growth)\n";
    summary << "  Inflation: " << (stats.inflation * 100) << "%\n";
    summary << "  Unemployment: " << (stats.unemployment * 100) << "%\n";
    summary << "  Employed: " << stats.employed << " / " << stats.population << "\n";
    output(summary.str());
}

void CommandExecutor::cmdPropagationShow(const Command &cmd)
{
    // Show the event history for this tick or a specific variable

    EconomicPropagation *propagator = simulation.GetPropagator();
    if (!propagator)
    {
        output("Error: Propagation engine not available\n");
        return;
    }

    string variable = getParam<string>(cmd, "variable", "");

    output("\n╔══════════════════════════════════════════════════╗\n");
    output("║         Propagation Event History                ║\n");
    output("║         (Economic cause-and-effect chain)        ║\n");
    output("╚══════════════════════════════════════════════════╝\n\n");

    string eventLog;
    if (variable.empty())
    {
        // Show all events from current tick
        eventLog = propagator->GetEventLog(-1);
    }
    else
    {
        // Show events for specific variable
        auto events = propagator->GetHistoryForVariable(variable);
        eventLog = "Events for: " + variable + "\n";
        if (events.empty())
        {
            eventLog += "(No events recorded)\n";
        }
        else
        {
            for (const auto &event : events)
            {
                eventLog += "  " + event.ToString() + "\n";
            }
        }
    }

    output(eventLog);
    output("\nUse 'propagation.chain <variable>' to see dependency chains.\n");
}

void CommandExecutor::cmdPropagationChain(const Command &cmd)
{
    // Show the dependency chain for a variable

    string variable = getParam<string>(cmd, "variable", "market.Rice.price");

    if (variable.empty())
    {
        output("Usage: propagation.chain market.Rice.price\n");
        output("Usage: propagation.chain farmer.Shafin.profit\n");
        output("Usage: propagation.chain government.unemployment\n");
        return;
    }

    EconomicPropagation *propagator = simulation.GetPropagator();
    if (!propagator)
    {
        output("Error: Propagation engine not available\n");
        return;
    }

    output("\n╔══════════════════════════════════════════════════╗\n");
    output("║      Dependency Chain: " + variable + "\n");
    output("║      (What causes and affects this variable)    ║\n");
    output("╚══════════════════════════════════════════════════╝\n\n");

    output("CAUSES (what affects " + variable + "):\n");
    propagator->PrintDependencyChain(variable);

    output("\nAFFECTS (what " + variable + " causes):\n");
    auto affected = propagator->GetAffectedVariables(variable);
    if (affected.empty())
    {
        output("  (No direct effects)\n");
    }
    else
    {
        for (const auto &var : affected)
        {
            output("  ↓ " + var + "\n");
        }
    }
}

void CommandExecutor::cmdPropagationGraph(const Command &cmd)
{
    // Export the dependency graph as a Graphviz DOT file

    EconomicPropagation *propagator = simulation.GetPropagator();
    if (!propagator)
    {
        output("Error: Propagation engine not available\n");
        return;
    }

    string filename = getParam<string>(cmd, "filename", "economy_dependency_graph.dot");

    output("\n╔══════════════════════════════════════════════════╗\n");
    output("║       Exporting Dependency Graph                 ║\n");
    output("╚══════════════════════════════════════════════════╝\n\n");

    propagator->ExportDependencyGraph(filename);

    output("Dependency graph exported to: " + filename + "\n");
    output("\nTo visualize this graph:\n");
    output("  1. Install Graphviz (https://graphviz.org)\n");
    output("  2. Run: dot -Tpng " + filename + " -o " + filename + ".png\n");
    output("  3. Open the PNG file to see the economic relationship network\n");
}
