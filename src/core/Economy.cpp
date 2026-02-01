#include "Economy.h"
#include "../models/PhillipsCurve.h"
#include "../models/OkunLaw.h"
#include "../models/QuantityTheory.h"
#include "../models/StochasticShock.h"
#include "../utils/Logger.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

Economy::Economy()
    : m_laborMarket(Market::Type::LABOR), m_goodsMarket(Market::Type::GOODS), m_totalConsumption(0.0), m_baseCpi(100.0), m_currentTick(0), m_nextAgentId(1), m_running(false)
{
    Statistics::Random::getInstance().seed(Config::RANDOM_SEED);
}

Economy::~Economy() = default;

void Economy::initialize(int population, int firms)
{
    reset();

    LOG_INFO("Initializing economy with " + std::to_string(population) +
             " citizens and " + std::to_string(firms) + " firms");

    // Create citizens with age distribution
    auto &rng = Statistics::Random::getInstance();
    for (int i = 0; i < population; ++i)
    {
        // Age distribution (skewed toward working age)
        int age = static_cast<int>(rng.normal(35, 15));
        age = std::max(0, std::min(Config::MAX_AGE, age));

        auto person = std::make_unique<Person>(m_nextAgentId++, age);
        m_citizens.push_back(std::move(person));
    }

    // Create firms across sectors
    int firmsPerSector = firms / static_cast<int>(Sector::CPPC_Count);
    for (int s = 0; s < static_cast<int>(Sector::CPPC_Count); ++s)
    {
        Sector sector = static_cast<Sector>(s);
        int sectorFirms = (s == static_cast<int>(Sector::CPPC_Count) - 1) ? (firms - firmsPerSector * s) : firmsPerSector;

        for (int i = 0; i < sectorFirms; ++i)
        {
            auto firm = std::make_unique<Firm>(m_nextAgentId++, sector);
            m_firms.push_back(std::move(firm));
        }
    }

    // Initial hiring - assign some workers to firms
    std::vector<Person *> availableWorkers;
    for (auto &person : m_citizens)
    {
        if (person->canWork() && !person->isEmployed())
        {
            availableWorkers.push_back(person.get());
        }
    }

    // Shuffle workers
    for (size_t i = availableWorkers.size() - 1; i > 0; --i)
    {
        size_t j = rng.uniformInt(0, static_cast<int>(i));
        std::swap(availableWorkers[i], availableWorkers[j]);
    }

    // Assign to firms (aim for ~96% employment initially)
    size_t targetEmployed = static_cast<size_t>(availableWorkers.size() * 0.96);
    size_t workerIdx = 0;

    for (auto &firm : m_firms)
    {
        int toHire = rng.uniformInt(5, 20);
        for (int i = 0; i < toHire && workerIdx < targetEmployed; ++i, ++workerIdx)
        {
            firm->hire(availableWorkers[workerIdx]);
        }
    }

    // Initialize stats
    calculateAggregates();
    m_stats.moneySupply = getGovernment().getMoneySupply();
    m_stats.interestRate = getGovernment().getInterestRate();

    LOG_SUCCESS("Economy initialized successfully");

    if (m_statsCallback)
    {
        m_statsCallback(m_stats);
    }
}

void Economy::tick()
{
    if (!m_running)
        return;

    m_currentTick++;
    Logger::getInstance().setCurrentTick(m_currentTick);
    getGovernment().resetMonthlyStats();
    m_totalConsumption = 0.0;

    // Store previous stats for comparison
    m_prevStats = m_stats;

    // 1. Check for random shocks
    checkForShocks();

    // 2. Handle births and deaths
    handleBirthsAndDeaths();

    // 3. Update all agents
    updateAgents();

    // 4. Clear markets (matching supply/demand)
    clearMarkets();

    // 5. Calculate aggregate statistics
    calculateAggregates();

    // 6. Apply economic model adjustments
    applyEconomicModels();

    // Notify listeners
    if (m_statsCallback)
    {
        m_statsCallback(m_stats);
    }

    // Log periodic updates
    if (m_currentTick % Config::TICKS_PER_YEAR == 0)
    {
        int year = m_currentTick / Config::TICKS_PER_YEAR;
        std::stringstream ss;
        ss << std::fixed << std::setprecision(1);
        ss << "Year " << year << " Summary: GDP $" << (m_stats.gdp / 1e9) << "B, "
           << "Unemployment " << (m_stats.unemployment * 100) << "%, "
           << "Inflation " << (m_stats.inflation * 100) << "%";
        LOG_INFO(ss.str());
    }
}

