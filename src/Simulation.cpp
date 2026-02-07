#include "Simulation.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include "Logger.h"

Simulation::Simulation()
    : m_government(std::make_unique<Government>()),
      m_selected_worker(nullptr),
      m_selected_farmer(nullptr),
      m_selected_owner(nullptr),
      m_selected_market(nullptr)
{
    m_stats.moneySupply = m_government->GetMoneySupply();
    m_stats.interestRate = m_government->GetInterestRate();

    // Default variable relations (can be expanded later)
    RegisterVariableRelation("government.income_tax_rate", "person.wallet", RelationEffect::Decreases);
    RegisterVariableRelation("government.minimum_wage", "worker.employment", RelationEffect::Decreases);
    RegisterVariableRelation("worker.monthly_income", "worker.wallet", RelationEffect::Increases);
    RegisterVariableRelation("market.price", "market.quantity_demanded", RelationEffect::Decreases);
    RegisterVariableRelation("market.price", "market.revenue", RelationEffect::Changes);
    RegisterVariableRelation("farmer.output_quantity", "market.supply", RelationEffect::Increases);
    RegisterVariableRelation("farmer.output_quantity", "market.price", RelationEffect::Decreases);
}

void Simulation::Initialize(int num_workers, int num_farmers, int num_owners)
{
    Reset();

    // Create some default entities for testing
    AddFarmer("Shafin", 50.0, "corn");
    AddOwner("Soron", 20000.0, "cloth", false);
    AddWorker("Jabir", 500.0, 0.7);

    // Create default markets
    CreateMarket("rice");
    CreateMarket("cloth");

    // Initialize stats
    RefreshStats();
}

void Simulation::Reset()
{
    m_workers.clear();
    m_farmers.clear();
    m_owners.clear();
    m_markets.clear();
    m_selected_worker = nullptr;
    m_selected_farmer = nullptr;
    m_selected_owner = nullptr;
    m_selected_market = nullptr;
    m_stats = EconomicStats();
    m_prevStats = EconomicStats();
}

void Simulation::AddWorker(const std::string &name, double income, double skill)
{
    int id = static_cast<int>(m_workers.size()) + 1;
    m_workers.push_back(std::make_unique<Worker>(id, name, income, skill));
    // Auto-select the newly created worker
    m_selected_worker = m_workers.back().get();
}

void Simulation::AddFarmer(const std::string &name, double land, const std::string &crop)
{
    int id = static_cast<int>(m_farmers.size()) + 1;
    m_farmers.push_back(std::make_unique<Farmer>(id, name, 1000.0, land, crop));
    // Auto-select the newly created farmer
    m_selected_farmer = m_farmers.back().get();
}

void Simulation::AddOwner(const std::string &name, double capital, const std::string &product, bool is_monopoly)
{
    int id = static_cast<int>(m_owners.size()) + 1;
    m_owners.push_back(std::make_unique<Owner>(id, name, capital, product, is_monopoly));
    // Auto-select the newly created owner
    m_selected_owner = m_owners.back().get();
}

Worker *Simulation::FindWorker(const std::string &name)
{
    for (auto &w : m_workers)
    {
        if (w->GetName() == name)
            return w.get();
    }
    return nullptr;
}

Farmer *Simulation::FindFarmer(const std::string &name)
{
    for (auto &f : m_farmers)
    {
        if (f->GetName() == name)
            return f.get();
    }
    return nullptr;
}

Owner *Simulation::FindOwner(const std::string &name)
{
    for (auto &o : m_owners)
    {
        if (o->GetName() == name)
            return o.get();
    }
    return nullptr;
}

void Simulation::CreateMarket(const std::string &product_name)
{
    m_markets[product_name] = std::make_unique<Market>(product_name);
    // Auto-select the newly created market
    m_selected_market = m_markets[product_name].get();
}

Market *Simulation::FindMarket(const std::string &product_name)
{
    auto it = m_markets.find(product_name);
    if (it != m_markets.end())
        return it->second.get();
    return nullptr;
}

void Simulation::SelectWorker(const std::string &name)
{
    m_selected_worker = FindWorker(name);
}

void Simulation::SelectFarmer(const std::string &name)
{
    m_selected_farmer = FindFarmer(name);
}

void Simulation::SelectOwner(const std::string &name)
{
    m_selected_owner = FindOwner(name);
}

void Simulation::SelectMarket(const std::string &product_name)
{
    m_selected_market = FindMarket(product_name);
}

void Simulation::ClearSelection()
{
    m_selected_worker = nullptr;
    m_selected_farmer = nullptr;
    m_selected_owner = nullptr;
    m_selected_market = nullptr;
}

