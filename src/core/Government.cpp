#include "Government.h"
#include "../utils/Logger.h"
#include <sstream>
#include <iomanip>

Government::Government()
    : m_taxRate(0.20), m_corporateTaxRate(0.21), m_interestRate(0.05), m_moneySupply(Config::INITIAL_MONEY_SUPPLY), m_budget(1e11) // 100 billion starting budget
      ,
      m_debt(0.0), m_spending(0.0), m_taxRevenue(0.0)
{
}

void Government::adjustTax(double rate, const std::string &target)
{
    rate = std::max(0.0, std::min(1.0, rate)); // Clamp to [0, 1]

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);

    if (target == "corporate" || target == "business")
    {
        double oldRate = m_corporateTaxRate;
        m_corporateTaxRate = rate;
        ss << "Corporate tax adjusted: " << (oldRate * 100) << "% -> " << (rate * 100) << "%";
    }
    else if (target == "income" || target == "personal")
    {
        double oldRate = m_taxRate;
        m_taxRate = rate;
        ss << "Income tax adjusted: " << (oldRate * 100) << "% -> " << (rate * 100) << "%";
    }
    else
    {
        // Adjust both
        double oldIncome = m_taxRate;
        double oldCorp = m_corporateTaxRate;
        m_taxRate = rate;
        m_corporateTaxRate = rate;
        ss << "All taxes adjusted to " << (rate * 100) << "%";
    }

    LOG_SUCCESS(ss.str());
}

void Government::setInterestRate(double rate)
{
    rate = std::max(0.0, std::min(0.25, rate)); // Clamp to [0%, 25%]

    double oldRate = m_interestRate;
    m_interestRate = rate;

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "Interest rate adjusted: " << (oldRate * 100) << "% -> " << (rate * 100) << "%";
    LOG_SUCCESS(ss.str());
}

void Government::injectMoney(double amount)
{
    m_moneySupply += amount;
    m_debt += amount; // Simplification: money injection increases debt

    std::stringstream ss;
    ss << std::fixed << std::setprecision(0);
    ss << "Money supply increased by $" << amount << " (QE)";
    LOG_SUCCESS(ss.str());
}

void Government::grantStimulus(double amount, const std::string &sector)
{
    if (amount > m_budget)
    {
        // Borrow the difference
        double deficit = amount - m_budget;
        m_debt += deficit;
        m_budget = 0;
        LOG_WARNING("Deficit spending: borrowed $" + std::to_string(static_cast<long long>(deficit)));
    }
    else
    {
        m_budget -= amount;
    }

    m_spending += amount;

    std::stringstream ss;
    ss << std::fixed << std::setprecision(0);
    ss << "Stimulus of $" << amount << " granted";
    if (sector != "all")
    {
        ss << " to " << sector << " sector";
    }
    LOG_SUCCESS(ss.str());
}

void Government::collectTaxes(double income)
{
    double taxCollected = income * m_taxRate;
    m_budget += taxCollected;
    m_taxRevenue += taxCollected;
}

void Government::spend(double amount)
{
    if (amount > m_budget)
    {
        double deficit = amount - m_budget;
        m_debt += deficit;
        m_budget = 0;
    }
    else
    {
        m_budget -= amount;
    }
    m_spending += amount;
}
