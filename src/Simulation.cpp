#include "Simulation.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cmath>
#include "Logger.h"

using namespace std;

Simulation::Simulation()
    : government(make_unique<Government>()),
      propagator(make_unique<EconomicPropagation>(this)),
      selected_worker(nullptr),
      selected_farmer(nullptr),
      selected_owner(nullptr),
      selected_market(nullptr)
{
    stats.moneySupply = government->GetMoneySupply();
    stats.interestRate = government->GetInterestRate();

    // Default variable relations (can be expanded later)
    RegisterVariableRelation("government.income_tax_rate", "person.wallet", RelationEffect::Decreases);
    RegisterVariableRelation("government.minimuwage", "worker.employment", RelationEffect::Decreases);
    RegisterVariableRelation("worker.monthly_income", "worker.wallet", RelationEffect::Increases);
    RegisterVariableRelation("market.price", "market.quantity_demanded", RelationEffect::Decreases);
    RegisterVariableRelation("market.price", "market.revenue", RelationEffect::Changes);
    RegisterVariableRelation("farmer.output_quantity", "market.supply", RelationEffect::Increases);
    RegisterVariableRelation("farmer.output_quantity", "market.price", RelationEffect::Decreases);
}

void Simulation::Initialize(int nuworkers, int nufarmers, int nuowners)
{
    Reset();

    // Create some default entities for testing
    AddFarmer("Shafin", 50.0, "Corn");
    AddOwner("Soron", 20000.0, "Cloth", false);
    AddWorker("Jabir", 500.0, 0.7);

    // Create default markets
    CreateMarket("Rice");
    CreateMarket("Cloth");

    // Initialize stats
    RefreshStats();
}

void Simulation::Reset()
{
    workers.clear();
    farmers.clear();
    owners.clear();
    markets.clear();
    selected_worker = nullptr;
    selected_farmer = nullptr;
    selected_owner = nullptr;
    selected_market = nullptr;
    stats = EconomicStats();
    prevStats = EconomicStats();
    currentTick = 0;
    propagator = make_unique<EconomicPropagation>(this);
}

void Simulation::AddWorker(const string &name, double income, double skill)
{
    int id = static_cast<int>(workers.size()) + 1;
    workers.push_back(make_unique<Worker>(id, name, income, skill));
    // Auto-select the newly created worker
    selected_worker = workers.back().get();
}

void Simulation::AddFarmer(const string &name, double land, const string &crop)
{
    int id = static_cast<int>(farmers.size()) + 1;
    farmers.push_back(make_unique<Farmer>(id, name, 1000.0, land, crop));
    // Auto-select the newly created farmer
    selected_farmer = farmers.back().get();
}

void Simulation::AddOwner(const string &name, double capital, const string &product, bool is_monopoly)
{
    int id = static_cast<int>(owners.size()) + 1;
    owners.push_back(make_unique<Owner>(id, name, capital, product, is_monopoly));
    // Auto-select the newly created owner
    selected_owner = owners.back().get();
}

Worker *Simulation::FindWorker(const string &name)
{
    for (auto &w : workers)
    {
        if (w->GetName() == name)
            return w.get();
    }
    return nullptr;
}

Farmer *Simulation::FindFarmer(const string &name)
{
    for (auto &f : farmers)
    {
        if (f->GetName() == name)
            return f.get();
    }
    return nullptr;
}

Owner *Simulation::FindOwner(const string &name)
{
    for (auto &o : owners)
    {
        if (o->GetName() == name)
            return o.get();
    }
    return nullptr;
}

void Simulation::CreateMarket(const string &product_name)
{
    markets[product_name] = make_unique<Market>(product_name);
    // Auto-select the newly created market
    selected_market = markets[product_name].get();
}

Market *Simulation::FindMarket(const string &product_name)
{
    auto it = markets.find(product_name);
    if (it != markets.end())
        return it->second.get();
    return nullptr;
}

void Simulation::SelectWorker(const string &name)
{
    selected_worker = FindWorker(name);
}

void Simulation::SelectFarmer(const string &name)
{
    selected_farmer = FindFarmer(name);
}

void Simulation::SelectOwner(const string &name)
{
    selected_owner = FindOwner(name);
}

void Simulation::SelectMarket(const string &product_name)
{
    selected_market = FindMarket(product_name);
}

void Simulation::ClearSelection()
{
    selected_worker = nullptr;
    selected_farmer = nullptr;
    selected_owner = nullptr;
    selected_market = nullptr;
}

double Simulation::CalculateTotalProduction()
{
    double total = 0.0;
    for (const auto &f : farmers)
        total += f->GetOutputQuantity();
    for (const auto &o : owners)
        total += o->GetProduction();
    return total;
}

