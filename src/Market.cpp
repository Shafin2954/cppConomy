#include "Market.h"
#include <sstream>
#include <algorithm>

Market::Market(const std::string &product_name)
    : m_product_name(product_name),
      m_quantity_demanded(0.0),
      m_quantity_supplied(0.0),
      m_current_price(0.0),
      m_equilibrium_price(0.0),
      m_last_price(0.0),
      m_shortage_surplus(0.0),
      m_tax_rate(0.0),
      m_subsidy_rate(0.0),
      m_min_wage(0.0),
      m_price_ceiling(0.0),
      m_price_floor(0.0),
      m_total_sales_value(0.0)
{
}

void Market::FindEquilibrium()
{
    // Simplified equilibrium: set price where demand == supply
    if (m_quantity_demanded + m_quantity_supplied > 0)
    {
        m_equilibrium_price = m_current_price;
    }
    else
    {
        m_equilibrium_price = 0.0;
    }
}

void Market::AdjustPrice()
{
    m_shortage_surplus = m_quantity_demanded - m_quantity_supplied;

    // Only adjust price if there's actual supply/demand activity
    if (m_quantity_demanded > 0 || m_quantity_supplied > 0)
    {
        if (m_shortage_surplus > 0)
        {
            m_current_price *= 1.02; // shortage -> price rises
        }
        else if (m_shortage_surplus < 0)
        {
            m_current_price *= 0.98; // surplus -> price falls
        }
    }
    // If no supply/demand, price stays as set by user

    if (m_current_price < 0)
        m_current_price = 0;
}

void Market::ApplyTaxAndSubsidy()
{
    // Taxes raise effective price; subsidies lower it
    double net_effect = m_tax_rate - m_subsidy_rate;
    if (net_effect > 0)
        m_current_price *= (1.0 + net_effect);
    else if (net_effect < 0)
        m_current_price *= (1.0 + net_effect);
}

void Market::EnforcePriceCeiling()
{
    if (m_price_ceiling > 0.0 && m_current_price > m_price_ceiling)
        m_current_price = m_price_ceiling;
}

void Market::EnforcePriceFloor()
{
    if (m_price_floor > 0.0 && m_current_price < m_price_floor)
        m_current_price = m_price_floor;
}

double Market::CalculatePriceElasticity(double price_change_percent,
                                        double quantity_change_percent)
{
    if (price_change_percent == 0)
        return 0.0;

    return quantity_change_percent / price_change_percent;
}

void Market::ApplyElasticity(double elasticity_coefficient)
{
    // Higher elasticity => more responsive demand
    m_quantity_demanded *= (1.0 + (elasticity_coefficient - 1.0) * 0.05);
}

void Market::AddToGDP(double transaction_value)
{
    m_total_sales_value += transaction_value;
}

double Market::CalculateInflation()
{
    if (m_last_price <= 0)
        return 0.0;
    return (m_current_price - m_last_price) / m_last_price;
}

void Market::RecordPrice(double price)
{
    m_last_price = price;
    m_price_history.push_back(price);
}

std::string Market::GetInfoString() const
{
    std::ostringstream ss;
    ss << "Market: " << m_product_name << "\n";
    ss << "Price: " << m_current_price << ", Equilibrium: " << m_equilibrium_price << "\n";
    ss << "Demand: " << m_quantity_demanded << ", Supply: " << m_quantity_supplied << "\n";
    ss << "Shortage/Surplus: " << m_shortage_surplus << "\n";
    ss << "Tax: " << m_tax_rate << ", Subsidy: " << m_subsidy_rate << "\n";
    return ss.str();
}

std::string Market::GetEquilibriumAnalysis() const
{
    std::ostringstream ss;
    ss << "Equilibrium Analysis for " << m_product_name << "\n";
    ss << "Qd: " << m_quantity_demanded << ", Qs: " << m_quantity_supplied << "\n";
    ss << "Price: " << m_current_price << ", Eq Price: " << m_equilibrium_price << "\n";
    return ss.str();
}
