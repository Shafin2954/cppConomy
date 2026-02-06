#include "CommandExecutor.h"
#include "../utils/Logger.h"
#include <sstream>

CommandExecutor::CommandExecutor(Simulation &simulation)
    : m_simulation(simulation)
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

void CommandExecutor::cmdAddWorker(const Command &cmd)
{
    std::string name = getParam<std::string>(cmd, "name", "Worker");
    double income = getParam<double>(cmd, "income", 1000.0);
    double skill = getParam<double>(cmd, "skill", 0.5);

    m_simulation.AddWorker(name, income, skill);
    m_simulation.RefreshStats();
    output("Added worker: " + name);
}

void CommandExecutor::cmdAddFarmer(const Command &cmd)
{
    std::string name = getParam<std::string>(cmd, "name", "Farmer");
    double land = getParam<double>(cmd, "land", 10.0);
    std::string crop = getParam<std::string>(cmd, "crop", "rice");

    m_simulation.AddFarmer(name, land, crop);
    m_simulation.RefreshStats();
    output("Added farmer: " + name);
}

void CommandExecutor::cmdAddOwner(const Command &cmd)
{
    std::string name = getParam<std::string>(cmd, "name", "Owner");
    double capital = getParam<double>(cmd, "capital", 5000.0);
    std::string product = getParam<std::string>(cmd, "product", "cloth");
    bool monopoly = getParam<bool>(cmd, "monopoly", false);

    m_simulation.AddOwner(name, capital, product, monopoly);
    m_simulation.RefreshStats();
    output("Added owner: " + name);
}

void CommandExecutor::cmdAddMarket(const Command &cmd)
{
    std::string product = getParam<std::string>(cmd, "product", "rice");
    m_simulation.CreateMarket(product);
    m_simulation.RefreshStats();
    output("Added market: " + product);
}

void CommandExecutor::cmdSelect(const Command &cmd)
{
    std::string worker = getParam<std::string>(cmd, "worker", "");
    std::string farmer = getParam<std::string>(cmd, "farmer", "");
    std::string owner = getParam<std::string>(cmd, "owner", "");
    std::string market = getParam<std::string>(cmd, "market", "");

    if (!worker.empty())
        m_simulation.SelectWorker(worker);
    if (!farmer.empty())
        m_simulation.SelectFarmer(farmer);
    if (!owner.empty())
        m_simulation.SelectOwner(owner);
    if (!market.empty())
        m_simulation.SelectMarket(market);

    output("Selection updated");
}

void CommandExecutor::cmdClearSelection(const Command &cmd)
{
    (void)cmd;
    m_simulation.ClearSelection();
    output("Selection cleared");
}

void CommandExecutor::cmdBuy(const Command &cmd)
{
    std::string worker = getParam<std::string>(cmd, "worker", "");
    std::string product = getParam<std::string>(cmd, "product", "");
    double qty = getParam<double>(cmd, "quantity", 1.0);
    double price = getParam<double>(cmd, "price", 1.0);

    Worker *w = m_simulation.FindWorker(worker);
    if (!w)
    {
        output("Error: worker not found");
        return;
    }

    double oldWallet = w->GetWallet();
    double oldUtility = w->GetTotalUtility();
    w->Consume(product, qty, price);
    m_simulation.RecordVariableChange("worker." + w->GetName() + ".wallet", oldWallet, w->GetWallet());
    m_simulation.RecordVariableChange("worker." + w->GetName() + ".utility", oldUtility, w->GetTotalUtility());
    m_simulation.RefreshStats();
    output("Purchase complete");
}

