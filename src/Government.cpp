#include "Government.h"
#include <sstream>
#include <algorithm>

Government::Government()
    : m_income_tax_rate(0.1),
      m_corporate_tax_rate(0.15),
      m_government_spending(1000.0),
      m_tax_revenue_collected(0.0),
      m_government_debt(0.0),
      m_budget_deficit(0.0),
      m_money_supply(100000.0),
      m_interest_rate(0.03),
      m_minimum_wage(10.0),
      m_nominal_gdp(0.0),
      m_real_gdp(0.0),
      m_inflation_rate(0.0),
      m_unemployment_rate(0.0),
      m_cpi(100.0),
      m_total_population(0),
      m_literacy_rate(80),
      m_mortality_rate(0.01)
{
}

void Government::SetIncomeTaxRate(double rate)
{
    m_income_tax_rate = std::clamp(rate, 0.0, 1.0);
}

void Government::SetCorporateTaxRate(double rate)
{
    m_corporate_tax_rate = std::clamp(rate, 0.0, 1.0);
}

void Government::SetGovernmentSpending(double amount)
{
    m_government_spending = std::max(0.0, amount);
}

void Government::CollectTaxes(double total_income, double total_profits)
{
    double income_tax = total_income * m_income_tax_rate;
    double corp_tax = total_profits * m_corporate_tax_rate;
    m_tax_revenue_collected = income_tax + corp_tax;
}

void Government::SetMoneySupply(double amount)
{
    m_money_supply = std::max(0.0, amount);
}

void Government::SetInterestRate(double rate)
{
    m_interest_rate = std::max(0.0, rate);
}

void Government::SetMinimumWage(double wage)
{
    m_minimum_wage = std::max(0.0, wage);
}

void Government::GrantSubsidy(const std::string &sector, double amount)
{
    m_subsidies[sector] = amount;
}

void Government::CalculateNominalGDP(double total_production_value)
{
    m_nominal_gdp = total_production_value;
}

void Government::CalculateRealGDP(double cpi_index)
{
    if (cpi_index <= 0)
        m_real_gdp = m_nominal_gdp;
    else
        m_real_gdp = m_nominal_gdp / (cpi_index / 100.0);
}

void Government::CalculateInflation(double previous_cpi, double current_cpi)
{
    if (previous_cpi <= 0)
        m_inflation_rate = 0.0;
    else
        m_inflation_rate = (current_cpi - previous_cpi) / previous_cpi;
}

void Government::UpdateCPI(double new_cpi)
{
    m_cpi = new_cpi;
}

void Government::UpdateUnemploymentRate(int unemployed, int total_labor_force)
{
    if (total_labor_force <= 0)
        m_unemployment_rate = 0.0;
    else
        m_unemployment_rate = static_cast<double>(unemployed) / total_labor_force;
}

void Government::UpdateBudget()
{
    m_budget_deficit = m_government_spending - m_tax_revenue_collected;
    if (m_budget_deficit > 0)
        m_government_debt += m_budget_deficit;
}

std::vector<double> Government::CalculatePPF(double total_labor, double total_capital)
{
    // Simplified PPF: linear tradeoff for now
    // Output = a * labor + b * capital
    std::vector<double> curve;
    int points = 10;
    for (int i = 0; i <= points; ++i)
    {
        double share = static_cast<double>(i) / points;
        double rice = total_labor * (1.0 - share);
        double ice_cream = total_capital * share;
        curve.push_back(rice);
        curve.push_back(ice_cream);
    }
    return curve;
}

std::string Government::GetInfoString() const
{
    std::ostringstream ss;
    ss << "Government Policies\n";
    ss << "Income Tax: " << m_income_tax_rate << ", Corporate Tax: " << m_corporate_tax_rate << "\n";
    ss << "Spending: " << m_government_spending << ", Debt: " << m_government_debt << "\n";
    ss << "Money Supply: " << m_money_supply << ", Interest Rate: " << m_interest_rate << "\n";
    ss << "Minimum Wage: " << m_minimum_wage << "\n";
    ss << "GDP: " << m_nominal_gdp << ", Inflation: " << m_inflation_rate << "\n";
    return ss.str();
}
