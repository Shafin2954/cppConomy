#include "Firm.h"
#include "Person.h"
#include "Economy.h"
#include "../utils/Logger.h"
#include "../utils/Statistics.h"
#include <cmath>
#include <algorithm>
#include <sstream>

Firm::Firm(int id, Sector sector)
    : Agent(id), m_sector(sector), m_capital(Statistics::Random::getInstance().uniform(100000, 1000000)), m_inventory(1000), m_price(Config::INITIAL_PRICE), m_wage(Config::INITIAL_WAGE / Config::TICKS_PER_YEAR) // Monthly wage
      ,
      m_revenue(0.0), m_profit(0.0), m_lastProduction(0.0), m_targetInventory(5000), m_jobOpenings(5)
{
    m_wealth = m_capital;
}

void Firm::update(Economy &economy)
{
    if (!m_active)
        return;

    // Pay wages first
    payWages();

    // Produce goods
    produce(economy);

    // Adjust prices based on inventory
    setPrice(economy);

    // Decide on hiring/firing
    double capacityUtilization = m_lastProduction / std::max(1.0, calculateProductionCapacity());

    if (capacityUtilization > 0.9 && m_profit > 0)
    {
        // Need more workers
        m_jobOpenings = std::min(10, m_jobOpenings + 2);
    }
    else if (capacityUtilization < 0.5 || m_profit < -m_wage * m_laborForce.size())
    {
        // Too many workers or losing money
        if (!m_laborForce.empty() && m_laborForce.size() > 2)
        {
            Person *toFire = m_laborForce.back();
            fire(toFire);
        }
        m_jobOpenings = 0;
    }

    // Register job openings with labor market
    if (m_jobOpenings > 0)
    {
        economy.getLaborMarket().addJobOpening(this, m_jobOpenings);
    }

    // Check for bankruptcy
    if (m_wealth < -m_capital * 0.5)
    {
        LOG_WARNING("Firm #" + std::to_string(m_id) + " in " +
                    sectorToString(m_sector) + " went bankrupt!");

        // Fire all workers
        for (Person *worker : m_laborForce)
        {
            worker->loseJob();
        }
        m_laborForce.clear();
        deactivate();
    }
}

void Firm::produce(Economy &economy)
{
    // Cobb-Douglas production: Y = A * K^alpha * L^(1-alpha)
    double K = m_capital;
    double L = static_cast<double>(m_laborForce.size());

    if (L == 0)
    {
        m_lastProduction = 0;
        return;
    }

    // Calculate worker productivity based on skills
    double avgSkill = 0.0;
    for (Person *worker : m_laborForce)
    {
        avgSkill += worker->getSkillLevel(m_sector);
    }
    avgSkill = L > 0 ? avgSkill / L : 0.5;

    double A = Config::TFP * (0.5 + avgSkill); // TFP adjusted by skill
    double alpha = Config::CAPITAL_SHARE;

    double output = A * std::pow(K, alpha) * std::pow(L, 1.0 - alpha);

    // Scale to reasonable monthly production
    output = output / 10000.0;

    m_lastProduction = output;
    m_inventory += output;

    // Depreciation of capital (monthly)
    m_capital *= 0.995;
}

void Firm::setPrice(Economy &economy)
{
    // Adjust price based on inventory levels relative to target
    double inventoryRatio = m_inventory / std::max(1.0, m_targetInventory);

    // If inventory is high, lower price; if low, raise price
    double priceAdjustment = 1.0;
    if (inventoryRatio > 1.5)
    {
        priceAdjustment = 0.98; // Decrease price
    }
    else if (inventoryRatio < 0.5)
    {
        priceAdjustment = 1.02; // Increase price
    }

    m_price *= priceAdjustment;

    // Floor and ceiling
    m_price = std::max(10.0, std::min(10000.0, m_price));
}

void Firm::hire(Person *worker)
{
    if (!worker || worker->isEmployed())
        return;

    m_laborForce.push_back(worker);
    worker->setEmployer(this, m_wage);

    if (m_jobOpenings > 0)
    {
        m_jobOpenings--;
    }
}

void Firm::fire(Person *worker)
{
    auto it = std::find(m_laborForce.begin(), m_laborForce.end(), worker);
    if (it != m_laborForce.end())
    {
        (*it)->loseJob();
        m_laborForce.erase(it);
    }
}

void Firm::payWages()
{
    double totalWages = m_wage * m_laborForce.size();
    m_wealth -= totalWages;

    for (Person *worker : m_laborForce)
    {
        worker->addWealth(m_wage);
    }
}

void Firm::sellGoods(double quantity, double price)
{
    double actualSale = std::min(quantity, m_inventory);
    m_inventory -= actualSale;
    double saleRevenue = actualSale * price;
    m_revenue += saleRevenue;
    m_wealth += saleRevenue;

    // Calculate profit (simplified)
    m_profit = m_revenue - (m_wage * m_laborForce.size());
}

double Firm::calculateMarginalCost() const
{
    // MC increases with production level
    double L = static_cast<double>(m_laborForce.size());
    if (L == 0)
        return m_wage;
    return m_wage / (Config::TFP * std::pow(m_capital, Config::CAPITAL_SHARE) *
                     std::pow(L, -Config::CAPITAL_SHARE));
}

double Firm::calculateMarginalRevenue(Economy &economy) const
{
    // Simplified: MR decreases with quantity (downward sloping demand)
    return m_price * (1.0 - 0.1 * m_inventory / m_targetInventory);
}

double Firm::calculateOptimalOutput(Economy &economy) const
{
    // Find output where MC = MR (simplified approximation)
    return m_targetInventory * 0.8;
}

double Firm::calculateProductionCapacity() const
{
    double K = m_capital;
    double L = static_cast<double>(m_laborForce.size());
    if (L == 0)
        return 0;
    return Config::TFP * std::pow(K, Config::CAPITAL_SHARE) *
           std::pow(L, 1.0 - Config::CAPITAL_SHARE) / 10000.0;
}