void CommandExecutor::cmdHarvest(const Command &cmd)
{
    std::string farmer = getParam<std::string>(cmd, "farmer", "");

    if (farmer.empty())
    {
        for (const auto &f : m_simulation.GetFarmers())
        {
            double oldOutput = f->GetOutputQuantity();
            f->Harvest();
            m_simulation.RecordVariableChange("farmer." + f->GetName() + ".output_quantity",
                                              oldOutput, f->GetOutputQuantity());
        }
        m_simulation.RefreshStats();
        output("All farmers harvested");
        return;
    }

    Farmer *f = m_simulation.FindFarmer(farmer);
    if (!f)
    {
        output("Error: farmer not found");
        return;
    }

    double oldOutput = f->GetOutputQuantity();
    f->Harvest();
    m_simulation.RecordVariableChange("farmer." + f->GetName() + ".output_quantity",
                                      oldOutput, f->GetOutputQuantity());
    m_simulation.RefreshStats();
    output("Harvested for " + farmer);
}

void CommandExecutor::cmdReset(const Command &cmd)
{
    (void)cmd;
    m_simulation.Reset();
    m_simulation.Initialize(2, 1, 1);
    m_simulation.RefreshStats();
    output("Simulation reset");
}

void CommandExecutor::cmdStatus(const Command &cmd)
{
    (void)cmd;
    output(m_simulation.GetStatusString());
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
            ss << "  " << info.name << " - " << info.description << "\n";
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
                        ss << "    " << param.first << ": " << param.second << "\n";
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
    (void)cmd;
    Logger::getInstance().clear();
    output("Log cleared");
}

// ========== Listing Commands ==========

void CommandExecutor::cmdPersons(const Command &cmd)
{
    (void)cmd;
    std::stringstream ss;
    ss << "All Persons:\n";
    ss << "  Workers:\n";
    for (const auto &w : m_simulation.GetWorkers())
        ss << "    - " << w->GetName() << "\n";
    ss << "  Farmers:\n";
    for (const auto &f : m_simulation.GetFarmers())
        ss << "    - " << f->GetName() << "\n";
    ss << "  Owners:\n";
    for (const auto &o : m_simulation.GetOwners())
        ss << "    - " << o->GetName() << "\n";
    output(ss.str());
}

void CommandExecutor::cmdWorkers(const Command &cmd)
{
    (void)cmd;
    std::stringstream ss;
    ss << "Workers:\n";
    for (const auto &w : m_simulation.GetWorkers())
    {
        ss << "  - " << w->GetName() << " (Income: $" << w->GetMonthlyIncome()
           << ", Skill: " << w->GetSkillLevel() << ")\n";
    }
    if (m_simulation.GetWorkers().empty())
        ss << "  (none)\n";
    output(ss.str());
}

void CommandExecutor::cmdFarmers(const Command &cmd)
{
    (void)cmd;
    std::stringstream ss;
    ss << "Farmers:\n";
    for (const auto &f : m_simulation.GetFarmers())
    {
        ss << "  - " << f->GetName() << " (Crop: " << f->GetCrop()
           << ", Land: " << f->GetLandSize() << " acres)\n";
    }
    if (m_simulation.GetFarmers().empty())
        ss << "  (none)\n";
    output(ss.str());
}

void CommandExecutor::cmdOwners(const Command &cmd)
{
    (void)cmd;
    std::stringstream ss;
    ss << "Owners:\n";
    for (const auto &o : m_simulation.GetOwners())
    {
        ss << "  - " << o->GetName() << " (Product: " << o->GetProductType()
           << ", Capital: $" << o->GetCapital() << ")\n";
    }
    if (m_simulation.GetOwners().empty())
        ss << "  (none)\n";
    output(ss.str());
}

void CommandExecutor::cmdMarkets(const Command &cmd)
{
    (void)cmd;
    std::stringstream ss;
    ss << "Markets:\n";
    for (const auto &[name, m] : m_simulation.GetMarkets())
    {
        ss << "  - " << name << " (Price: $" << m->GetCurrentPrice() << ")\n";
    }
    if (m_simulation.GetMarkets().empty())
        ss << "  (none)\n";
    output(ss.str());
}

// ========== Detail/Property Commands ==========

