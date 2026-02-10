#include "Government.h"
#include <sstream>
#include <algorithm>

using namespace std;

Government::Government()
    : income_tax_rate(0.1),
      corporate_tax_rate(0.15),
      government_spending(1000.0),
      tax_revenue_collected(0.0),
      government_debt(0.0),
      budget_deficit(0.0),
      money_supply(100000.0),
      interest_rate(0.03),
      minimuwage(10.0),
      nominal_gdp(0.0),
      real_gdp(0.0),
      inflation_rate(0.0),
      unemployment_rate(0.0),
      cpi(100.0),
      total_population(0),
      literacy_rate(80),
      mortality_rate(0.01)
{
}

void Government::SetIncomeTaxRate(double rate)
{
    income_tax_rate = clamp(rate, 0.0, 1.0);
}

void Government::SetCorporateTaxRate(double rate)
{
    corporate_tax_rate = clamp(rate, 0.0, 1.0);
}

void Government::SetGovernmentSpending(double amount)
{
    government_spending = max(0.0, amount);
}

void Government::CollectTaxes(double total_income, double total_profits)
{
    double income_tax = total_income * income_tax_rate;
    double corp_tax = total_profits * corporate_tax_rate;
    tax_revenue_collected = income_tax + corp_tax;
}

void Government::SetMoneySupply(double amount)
{
    money_supply = max(0.0, amount);
}

void Government::SetInterestRate(double rate)
{
    interest_rate = max(0.0, rate);
}

void Government::SetMinimumWage(double wage)
{
    minimuwage = max(0.0, wage);
}

void Government::GrantSubsidy(const string &sector, double amount)
{
    subsidies[sector] = amount;
}

void Government::CalculateNominalGDP(double total_production_value)
{
    nominal_gdp = total_production_value;
}

void Government::CalculateRealGDP(double cpi_index)
{
    if (cpi_index <= 0)
        real_gdp = nominal_gdp;
    else
        real_gdp = nominal_gdp / (cpi_index / 100.0);
}

void Government::CalculateInflation(double previous_cpi, double current_cpi)
{
    if (previous_cpi <= 0)
        inflation_rate = 0.0;
    else
        inflation_rate = (current_cpi - previous_cpi) / previous_cpi;
}

void Government::UpdateCPI(double new_cpi)
{
    cpi = new_cpi;
}

void Government::UpdateUnemploymentRate(int unemployed, int total_labor_force)
{
    if (total_labor_force <= 0)
        unemployment_rate = 0.0;
    else
        unemployment_rate = static_cast<double>(unemployed) / total_labor_force;
}

void Government::UpdateBudget()
{
    budget_deficit = government_spending - tax_revenue_collected;
    if (budget_deficit > 0)
        government_debt += budget_deficit;
}

vector<double> Government::CalculatePPF(double total_labor, double total_capital)
{
    // Simplified PPF: linear tradeoff for now
    // Output = a * labor + b * capital
    vector<double> curve;
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

string Government::GetInfoString() const
{
    ostringstream ss;
    ss << "Government Policies\n";
    ss << "Income Tax: " << income_tax_rate << ", Corporate Tax: " << corporate_tax_rate << "\n";
    ss << "Spending: " << government_spending << ", Debt: " << government_debt << "\n";
    ss << "Money Supply: " << money_supply << ", Interest Rate: " << interest_rate << "\n";
    ss << "Minimum Wage: " << minimuwage << "\n";
    ss << "GDP: " << nominal_gdp << ", Inflation: " << inflation_rate << "\n";
    return ss.str();
}
