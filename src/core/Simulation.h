#pragma once

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <functional>
#include "Person.h"
#include "Worker.h"
#include "Farmer.h"
#include "Owner.h"
#include "Market.h"
#include "Government.h"

// ============================================================================
// Simulation.h - Central controller managing the entire economic simulation
//
// Manages: Creation/destruction of all agents, markets, government
//          Coordination of market transactions
//          Variable change tracking and relation mapping
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
    std::vector<std::unique_ptr<Worker>> m_workers;
    std::vector<std::unique_ptr<Farmer>> m_farmers;
    std::vector<std::unique_ptr<Owner>> m_owners;

    // ========== Markets (Places of Exchange) ==========
    std::map<std::string, std::unique_ptr<Market>> m_markets; // product_name -> Market

    // ========== Government (Policy Maker) ==========
    std::unique_ptr<Government> m_government;

    // ========== Variable Change Tracking ==========
    struct VariableRelation
    {
        std::string target;
        RelationEffect effect;
    };

    std::map<std::string, std::vector<VariableRelation>> m_variableRelations;

    // Selected entities (for UI display)
    Worker *m_selected_worker;
    Farmer *m_selected_farmer;
    Owner *m_selected_owner;
    Market *m_selected_market;

    // Dashboard stats storage
    EconomicStats m_stats;
    EconomicStats m_prevStats;
    StatsCallback m_statsCallback;

public:
    // ========== Constructors ==========
    Simulation();
    ~Simulation() = default;

    // ========== Simulation Control ==========

    // Initialize with some default agents and markets
    void Initialize(int num_workers, int num_farmers, int num_owners);

    // Refresh stats after any manual changes
    void RefreshStats();

    // Reset simulation to initial state
    void Reset();

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
    const std::vector<std::unique_ptr<Worker>> &GetWorkers() const { return m_workers; }
    const std::vector<std::unique_ptr<Farmer>> &GetFarmers() const { return m_farmers; }
    const std::vector<std::unique_ptr<Owner>> &GetOwners() const { return m_owners; }

    // ========== Market Management ==========

    // Create market for a product
    void CreateMarket(const std::string &product_name);

    // Get market
    Market *FindMarket(const std::string &product_name);

    // Get all markets
    const std::map<std::string, std::unique_ptr<Market>> &GetMarkets() const
    {
        return m_markets;
    }

    // ========== Selection (for UI) ==========

    // Select agent to display details
    void SelectWorker(const std::string &name);
    void SelectFarmer(const std::string &name);
    void SelectOwner(const std::string &name);
    void SelectMarket(const std::string &product_name);

    // Get selected entities
    Worker *GetSelectedWorker() const { return m_selected_worker; }
    Farmer *GetSelectedFarmer() const { return m_selected_farmer; }
    Owner *GetSelectedOwner() const { return m_selected_owner; }
    Market *GetSelectedMarket() const { return m_selected_market; }

    // Clear selections
    void ClearSelection();

    // ========== Government Access ==========

    Government *GetGovernment() { return m_government.get(); }
    const Government *GetGovernment() const { return m_government.get(); }

    // ========== Stats Access ==========

    const EconomicStats &GetStats() const { return m_stats; }
    void SetStatsCallback(StatsCallback callback) { m_statsCallback = callback; }

    // ========== State Queries ==========

    int GetTotalWorkers() const { return m_workers.size(); }
    int GetTotalFarmers() const { return m_farmers.size(); }
    int GetTotalOwners() const { return m_owners.size(); }
    int GetTotalMarkets() const { return m_markets.size(); }

    // ========== Economic Aggregates ==========

    // Calculate total production value (for GDP)
    double CalculateTotalProduction();

    // Calculate unemployment rate
    double CalculateUnemploymentRate();

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