void CommandExecutor::cmdPerson(const Command &cmd)
{
    std::string name = getParam<std::string>(cmd, "name", "");
    if (name.empty())
    {
        output("Error: name parameter required");
        return;
    }

    // Try to find as worker, farmer, or owner
    Worker *w = m_simulation.FindWorker(name);
    Farmer *f = m_simulation.FindFarmer(name);
    Owner *o = m_simulation.FindOwner(name);

    Person *p = nullptr;
    std::string type;
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
        std::string prop = cmd.propertyAccess;
        std::stringstream ss;
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
    std::string name = getParam<std::string>(cmd, "name", "");
    if (name.empty())
    {
        output("Error: name parameter required");
        return;
    }

    Worker *w = m_simulation.FindWorker(name);
    if (!w)
    {
        output("Error: worker '" + name + "' not found");
        return;
    }

    m_simulation.SelectWorker(name);

    if (cmd.propertyAccess.empty())
    {
        output(w->GetInfoString());
    }
    else
    {
        std::string prop = cmd.propertyAccess;
        std::stringstream ss;
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
    std::string name = getParam<std::string>(cmd, "name", "");
    if (name.empty())
    {
        output("Error: name parameter required");
        return;
    }

    Farmer *f = m_simulation.FindFarmer(name);
    if (!f)
    {
        output("Error: farmer '" + name + "' not found");
        return;
    }

    m_simulation.SelectFarmer(name);

    if (cmd.propertyAccess.empty())
    {
        output(f->GetInfoString());
    }
    else
    {
        std::string prop = cmd.propertyAccess;
        std::stringstream ss;
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
    std::string name = getParam<std::string>(cmd, "name", "");
    if (name.empty())
    {
        output("Error: name parameter required");
        return;
    }

    Owner *o = m_simulation.FindOwner(name);
    if (!o)
    {
        output("Error: owner '" + name + "' not found");
        return;
    }

    m_simulation.SelectOwner(name);

    if (cmd.propertyAccess.empty())
    {
        output(o->GetInfoString());
    }
    else
    {
        std::string prop = cmd.propertyAccess;
        std::stringstream ss;
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
    std::string name = getParam<std::string>(cmd, "name", "");
    if (name.empty())
    {
        output("Error: name parameter required");
        return;
    }

    Market *m = m_simulation.FindMarket(name);
    if (!m)
    {
        output("Error: market '" + name + "' not found");
        return;
    }

    m_simulation.SelectMarket(name);

    if (cmd.propertyAccess.empty())
    {
        std::stringstream ss;
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
        std::string prop = cmd.propertyAccess;
        std::stringstream ss;
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
    const auto &stats = m_simulation.GetStats();

    if (cmd.propertyAccess.empty())
    {
        std::stringstream ss;
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
        std::string prop = cmd.propertyAccess;
        std::stringstream ss;
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
T CommandExecutor::getParam(const Command &cmd, const std::string &name, T defaultValue) const
{
    auto it = cmd.params.find(name);
    if (it == cmd.params.end())
        return defaultValue;

    if (auto val = std::get_if<T>(&it->second))
        return *val;

    return defaultValue;
}

void CommandExecutor::output(const std::string &message)
{
    if (m_outputCallback)
        m_outputCallback(message);
}
bool CommandExecutor::executeAssignment(const Command &cmd)
{
    if (cmd.name == "system")
    {
        return assignSystemProperty(cmd);
    }
    else if (cmd.name == "worker")
    {
        std::string name = getParam<std::string>(cmd, "name", "");
        Worker *w = m_simulation.FindWorker(name);
        if (!w)
        {
            output("Error: worker '" + name + "' not found");
            return false;
        }
        return assignWorkerProperty(cmd, w);
    }
    else if (cmd.name == "farmer")
    {
        std::string name = getParam<std::string>(cmd, "name", "");
        Farmer *f = m_simulation.FindFarmer(name);
        if (!f)
        {
            output("Error: farmer '" + name + "' not found");
            return false;
        }
        return assignFarmerProperty(cmd, f);
    }
    else if (cmd.name == "owner")
    {
        std::string name = getParam<std::string>(cmd, "name", "");
        Owner *o = m_simulation.FindOwner(name);
        if (!o)
        {
            output("Error: owner '" + name + "' not found");
            return false;
        }
        return assignOwnerProperty(cmd, o);
    }
    else if (cmd.name == "market")
    {
        std::string name = getParam<std::string>(cmd, "name", "");
        Market *m = m_simulation.FindMarket(name);
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
    const std::string &prop = cmd.assignmentProperty;
    double doubleVal = 0.0;
    std::string stringVal;

    // Try to extract double value
    if (auto val = std::get_if<double>(&cmd.assignmentValue))
    {
        doubleVal = *val;
    }
    else if (auto val = std::get_if<std::string>(&cmd.assignmentValue))
    {
        stringVal = *val;
    }

    if (prop == "wage")
    {
        double oldWage = worker->GetCurrentWage();
        worker->UpdateLaborSupply(doubleVal);
        m_simulation.RecordVariableChange("worker." + worker->GetName() + ".wage", oldWage, doubleVal);
    }
    else if (prop == "income")
    {
        double oldIncome = worker->GetMonthlyIncome();
        worker->SetMonthlyIncome(doubleVal);
        m_simulation.RecordVariableChange("worker." + worker->GetName() + ".income", oldIncome, doubleVal);
    }
    else if (prop == "skill")
    {
        double oldSkill = worker->GetSkillLevel();
        worker->SetSkillLevel(doubleVal);
        m_simulation.RecordVariableChange("worker." + worker->GetName() + ".skill", oldSkill, doubleVal);
    }
    else if (prop == "min_wage")
    {
        double oldMin = worker->GetMinAcceptableWage();
        worker->SetMinAcceptableWage(doubleVal);
        m_simulation.RecordVariableChange("worker." + worker->GetName() + ".min_wage", oldMin, doubleVal);
    }
    else
    {
        output("Error: unknown worker property '" + prop + "'");
        return false;
    }

    m_simulation.RefreshStats();
    output("Worker " + worker->GetName() + "." + prop + " = " + std::to_string(doubleVal));
    return true;
}

bool CommandExecutor::assignFarmerProperty(const Command &cmd, Farmer *farmer)
{
    const std::string &prop = cmd.assignmentProperty;
    double doubleVal = 0.0;

    if (auto val = std::get_if<double>(&cmd.assignmentValue))
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
        m_simulation.RecordVariableChange("farmer." + farmer->GetName() + ".fertilizer", oldFert, doubleVal);
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

    m_simulation.RefreshStats();
    output("Farmer " + farmer->GetName() + "." + prop + " = " + std::to_string(doubleVal));
    return true;
}

bool CommandExecutor::assignOwnerProperty(const Command &cmd, Owner *owner)
{
    const std::string &prop = cmd.assignmentProperty;
    double doubleVal = 0.0;

    if (auto val = std::get_if<double>(&cmd.assignmentValue))
    {
        doubleVal = *val;
    }

    if (prop == "price")
    {
        double oldPrice = owner->GetPrice();
        owner->SetPrice(doubleVal);
        m_simulation.RecordVariableChange("owner." + owner->GetName() + ".price", oldPrice, doubleVal);
    }
    else
    {
        output("Error: unknown owner property '" + prop + "'");
        return false;
    }

    m_simulation.RefreshStats();
    output("Owner " + owner->GetName() + "." + prop + " = " + std::to_string(doubleVal));
    return true;
}

bool CommandExecutor::assignMarketProperty(const Command &cmd, Market *market)
{
    const std::string &prop = cmd.assignmentProperty;
    double doubleVal = 0.0;

    if (auto val = std::get_if<double>(&cmd.assignmentValue))
    {
        doubleVal = *val;
    }

    if (prop == "price")
    {
        double oldPrice = market->GetCurrentPrice();
        market->SetPrice(doubleVal);
        m_simulation.RecordVariableChange("market." + market->GetProductName() + ".price", oldPrice, doubleVal);
    }
    else if (prop == "demand")
    {
        double oldDemand = market->GetQuantityDemanded();
        market->SetDemand(doubleVal);
        m_simulation.RecordVariableChange("market." + market->GetProductName() + ".demand", oldDemand, doubleVal);
    }
    else if (prop == "supply")
    {
        double oldSupply = market->GetQuantitySupplied();
        market->SetSupply(doubleVal);
        m_simulation.RecordVariableChange("market." + market->GetProductName() + ".supply", oldSupply, doubleVal);
    }
    else if (prop == "tax")
    {
        double oldTax = market->GetTaxRate();
        market->SetTaxRate(doubleVal);
        m_simulation.RecordVariableChange("market." + market->GetProductName() + ".tax", oldTax, doubleVal);
    }
    else if (prop == "subsidy")
    {
        double oldSubsidy = market->GetSubsidyRate();
        market->SetSubsidyRate(doubleVal);
        m_simulation.RecordVariableChange("market." + market->GetProductName() + ".subsidy", oldSubsidy, doubleVal);
    }
    else
    {
        output("Error: unknown market property '" + prop + "'");
        return false;
    }

    m_simulation.RefreshStats();
    output("Market " + market->GetProductName() + "." + prop + " = " + std::to_string(doubleVal));
    return true;
}

bool CommandExecutor::assignSystemProperty(const Command &cmd)
{
    const std::string &prop = cmd.assignmentProperty;
    double doubleVal = 0.0;

    if (auto val = std::get_if<double>(&cmd.assignmentValue))
    {
        doubleVal = *val;
    }

    Government *gov = m_simulation.GetGovernment();
    if (!gov)
    {
        output("Error: government not initialized");
        return false;
    }

    if (prop == "income_tax_rate" || prop == "income_tax")
    {
        double oldRate = gov->GetIncomeTaxRate();
        gov->SetIncomeTaxRate(doubleVal);
        m_simulation.RecordVariableChange("system.income_tax_rate", oldRate, doubleVal);
    }
    else if (prop == "corporate_tax_rate" || prop == "corporate_tax")
    {
        double oldRate = gov->GetCorporateTaxRate();
        gov->SetCorporateTaxRate(doubleVal);
        m_simulation.RecordVariableChange("system.corporate_tax_rate", oldRate, doubleVal);
    }
    else if (prop == "min_wage" || prop == "minimum_wage")
    {
        double oldWage = gov->GetMinimumWage();
        gov->SetMinimumWage(doubleVal);
        m_simulation.RecordVariableChange("system.minimum_wage", oldWage, doubleVal);
    }
    else if (prop == "spending" || prop == "government_spending")
    {
        double oldSpending = gov->GetGovernmentSpending();
        gov->SetGovernmentSpending(doubleVal);
        m_simulation.RecordVariableChange("system.government_spending", oldSpending, doubleVal);
    }
    else if (prop == "money_supply" || prop == "money")
    {
        double oldSupply = gov->GetMoneySupply();
        gov->SetMoneySupply(doubleVal);
        m_simulation.RecordVariableChange("system.money_supply", oldSupply, doubleVal);
    }
    else if (prop == "interest_rate" || prop == "interest")
    {
        double oldRate = gov->GetInterestRate();
        gov->SetInterestRate(doubleVal);
        m_simulation.RecordVariableChange("system.interest_rate", oldRate, doubleVal);
    }
    else
    {
        output("Error: unknown system property '" + prop + "'");
        return false;
    }

    m_simulation.RefreshStats();
    output("System." + prop + " = " + std::to_string(doubleVal));
    return true;
}