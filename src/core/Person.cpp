#include "Person.h"
#include <sstream>
#include <algorithm>
#include <cmath>

// ============================================================================
// Person.cpp - Minimal implementation with clear economic logic
// ============================================================================

Person::Person(int id, const std::string &name, double initial_income)
    : m_id(id), m_name(name), m_age(18), m_is_alive(true),
      m_literacy_score(0.5), m_wallet(initial_income), m_savings(0.0),
      m_monthly_income(initial_income), m_total_utility(0.0),
      m_addiction_level(0.0), m_income_tax_paid(0.0)
{
    // Default preferences (can be updated from commands)
    m_preferences["rice"] = 1.0;
    m_preferences["ice_cream"] = 1.0;
    m_preferences["curd"] = 1.0;
}

void Person::SetPreference(const std::string &product, double weight)
{
    m_preferences[product] = std::max(0.0, weight);
}

void Person::Consume(const std::string &product, double quantity, double price)
{
    if (!m_is_alive || quantity <= 0.0 || price <= 0.0)
        return;

    double total_cost = quantity * price;

    // Budget constraint: cannot spend more than wallet
    if (total_cost > m_wallet)
    {
        // Buy as much as possible with remaining budget
        quantity = m_wallet / price;
        total_cost = quantity * price;
    }

    // Spend and update owned goods
    m_wallet -= total_cost;
    m_goods_owned[product] += quantity;

    // Update utility after consumption
    UpdateUtility();
}

void Person::UpdateUtility()
{
    // Simple diminishing marginal utility:
    // Utility = sum over goods of preference * log(1 + quantity)
    double utility = 0.0;
    for (const auto &item : m_goods_owned)
    {
        const std::string &product = item.first;
        double quantity = item.second;
        double preference = 1.0;

        auto it = m_preferences.find(product);
        if (it != m_preferences.end())
            preference = it->second;

        utility += preference * std::log(1.0 + quantity);
    }

    // Addiction: if addiction level is high, utility from addictive goods is boosted
    if (m_addiction_level > 0.0)
    {
        auto it = m_goods_owned.find("ice_cream");
        if (it != m_goods_owned.end())
            utility += m_addiction_level * std::log(1.0 + it->second);
    }

    m_total_utility = utility;
}

void Person::SubstituteGood(const std::string &from_product,
                            const std::string &to_product,
                            double quantity)
{
    if (quantity <= 0.0)
        return;

    double &from_qty = m_goods_owned[from_product];
    double actual = std::min(from_qty, quantity);

    from_qty -= actual;
    m_goods_owned[to_product] += actual;

    UpdateUtility();
}

void Person::PayTax(double tax_amount)
{
    if (tax_amount <= 0.0)
        return;

    double paid = std::min(m_wallet, tax_amount);
    m_wallet -= paid;
    m_income_tax_paid += paid;
}

void Person::AgeTick()
{
    if (!m_is_alive)
        return;

    m_age++;

    // Simple mortality rule: small chance after 60
    if (m_age > 60)
    {
        double death_chance = 0.01 * (m_age - 60); // grows with age
        if (death_chance > 0.5)
            death_chance = 0.5;

        // Deterministic mortality for now (no RNG) – die at 90
        if (m_age >= 90)
            m_is_alive = false;
    }

    // Literacy slowly improves with time (simplified)
    m_literacy_score = std::min(1.0, m_literacy_score + 0.005);
}

std::string Person::GetInfoString() const
{
    std::ostringstream ss;
    ss << "Person: " << m_name << " (ID=" << m_id << ")\n";
    ss << "Age: " << m_age << ", Alive: " << (m_is_alive ? "Yes" : "No") << "\n";
    ss << "Income: " << m_monthly_income << ", Wallet: " << m_wallet
       << ", Savings: " << m_savings << "\n";
    ss << "Utility: " << m_total_utility << ", Literacy: " << m_literacy_score << "\n";
    ss << "Addiction: " << m_addiction_level << ", Tax Paid: " << m_income_tax_paid << "\n";

    ss << "Goods Owned: ";
    if (m_goods_owned.empty())
    {
        ss << "(none)";
    }
    else
    {
        bool first = true;
        for (const auto &item : m_goods_owned)
        {
            if (!first)
                ss << ", ";
            ss << item.first << "=" << item.second;
            first = false;
        }
    }
    ss << "\n";

    return ss.str();
}

double Person::CalculateMarginalUtility(const std::string &product, double quantity)
{
    // MU = preference / (1 + quantity)
    double preference = 1.0;
    auto it = m_preferences.find(product);
    if (it != m_preferences.end())
        preference = it->second;

    return preference / (1.0 + quantity);
}
