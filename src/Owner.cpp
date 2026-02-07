#include "Owner.h"
#include <sstream>
#include <cmath>
#include <algorithm>

Owner::Owner(int id, const std::string &name, double initial_capital,
             const std::string &product_type, bool is_monopoly)
    : Person(id, name, initial_capital),
      m_capital_machines(initial_capital),
      m_labor_hired(0),
      m_technology_level(1.0),
      m_product_type(product_type),
      m_production_output(0.0),
      m_is_monopoly(is_monopoly),
      m_current_price(0.0),
      m_revenue(0.0),
      m_total_fixed_cost(200.0),
      m_total_variable_cost(0.0),
      m_total_cost(0.0),
      m_average_fixed_cost(0.0),
      m_average_variable_cost(0.0),
      m_average_cost(0.0),
      m_marginal_cost(0.0),
      m_marginal_revenue(0.0),
      m_profit(0.0),
      m_is_operating(true)
{
}

void Owner::Hire()
{
    m_labor_hired += 1;
}

void Owner::Fire()
{
    if (m_labor_hired > 0)
        m_labor_hired -= 1;
}

void Owner::Produce()
{
    // Simplified production function: Q = (labor * tech) * sqrt(capital)
    double capital_factor = std::sqrt(std::max(1.0, m_capital_machines));
    m_production_output = (m_labor_hired * m_technology_level) * capital_factor;
}

void Owner::CalculateCosts(double wage_rate, double material_cost_per_unit)
{
    m_total_variable_cost = (m_labor_hired * wage_rate) + (m_production_output * material_cost_per_unit);
    m_total_cost = m_total_fixed_cost + m_total_variable_cost;

    if (m_production_output > 0)
    {
        m_average_fixed_cost = m_total_fixed_cost / m_production_output;
        m_average_variable_cost = m_total_variable_cost / m_production_output;
        m_average_cost = m_total_cost / m_production_output;
        m_marginal_cost = m_total_variable_cost / m_production_output;
    }
    else
    {
        m_average_fixed_cost = 0.0;
        m_average_variable_cost = 0.0;
        m_average_cost = 0.0;
        m_marginal_cost = 0.0;
    }
}

void Owner::CalculateRevenue()
{
    m_revenue = m_current_price * m_production_output;
}

void Owner::CalculateProfit()
{
    m_profit = m_revenue - m_total_cost;
}

int Owner::GetOptimalQuantity(double market_price)
{
    // Simple rule: produce where price ~= marginal cost
    m_current_price = market_price;

    if (m_marginal_cost <= 0.0)
        return 0;

    // If MC < Price, produce more; if MC > Price, produce less
    // Here, just approximate optimal quantity as current output
    return static_cast<int>(m_production_output);
}

void Owner::CheckShutdownCondition(double market_price)
{
    m_current_price = market_price;

    // Shutdown if price < AVC
    if (m_average_variable_cost > 0.0 && market_price < m_average_variable_cost)
    {
        m_is_operating = false;
        m_production_output = 0.0;
    }
    else
    {
        m_is_operating = true;
    }
}

void Owner::ExpandFactory()
{
    // Double capital and labor
    m_capital_machines *= 2.0;
    m_labor_hired *= 2;
    m_technology_level *= 1.1;
}

void Owner::SetMonopolyPrice()
{
    if (!m_is_monopoly)
        return;

    // Simple monopoly pricing: set price above MC
    if (m_marginal_cost > 0.0)
        m_current_price = m_marginal_cost * 1.5;
}

std::string Owner::GetInfoString() const
{
    std::ostringstream ss;
    ss << "Owner: " << m_name << " (ID=" << m_id << ")\n";
    ss << "Product: " << m_product_type << ", Monopoly: " << (m_is_monopoly ? "Yes" : "No") << "\n";
    ss << "Capital: " << m_capital_machines << ", Labor: " << m_labor_hired << "\n";
    ss << "Output: " << m_production_output << ", Price: " << m_current_price << "\n";
    ss << "Revenue: " << m_revenue << ", Profit: " << m_profit << "\n";
    ss << "Costs - TC: " << m_total_cost << ", AC: " << m_average_cost
       << ", MC: " << m_marginal_cost << "\n";
    return ss.str();
}