double Simulation::CalculateUnemploymentRate()
{
    if (workers.empty())
        return 0.0;
    int unemployed = 0;
    for (const auto &w : workers)
    {
        if (!w->IsEmployed())
            unemployed++;
    }
    return static_cast<double>(unemployed) / workers.size();
}

string Simulation::GetStatusString() const
{
    ostringstream ss;
    ss << "Workers: " << workers.size() << ", Farmers: " << farmers.size()
       << ", Owners: " << owners.size() << "\n";
    ss << "Markets: " << markets.size() << "\n";
    return ss.str();
}

void Simulation::RefreshStats()
{
    // Update government aggregates
    double total_production = CalculateTotalProduction();
    government->CalculateNominalGDP(total_production);

    // Update stats snapshot
    prevStats = stats;
    stats.gdp = government->GetNominalGDP();
    stats.inflation = government->GetInflationRate();
    stats.cpi = government->GetCPI();
    stats.population = static_cast<int>(workers.size() + farmers.size() + owners.size());
    stats.firms = static_cast<int>(owners.size());
    stats.employed = 0;
    for (const auto &w : workers)
    {
        if (w->IsEmployed())
            stats.employed++;
    }
    stats.unemployment = CalculateUnemploymentRate();
    stats.moneySupply = government->GetMoneySupply();
    stats.interestRate = government->GetInterestRate();
    stats.debt = government->GetGovDebt();
    stats.budget = -government->GetBudgetDeficit();

    if (prevStats.gdp > 0.0)
        stats.gdpGrowth = (stats.gdp - prevStats.gdp) / prevStats.gdp;
    else
        stats.gdpGrowth = 0.0;

    if (statsCallback)
        statsCallback(stats);
}

void Simulation::RegisterVariableRelation(const string &source,
                                          const string &target,
                                          RelationEffect effect)
{
    variableRelations[source].push_back({target, effect});
}

static string effectToString(Simulation::RelationEffect effect)
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

void Simulation::RecordVariableChange(const string &variable,
                                      double oldValue,
                                      double newValue)
{
    string changeType = "unchanged";
    if (newValue > oldValue)
        changeType = "increased";
    else if (newValue < oldValue)
        changeType = "decreased";

    ostringstream ss;
    ss << "Change: " << variable << " " << changeType << " from "
       << fixed << setprecision(3) << oldValue << " to " << newValue;
    LOG_INFO(ss.str());

    auto it = variableRelations.find(variable);
    if (it == variableRelations.end())
        return;

    for (const auto &relation : it->second)
    {
        string expected = "change";
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

        ostringstream rel;
        rel << "Relation: " << variable << " " << changeType
            << " -> " << relation.target << " expected to " << expected;
        LOG_INFO(rel.str());
    }
}

void Simulation::RecordVariableChange(const string &variable,
                                      const string &oldValue,
                                      const string &newValue)
{
    string changeType = (oldValue == newValue) ? "unchanged" : "changed";

    ostringstream ss;
    ss << "Change: " << variable << " " << changeType << " from "
       << oldValue << " to " << newValue;
    LOG_INFO(ss.str());

    auto it = variableRelations.find(variable);
    if (it == variableRelations.end())
        return;

    for (const auto &relation : it->second)
    {
        ostringstream rel;
        rel << "Relation: " << variable << " changed -> " << relation.target
            << " expected to " << effectToString(relation.effect);
        LOG_INFO(rel.str());
    }
}
// ========== Harvest & Market Operations ==========

void Simulation::UpdateMarketFromHarvest(Farmer *farmer)
{
    if (!farmer || farmer->GetOutputQuantity() <= 0.0)
        return;

    // Get or create market for farmer's crop
    Market *market = FindMarket(farmer->GetCrop());
    if (!market)
    {
        CreateMarket(farmer->GetCrop());
        market = FindMarket(farmer->GetCrop());
    }

    if (market)
    {
        // Add harvest output as supply to the market
        double oldSupply = market->GetQuantitySupplied();
        market->SetSupply(market->GetQuantitySupplied() + farmer->GetOutputQuantity());

        RecordVariableChange("farmer." + farmer->GetName() + ".harvest_output",
                             0.0, farmer->GetOutputQuantity());
        RecordVariableChange("market." + farmer->GetCrop() + ".supply",
                             oldSupply, market->GetQuantitySupplied());

        if (propagator)
        {
            propagator->EmitChange("market.update_froharvest",
                                   "market." + farmer->GetCrop() + ".supply",
                                   oldSupply,
                                   market->GetQuantitySupplied(),
                                   "Harvest added to market supply");
        }
    }
}

