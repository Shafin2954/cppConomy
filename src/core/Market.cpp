#include "Market.h"
#include "Person.h"
#include "Firm.h"
#include "../utils/Statistics.h"
#include "../utils/Logger.h"
#include <algorithm>
#include <sstream>

void Market::addJobOpening(Firm *firm, int positions)
{
    if (!firm || positions <= 0)
        return;
    m_jobOpenings.push_back({firm, positions, firm->getWage()});
}

void Market::addJobSeeker(Person *person)
{
    if (!person)
        return;
    // Reservation wage is 80% of average wage
    m_jobSeekers.push_back({person, m_averageWage * 0.8});
}

void Market::clearLaborMarket()
{
    auto &rng = Statistics::Random::getInstance();

    // Shuffle job seekers for random matching
    std::vector<JobSeeker> shuffled = m_jobSeekers;
    for (size_t i = shuffled.size() - 1; i > 0; --i)
    {
        size_t j = rng.uniformInt(0, static_cast<int>(i));
        std::swap(shuffled[i], shuffled[j]);
    }

    int hires = 0;

    for (auto &seeker : shuffled)
    {
        if (seeker.person->isEmployed())
            continue;

        // Find a matching job
        for (auto &opening : m_jobOpenings)
        {
            if (opening.positions <= 0)
                continue;
            if (opening.wage < seeker.reservationWage)
                continue;

            // Check skill match
            double skillLevel = seeker.person->getSkillLevel(opening.firm->getSector());
            if (skillLevel < 0.2 && rng.uniform() > 0.3)
                continue; // Low skill, low chance

            // Hire!
            opening.firm->hire(seeker.person);
            opening.positions--;
            hires++;
            break;
        }
    }

    // Update average wage
    double totalWages = 0.0;
    int wageCount = 0;
    for (const auto &opening : m_jobOpenings)
    {
        totalWages += opening.wage * (opening.positions + 1);
        wageCount += opening.positions + 1;
    }
    if (wageCount > 0)
    {
        m_averageWage = totalWages / wageCount;
    }

    if (hires > 0)
    {
        LOG_INFO("Labor market: " + std::to_string(hires) + " workers hired");
    }

    // Clear for next tick
    m_jobOpenings.clear();
    m_jobSeekers.clear();
}

void Market::addSupply(Firm *firm, double quantity, double price)
{
    if (!firm || quantity <= 0)
        return;
    m_supplies.push_back({firm, quantity, price});
    m_totalSupply += quantity;
}

void Market::addDemand(double amount)
{
    m_totalDemand += amount;
}

void Market::clearGoodsMarket()
{
    if (m_supplies.empty() || m_totalDemand <= 0)
    {
        m_supplies.clear();
        m_totalDemand = 0;
        m_totalSupply = 0;
        return;
    }

    // Sort by price (lowest first - competitive market)
    std::sort(m_supplies.begin(), m_supplies.end(),
              [](const Supply &a, const Supply &b)
              { return a.price < b.price; });

    double remainingDemand = m_totalDemand;
    double totalValue = 0.0;
    double totalQuantity = 0.0;

    for (auto &supply : m_supplies)
    {
        if (remainingDemand <= 0)
            break;

        double soldQuantity = std::min(supply.quantity, remainingDemand);
        supply.firm->sellGoods(soldQuantity, supply.price);

        totalValue += soldQuantity * supply.price;
        totalQuantity += soldQuantity;
        remainingDemand -= soldQuantity;
    }

    // Update average price
    if (totalQuantity > 0)
    {
        m_averagePrice = totalValue / totalQuantity;
    }

    // Clear for next tick
    m_supplies.clear();
    m_totalDemand = 0;
    m_totalSupply = 0;
}

int Market::getJobOpenings() const
{
    int total = 0;
    for (const auto &opening : m_jobOpenings)
    {
        total += opening.positions;
    }
    return total;
}

void Market::reset()
{
    m_jobOpenings.clear();
    m_jobSeekers.clear();
    m_supplies.clear();
    m_totalDemand = 0;
    m_totalSupply = 0;
}