double Simulation::CalculateTotalProduction()
{
    double total = 0.0;
    for (const auto &f : m_farmers)
        total += f->GetOutputQuantity();
    for (const auto &o : m_owners)
        total += o->GetProduction();
    return total;
}

double Simulation::CalculateUnemploymentRate()
{
    if (m_workers.empty())
        return 0.0;
    int unemployed = 0;
    for (const auto &w : m_workers)
    {
        if (!w->IsEmployed())
            unemployed++;
    }
    return static_cast<double>(unemployed) / m_workers.size();
}

std::string Simulation::GetStatusString() const
{
    std::ostringstream ss;
    ss << "Workers: " << m_workers.size() << ", Farmers: " << m_farmers.size()
       << ", Owners: " << m_owners.size() << "\n";
    ss << "Markets: " << m_markets.size() << "\n";
    return ss.str();
}

void Simulation::RefreshStats()
{
    // Update government aggregates
    double total_production = CalculateTotalProduction();
    m_government->CalculateNominalGDP(total_production);

    // Update stats snapshot
    m_prevStats = m_stats;
    m_stats.gdp = m_government->GetNominalGDP();
    m_stats.inflation = m_government->GetInflationRate();
    m_stats.cpi = m_government->GetCPI();
    m_stats.population = static_cast<int>(m_workers.size() + m_farmers.size() + m_owners.size());
    m_stats.firms = static_cast<int>(m_owners.size());
    m_stats.employed = 0;
    for (const auto &w : m_workers)
    {
        if (w->IsEmployed())
            m_stats.employed++;
    }
    m_stats.unemployment = CalculateUnemploymentRate();
    m_stats.moneySupply = m_government->GetMoneySupply();
    m_stats.interestRate = m_government->GetInterestRate();
    m_stats.debt = m_government->GetGovDebt();
    m_stats.budget = -m_government->GetBudgetDeficit();

    if (m_prevStats.gdp > 0.0)
        m_stats.gdpGrowth = (m_stats.gdp - m_prevStats.gdp) / m_prevStats.gdp;
    else
        m_stats.gdpGrowth = 0.0;

    if (m_statsCallback)
        m_statsCallback(m_stats);
}

void Simulation::RegisterVariableRelation(const std::string &source,
                                          const std::string &target,
                                          RelationEffect effect)
{
    m_variableRelations[source].push_back({target, effect});
}

static std::string effectToString(Simulation::RelationEffect effect)
{
    switch (effect)
    {
    case Simulation::RelationEffect::Increases:
        return "increase";
    case Simulation::RelationEffect::Decreases:
        return "decrease";
    default:
        return "change";
    }
}

void Simulation::RecordVariableChange(const std::string &variable,
                                      double oldValue,
                                      double newValue)
{
    std::string changeType = "unchanged";
    if (newValue > oldValue)
        changeType = "increased";
    else if (newValue < oldValue)
        changeType = "decreased";

    std::ostringstream ss;
    ss << "Change: " << variable << " " << changeType << " from "
       << std::fixed << std::setprecision(3) << oldValue << " to " << newValue;
    LOG_INFO(ss.str());

    auto it = m_variableRelations.find(variable);
    if (it == m_variableRelations.end())
        return;

    for (const auto &relation : it->second)
    {
        std::string expected = "change";
        if (changeType == "unchanged")
        {
            expected = "unchanged";
        }
        else if (relation.effect == RelationEffect::Increases)
        {
            expected = (changeType == "increased") ? "increase" : "decrease";
        }
        else if (relation.effect == RelationEffect::Decreases)
        {
            expected = (changeType == "increased") ? "decrease" : "increase";
        }
        else
        {
            expected = effectToString(relation.effect);
        }

        std::ostringstream rel;
        rel << "Relation: " << variable << " " << changeType
            << " -> " << relation.target << " expected to " << expected;
        LOG_INFO(rel.str());
    }
}

void Simulation::RecordVariableChange(const std::string &variable,
                                      const std::string &oldValue,
                                      const std::string &newValue)
{
    std::string changeType = (oldValue == newValue) ? "unchanged" : "changed";

    std::ostringstream ss;
    ss << "Change: " << variable << " " << changeType << " from "
       << oldValue << " to " << newValue;
    LOG_INFO(ss.str());

    auto it = m_variableRelations.find(variable);
    if (it == m_variableRelations.end())
        return;

    for (const auto &relation : it->second)
    {
        std::ostringstream rel;
        rel << "Relation: " << variable << " changed -> " << relation.target
            << " expected to " << effectToString(relation.effect);
        LOG_INFO(rel.str());
    }
}