void Simulation::UpdateConsumerDemand()
{
    // Update quantity demanded for all consumers (workers, farmers, owners) at current market prices
    for (auto &worker : workers)
    {
        for (const auto &[product, market_ptr] : markets)
        {
            if (market_ptr)
            {
                worker->UpdateQuantityDemanded(product, market_ptr->GetCurrentPrice());
            }
        }
    }

    for (auto &farmer : farmers)
    {
        for (const auto &[product, market_ptr] : markets)
        {
            if (market_ptr)
            {
                farmer->UpdateQuantityDemanded(product, market_ptr->GetCurrentPrice());
            }
        }
    }

    for (auto &owner : owners)
    {
        for (const auto &[product, market_ptr] : markets)
        {
            if (market_ptr)
            {
                owner->UpdateQuantityDemanded(product, market_ptr->GetCurrentPrice());
            }
        }
    }
}

void Simulation::RecalculateMarketEquilibria()
{
    // First update all consumer demand at current prices
    UpdateConsumerDemand();

    // Then calculate equilibrium for each market
    for (auto &[product_name, market] : markets)
    {
        if (market)
        {
            // Aggregate consumer demand (sum all demand at current price)
            double totalDemand = 0.0;
            for (const auto &worker : workers)
                totalDemand += worker->GetQuantityDemanded(product_name);
            for (const auto &farmer : farmers)
                totalDemand += farmer->GetQuantityDemanded(product_name);
            for (const auto &owner : owners)
                totalDemand += owner->GetQuantityDemanded(product_name);

            // Update market demand
            market->SetDemand(totalDemand);

            // Find new equilibrium
            market->FindEquilibrium();

            RecordVariableChange("market." + product_name + ".equilibriuprice",
                                 market->GetCurrentPrice(), market->GetCurrentPrice());
        }
    }
}

void Simulation::Step()
{
    // =====================================================================
    // Phase 1: Agent actions (production/harvest)
    // =====================================================================
    for (auto &farmer : farmers)
    {
        double oldOutput = farmer->GetOutputQuantity();
        farmer->Harvest();

        if (propagator)
        {
            propagator->EmitChange("farmer." + farmer->GetName(),
                                   "farmer." + farmer->GetName() + ".output_quantity",
                                   oldOutput,
                                   farmer->GetOutputQuantity(),
                                   "Farmer harvested");
        }

        UpdateMarketFromHarvest(farmer.get());
    }

    for (auto &owner : owners)
    {
        double oldProduction = owner->GetProduction();
        owner->Produce();

        if (propagator)
        {
            propagator->EmitChange("owner." + owner->GetName(),
                                   "owner." + owner->GetName() + ".production",
                                   oldProduction,
                                   owner->GetProduction(),
                                   "Owner production updated");
        }
    }

    // =====================================================================
    // Phase 2: Market clearing (equilibrium recalculation)
    // =====================================================================
    UpdateConsumerDemand();

    for (auto &[product_name, market] : markets)
    {
        if (!market)
            continue;

        double oldPrice = market->GetCurrentPrice();

        double totalDemand = 0.0;
        for (const auto &worker : workers)
            totalDemand += worker->GetQuantityDemanded(product_name);
        for (const auto &farmer : farmers)
            totalDemand += farmer->GetQuantityDemanded(product_name);
        for (const auto &owner : owners)
            totalDemand += owner->GetQuantityDemanded(product_name);

        market->SetDemand(totalDemand);
        market->FindEquilibrium();

        double newPrice = market->GetCurrentPrice();
        if (propagator && abs(newPrice - oldPrice) > 1e-6)
        {
            propagator->EmitChange("market_clearing",
                                   "market." + product_name + ".price",
                                   oldPrice,
                                   newPrice,
                                   "Equilibrium price recalculated");
        }
    }

    // =====================================================================
    // Phase 3: Propagation (immediate → decision → delayed effects)
    // =====================================================================
    if (propagator)
    {
        propagator->ProcessTier1();
        propagator->ProcessTier2();
        propagator->ProcessTier3();
    }

    // =====================================================================
    // Phase 4: Government accounting (taxes, unemployment, budget)
    // =====================================================================
    if (government)
    {
        double totalIncome = 0.0;
        double totalProfits = 0.0;

        int totalLabor = static_cast<int>(workers.size());
        int unemployed = 0;

        for (const auto &worker : workers)
        {
            totalIncome += worker->GetMonthlyIncome();
            if (!worker->IsEmployed())
                unemployed++;
        }

        for (const auto &farmer : farmers)
            totalProfits += max(0.0, farmer->GetProfit());
        for (const auto &owner : owners)
            totalProfits += max(0.0, owner->GetProfit());

        government->CollectTaxes(totalIncome, totalProfits);
        government->UpdateUnemploymentRate(unemployed, totalLabor);
        government->UpdateBudget();
    }

    // =====================================================================
    // Phase 5: Statistics refresh
    // =====================================================================
    RefreshStats();

    // Advance time for delayed effects
    if (propagator)
        propagator->AdvanceTick();
    currentTick++;
}