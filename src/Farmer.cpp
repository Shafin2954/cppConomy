#include "Farmer.h"
#include <sstream>
#include <algorithm>
#include <cmath>

Farmer::Farmer(int id, const std::string &name, double initial_income,
               double land, const std::string &crop)
    : Person(id, name, initial_income),
      m_land_size(land),
      m_labor_hired(0),
      m_technology_level(1.0),
      m_fertilizer_units(0.0),
      m_weather_factor(1.0),
      m_crop(crop),
      m_output_quantity(0.0),
      m_output_price(0.0),
      m_revenue(0.0),
      m_fixed_cost(100.0),
      m_variable_cost(0.0),
      m_total_cost(0.0),
      m_average_cost(0.0),
      m_marginal_cost(0.0),
      m_profit(0.0)
{
}

void Farmer::Hire(int workers)
{
    if (workers <= 0)
        return;
    m_labor_hired += workers;
}

void Farmer::Fire(int workers)
{
    if (workers <= 0)
        return;
    m_labor_hired = std::max(0, m_labor_hired - workers);
}

void Farmer::Plant(const std::string &new_crop)
{
    m_crop = new_crop;
}

void Farmer::Harvest()
{
    m_output_quantity = CalculateProduction();
}

void Farmer::AddFertilizer(double units)
{
    if (units <= 0)
        return;
    m_fertilizer_units += units;
}

void Farmer::UpgradeTechnology()
{
    m_technology_level += 0.1;
}

void Farmer::CalculateCosts(double wage_rate, double fertilizer_price)
{
    m_variable_cost = (m_labor_hired * wage_rate) + (m_fertilizer_units * fertilizer_price);
    m_total_cost = m_fixed_cost + m_variable_cost;
    if (m_output_quantity > 0)
    {
        m_average_cost = m_total_cost / m_output_quantity;
    }
    else
    {
        m_average_cost = 0.0;
    }

    // Simplified marginal cost: variable cost per unit
    if (m_output_quantity > 0)
    {
        m_marginal_cost = m_variable_cost / m_output_quantity;
    }
    else
    {
        m_marginal_cost = 0.0;
    }
}

double Farmer::CalculateProduction()
{
    // Production function with diminishing returns
    // Output = land * tech * weather * sqrt(labor + fertilizer)
    double input = std::sqrt(static_cast<double>(m_labor_hired) + m_fertilizer_units + 1.0);
    return m_land_size * m_technology_level * m_weather_factor * input;
}

void Farmer::CalculateProfit(double output_price)
{
    m_output_price = output_price;
    m_revenue = m_output_quantity * output_price;
    m_profit = m_revenue - m_total_cost;
}

double Farmer::GetMarginalProductOfLabor() const
{
    // Approximate MP of labor by derivative of sqrt(x)
    if (m_labor_hired <= 0)
        return 0.0;
    return (m_land_size * m_technology_level * m_weather_factor) / (2.0 * std::sqrt(m_labor_hired));
}

double Farmer::GetMarginalProductOfFertilizer() const
{
    if (m_fertilizer_units <= 0)
        return 0.0;
    return (m_land_size * m_technology_level * m_weather_factor) / (2.0 * std::sqrt(m_fertilizer_units));
}

std::string Farmer::GetInfoString() const
{
    std::ostringstream ss;
    ss << "Farmer: " << m_name << " (ID=" << m_id << ")\n";
    ss << "Crop: " << m_crop << ", Land: " << m_land_size << " acres\n";
    ss << "Labor: " << m_labor_hired << ", Fertilizer: " << m_fertilizer_units << "\n";
    ss << "Output: " << m_output_quantity << ", Price: " << m_output_price << "\n";
    ss << "Revenue: " << m_revenue << ", Profit: " << m_profit << "\n";
    ss << "Costs - Fixed: " << m_fixed_cost << ", Variable: " << m_variable_cost
       << ", Total: " << m_total_cost << "\n";
    return ss.str();
}