void Economy::reset()
{
    m_citizens.clear();
    m_firms.clear();
    m_laborMarket.reset();
    m_goodsMarket.reset();
    m_stats = EconomicStats();
    m_prevStats = EconomicStats();
    m_totalConsumption = 0.0;
    m_currentTick = 0;
    m_nextAgentId = 1;
    m_running = false;
    Logger::getInstance().clear();
}

void Economy::addCitizen(int count)
{
    auto &rng = Statistics::Random::getInstance();
    for (int i = 0; i < count; ++i)
    {
        int age = rng.uniformInt(18, 30); // New workers
        auto person = std::make_unique<Person>(m_nextAgentId++, age);
        m_citizens.push_back(std::move(person));
    }
    LOG_INFO("Added " + std::to_string(count) + " new citizens");
}

void Economy::addFirm(int count, Sector sector)
{
    for (int i = 0; i < count; ++i)
    {
        auto firm = std::make_unique<Firm>(m_nextAgentId++, sector);
        m_firms.push_back(std::move(firm));
    }
    LOG_INFO("Added " + std::to_string(count) + " new firms in " + sectorToString(sector));
}

void Economy::removeCitizen(int id)
{
    auto it = std::find_if(m_citizens.begin(), m_citizens.end(),
                           [id](const auto &p)
                           { return p->getId() == id; });
    if (it != m_citizens.end())
    {
        (*it)->deactivate();
    }
}

void Economy::removeFirm(int id)
{
    auto it = std::find_if(m_firms.begin(), m_firms.end(),
                           [id](const auto &f)
                           { return f->getId() == id; });
    if (it != m_firms.end())
    {
        (*it)->deactivate();
    }
}

void Economy::recordConsumption(double amount)
{
    m_totalConsumption += amount;
    m_goodsMarket.addDemand(amount / m_goodsMarket.getAveragePrice());
}

void Economy::triggerShock(const std::string &type, double severity)
{
    StochasticShock::triggerShock(*this, type, severity);
}

void Economy::updateAgents()
{
    // Update all citizens
    for (auto &person : m_citizens)
    {
        if (person->isActive())
        {
            person->update(*this);
        }
    }

    // Update all firms
    for (auto &firm : m_firms)
    {
        if (firm->isActive())
        {
            firm->update(*this);

            // Add firm's inventory to goods market
            m_goodsMarket.addSupply(firm.get(), firm->getInventory(), firm->getPrice());
        }
    }
}

void Economy::clearMarkets()
{
    m_laborMarket.clearLaborMarket();
    m_goodsMarket.clearGoodsMarket();
}

void Economy::calculateAggregates()
{
    m_stats.currentTick = m_currentTick;

    // Count active agents
    int activePopulation = 0;
    int employed = 0;
    int laborForce = 0;
    double totalWealth = 0.0;
    std::vector<double> wealthDistribution;

    for (const auto &person : m_citizens)
    {
        if (!person->isActive())
            continue;
        activePopulation++;
        totalWealth += person->getWealth();
        wealthDistribution.push_back(person->getWealth());

        if (person->canWork())
        {
            laborForce++;
            if (person->isEmployed())
            {
                employed++;
            }
        }
    }

    int activeFirms = 0;
    double totalProduction = 0.0;
    double totalRevenue = 0.0;
    double totalWages = 0.0;

    for (const auto &firm : m_firms)
    {
        if (!firm->isActive())
            continue;
        activeFirms++;
        totalProduction += firm->getProduction();
        totalRevenue += firm->getRevenue();
        totalWages += firm->getWage() * firm->getWorkerCount();
    }

    // Update stats
    m_stats.population = activePopulation;
    m_stats.employed = employed;
    m_stats.firms = activeFirms;

    // Unemployment rate
    m_stats.unemployment = laborForce > 0 ? static_cast<double>(laborForce - employed) / laborForce : 0.0;

    m_stats.laborForceParticipation = activePopulation > 0 ? static_cast<double>(laborForce) / activePopulation : 0.0;

    // GDP = C + I + G + (X - M)  [simplified to C + production value]
    m_stats.totalConsumption = m_totalConsumption;
    m_stats.totalProduction = totalProduction * m_goodsMarket.getAveragePrice();
    m_stats.gdp = m_stats.totalConsumption + m_stats.totalProduction * 0.3; // Simplified

    // GDP Growth
    if (m_prevStats.gdp > 0)
    {
        m_stats.gdpGrowth = (m_stats.gdp - m_prevStats.gdp) / m_prevStats.gdp;
    }

    // Price level and CPI
    double avgPrice = m_goodsMarket.getAveragePrice();
    m_stats.priceLevel = avgPrice / Config::INITIAL_PRICE;
    m_stats.cpi = m_baseCpi * m_stats.priceLevel;

    // Inflation (annualized)
    if (m_prevStats.cpi > 0)
    {
        double monthlyInflation = (m_stats.cpi - m_prevStats.cpi) / m_prevStats.cpi;
        m_stats.inflation = monthlyInflation * Config::TICKS_PER_YEAR; // Annualized
    }

    // Wages
    m_stats.averageWage = m_laborMarket.getAverageWage() * Config::TICKS_PER_YEAR;

    // Government stats
    auto &gov = getGovernment();
    m_stats.moneySupply = gov.getMoneySupply();
    m_stats.interestRate = gov.getInterestRate();
    m_stats.taxRevenue = gov.getTaxRevenue();
    m_stats.govSpending = gov.getSpending();
    m_stats.budget = gov.getBudget();
    m_stats.debt = gov.getDebt();

    // Gini coefficient
    m_stats.giniCoefficient = Statistics::giniCoefficient(wealthDistribution);

    // Remove inactive agents
    m_citizens.erase(
        std::remove_if(m_citizens.begin(), m_citizens.end(),
                       [](const auto &p)
                       { return !p->isActive(); }),
        m_citizens.end());

    m_firms.erase(
        std::remove_if(m_firms.begin(), m_firms.end(),
                       [](const auto &f)
                       { return !f->isActive(); }),
        m_firms.end());
}

