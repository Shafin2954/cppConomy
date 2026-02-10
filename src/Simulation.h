#pragma once

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <functional>
#include "Consumer.h"
#include "Worker.h"
#include "Farmer.h"
#include "Owner.h"
#include "Market.h"
#include "Government.h"
#include "EconomicPropagation.h"

// ============================================================================
// Simulation.h - Central controller managing the entire economic simulation
//
// Manages: Creation/destruction of all agents, markets, government
//          Coordination of market transactions
//          Variable change tracking and relation mapping
//          Economic propagation engine (variable interconnections)
//
// The propagation engine (EconomicPropagation) handles HOW changes cascade.
// This class orchestrates WHEN processing happens via the Step() method:
//   Phase 1: Agent actions (harvest, produce, trade)
//   Phase 2: Market clearing (find equilibrium prices)
//   Phase 3: Propagation (cascade economic effects through tiers 1→2→3)
//   Phase 4: Government policy (collect taxes, update budget)
//   Phase 5: Statistics (refresh GDP, inflation, unemployment)
// ============================================================================

class Simulation
{
public:
    // ========== Variable Relation Tracking ==========
    enum class RelationEffect
    {
        Increases,
        Decreases,
        Changes
    };

    // ========== Dashboard Stats (for UI) ==========
    struct EconomicStats
    {
        double gdp = 0.0;
        double gdpGrowth = 0.0;
        double inflation = 0.0;
        double cpi = 100.0;
        double unemployment = 0.0;
        int employed = 0;
        int population = 0;
        double moneySupply = 0.0;
        double interestRate = 0.0;
        double budget = 0.0;
        double debt = 0.0;
        double giniCoefficient = 0.0;
        int firms = 0;
    };

    using StatsCallback = std::function<void(const EconomicStats &)>;

private:
    // ========== Agents (Economic Participants) ==========
    std::vector<std::unique_ptr<Worker>> workers;
    std::vector<std::unique_ptr<Farmer>> farmers;
    std::vector<std::unique_ptr<Owner>> owners;

    // ========== Markets (Places of Exchange) ==========
    std::map<std::string, std::unique_ptr<Market>> markets; // product_name -> Market

    // ========== Government (Policy Maker) ==========
    std::unique_ptr<Government> government;

    // ========== Variable Change Tracking ==========
    struct VariableRelation
    {
        std::string target;
        RelationEffect effect;
    };

    std::map<std::string, std::vector<VariableRelation>> variableRelations;

    // ========== Propagation Engine ==========
    // The core system that connects all economic variables
    // Processes events through 3 tiers: Immediate → Decisions → Delayed
    std::unique_ptr<EconomicPropagation> propagator;

    // Tick counter: tracks simulation time steps
    // Each Step() advances by one tick; used by propagation for delayed effects
    int currentTick = 0;

    // Selected entities (for UI display)
    Worker *selected_worker;
    Farmer *selected_farmer;
    Owner *selected_owner;
    Market *selected_market;

    // Dashboard stats storage
    EconomicStats stats;
    EconomicStats prevStats;
    StatsCallback statsCallback;

public:
    // ========== Constructors ==========
    Simulation();
    ~Simulation() = default;

    // ========== Simulation Control ==========

    // Initialize with some default agents and markets
    void Initialize(int nuworkers, int nufarmers, int nuowners);

    // Refresh stats after any manual changes
    void RefreshStats();

    // Reset simulation to initial state
    void Reset();

    // ========== Step (Tick-based Simulation) ==========
    // Advance the simulation by one tick.
    // This is the MAIN LOOP method that processes everything:
    //   Phase 1: Market clearing - find equilibrium prices for all markets
    //   Phase 2: Tier 1 propagation - immediate effects (price → demand, revenue)
    //   Phase 3: Tier 2 propagation - agent decisions (profit → hiring, production)
    //   Phase 4: Tier 3 propagation - delayed macro effects (money → inflation)
    //   Phase 5: Government - collect taxes, update budget, enforce policies
    //   Phase 6: Statistics - refresh GDP, inflation, unemployment numbers
    void Step();

    // Get current simulation tick number
    int GetCurrentTick() const { return currentTick; }

    // ========== Propagation Engine Access ==========
    // Get the propagation engine for direct event emission or queries
    EconomicPropagation *GetPropagator() { return propagator.get(); }
    const EconomicPropagation *GetPropagator() const { return propagator.get(); }

    // ========== Agent Management ==========

    // Add agents
    void AddWorker(const std::string &name, double income, double skill);
    void AddFarmer(const std::string &name, double land, const std::string &crop);
    void AddOwner(const std::string &name, double capital,
                  const std::string &product, bool is_monopoly);

    // Get agents
    Worker *FindWorker(const std::string &name);
    Farmer *FindFarmer(const std::string &name);
    Owner *FindOwner(const std::string &name);

    // Get all agents (for iteration)
    const std::vector<std::unique_ptr<Worker>> &GetWorkers() const { return workers; }
    const std::vector<std::unique_ptr<Farmer>> &GetFarmers() const { return farmers; }
    const std::vector<std::unique_ptr<Owner>> &GetOwners() const { return owners; }

    // ========== Market Management ==========

    // Create market for a product
    void CreateMarket(const std::string &product_name);

    // Get market
    Market *FindMarket(const std::string &product_name);

    // Get all markets
    const std::map<std::string, std::unique_ptr<Market>> &GetMarkets() const
    {
        return markets;
    }

    // ========== Selection (for UI) ==========

    // Select agent to display details
    void SelectWorker(const std::string &name);
    void SelectFarmer(const std::string &name);
    void SelectOwner(const std::string &name);
    void SelectMarket(const std::string &product_name);

    // Get selected entities
    Worker *GetSelectedWorker() const { return selected_worker; }
    Farmer *GetSelectedFarmer() const { return selected_farmer; }
    Owner *GetSelectedOwner() const { return selected_owner; }
    Market *GetSelectedMarket() const { return selected_market; }

    // Clear selections
    void ClearSelection();

    // ========== Government Access ==========

    Government *GetGovernment() { return government.get(); }
    const Government *GetGovernment() const { return government.get(); }

    // ========== Stats Access ==========

    const EconomicStats &GetStats() const { return stats; }
    void SetStatsCallback(StatsCallback callback) { statsCallback = callback; }

    // ========== State Queries ==========

    int GetTotalWorkers() const { return workers.size(); }
    int GetTotalFarmers() const { return farmers.size(); }
    int GetTotalOwners() const { return owners.size(); }
    int GetTotalMarkets() const { return markets.size(); }

    // ========== Economic Aggregates ==========

    // Calculate total production value (for GDP)
    double CalculateTotalProduction();

    // Calculate unemployment rate
    double CalculateUnemploymentRate();

    // ========== Market & Harvest Operations ==========

    // Called after farmer harvests - updates market supply automatically
    void UpdateMarketFromHarvest(Farmer *farmer);

    // Recalculate all market equilibria based on consumer demand and current supply
    void RecalculateMarketEquilibria();

    // Update consumer demand for all products at current market prices
    void UpdateConsumerDemand();

    // Display current state
    std::string GetStatusString() const;

    // ========== Variable Relation Tracking ==========

    void RegisterVariableRelation(const std::string &source,
                                  const std::string &target,
                                  RelationEffect effect);

    void RecordVariableChange(const std::string &variable,
                              double oldValue,
                              double newValue);

    void RecordVariableChange(const std::string &variable,
                              const std::string &oldValue,
                              const std::string &newValue);
};
