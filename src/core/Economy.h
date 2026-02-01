#pragma once

#include "Person.h"
#include "Firm.h"
#include "Government.h"
#include "Market.h"
#include "../utils/Config.h"
#include "../utils/Statistics.h"
#include <vector>
#include <memory>
#include <functional>

// Forward declarations
class PhillipsCurve;
class OkunLaw;
class QuantityTheory;
class StochasticShock;

struct EconomicStats
{
    // Core indicators
    double gdp = 0.0;
    double gdpGrowth = 0.0;
    double realGdp = 0.0;

    // Price indicators
    double cpi = 100.0;
    double inflation = 0.0;
    double priceLevel = 1.0;

    // Labor indicators
    double unemployment = 0.0;
    double laborForceParticipation = 0.0;
    double averageWage = 0.0;

    // Money indicators
    double moneySupply = 0.0;
    double velocity = Config::MONEY_VELOCITY;
    double interestRate = 0.0;

    // Government
    double taxRevenue = 0.0;
    double govSpending = 0.0;
    double budget = 0.0;
    double debt = 0.0;

    // Population
    int population = 0;
    int employed = 0;
    int firms = 0;

    // Aggregates
    double totalConsumption = 0.0;
    double totalProduction = 0.0;
    double totalInvestment = 0.0;

    // Distribution
    double giniCoefficient = 0.0;

    int currentTick = 0;
};

class Economy
{
public:
    using StatsCallback = std::function<void(const EconomicStats &)>;

    Economy();
    ~Economy();

    // Simulation control
    void initialize(int population, int firms);
    void tick();
    void reset();

    // Agent management
    void addCitizen(int count = 1);
    void addFirm(int count = 1, Sector sector = Sector::CPPC_Services);
    void removeCitizen(int id);
    void removeFirm(int id);

    // Market access
    Market &getLaborMarket() { return m_laborMarket; }
    Market &getGoodsMarket() { return m_goodsMarket; }

    // Government access
    Government &getGovernment() { return Government::getInstance(); }

    // Consumption tracking
    void recordConsumption(double amount);

    // Stats
    const EconomicStats &getStats() const { return m_stats; }
    void setStatsCallback(StatsCallback callback) { m_statsCallback = callback; }

    // Shock triggers
    void triggerShock(const std::string &type, double severity = 1.0);

    // State
    bool isRunning() const { return m_running; }
    void setRunning(bool running) { m_running = running; }
    int getCurrentTick() const { return m_currentTick; }

private:
    void updateAgents();
    void clearMarkets();
    void calculateAggregates();
    void applyEconomicModels();
    void checkForShocks();
    void handleBirthsAndDeaths();
    void distributeStimulus(double amount, Sector sector);

    std::vector<std::unique_ptr<Person>> m_citizens;
    std::vector<std::unique_ptr<Firm>> m_firms;

    Market m_laborMarket;
    Market m_goodsMarket;

    EconomicStats m_stats;
    EconomicStats m_prevStats;

    StatsCallback m_statsCallback;

    double m_totalConsumption;
    double m_baseCpi;
    int m_currentTick;
    int m_nextAgentId;
    bool m_running;
};