void Economy::applyEconomicModels()
{
    // Phillips Curve: adjust inflation based on unemployment gap
    double naturalUnemployment = Config::NATURAL_UNEMPLOYMENT;
    double unemploymentGap = m_stats.unemployment - naturalUnemployment;
    double phillipsInflation = PhillipsCurve::calculate(m_stats.unemployment,
                                                        naturalUnemployment,
                                                        m_stats.inflation);

    // Okun's Law: relate GDP gap to unemployment
    double potentialGdp = OkunLaw::calculatePotentialGdp(m_stats.gdp,
                                                         m_stats.unemployment,
                                                         naturalUnemployment);
    m_stats.realGdp = potentialGdp;

    // Quantity Theory: MV = PQ
    double impliedPriceLevel = QuantityTheory::calculatePriceLevel(
        m_stats.moneySupply, m_stats.velocity, m_stats.totalProduction);

    // Gradually adjust price toward equilibrium
    if (impliedPriceLevel > 0 && m_stats.priceLevel > 0)
    {
        double adjustment = (impliedPriceLevel / m_stats.priceLevel - 1.0) * 0.1;
        m_baseCpi *= (1.0 + adjustment);
    }
}

void Economy::checkForShocks()
{
    auto &rng = Statistics::Random::getInstance();

    // Check for each type of shock
    if (rng.bernoulli(0.005 / Config::TICKS_PER_YEAR))
    { // ~0.5% per year
        triggerShock("pandemic", rng.uniform(0.5, 1.0));
    }
    else if (rng.bernoulli(0.01 / Config::TICKS_PER_YEAR))
    { // ~1% per year
        triggerShock("market_crash", rng.uniform(0.5, 1.0));
    }
    else if (rng.bernoulli(0.02 / Config::TICKS_PER_YEAR))
    { // ~2% per year
        triggerShock("tech_boom", rng.uniform(0.5, 1.0));
    }
    else if (rng.bernoulli(0.01 / Config::TICKS_PER_YEAR))
    { // ~1% per year
        triggerShock("oil_shock", rng.uniform(0.5, 1.0));
    }
}

void Economy::handleBirthsAndDeaths()
{
    auto &rng = Statistics::Random::getInstance();

    // Birth rate (~1% per year, distributed monthly)
    double birthRate = 0.01 / Config::TICKS_PER_YEAR;
    int births = static_cast<int>(m_stats.population * birthRate);
    births += rng.bernoulli(m_stats.population * birthRate - births) ? 1 : 0;

    for (int i = 0; i < births; ++i)
    {
        auto person = std::make_unique<Person>(m_nextAgentId++, 0);
        m_citizens.push_back(std::move(person));
    }

    // Deaths are handled in Person::checkDeath()
}

void Economy::distributeStimulus(double amount, Sector sector)
{
    // Distribute stimulus to citizens
    double perPerson = amount / std::max(1, m_stats.population);
    for (auto &person : m_citizens)
    {
        if (person->isActive())
        {
            person->addWealth(perPerson);
        }
    }
}
