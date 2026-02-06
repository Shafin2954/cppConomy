#pragma once

#include <string>
#include <vector>
#include <map>

// ============================================================================
// Government.h - The policy maker controlling fiscal and monetary policy
//
// Maps to: Fiscal Policy (Taxes, Spending), Monetary Policy (Money Supply, Interest),
//          Minimum Wage effects, Subsidies, PPF (resource allocation),
//          Government Budget, Deficit/Debt
// ============================================================================

class Government
{
private:
    // ========== Fiscal Policy Tools ==========
    double m_income_tax_rate;       // Tax on worker income: affects disposable income
    double m_corporate_tax_rate;    // Tax on business profits: affects investment
    double m_government_spending;   // G in GDP equation: injects demand into economy
    double m_tax_revenue_collected; // Total taxes from this period
    double m_government_debt;       // Accumulated deficits
    double m_budget_deficit;        // Spending - Tax Revenue

    // ========== Monetary Policy Tools ==========
    double m_money_supply;  // M in economy: affects inflation (MV = PQ)
    double m_interest_rate; // Central bank rate: affects investment decisions

    // ========== Regulations & Safety Nets ==========
    double m_minimum_wage; // Wage floor: if >equilibrium, causes unemployment

    // Subsidies: support specific sectors
    std::map<std::string, double> m_subsidies; // sector -> subsidy amount

    // ========== Macroeconomic Aggregates ==========
    double m_nominal_gdp;       // Sum of all production values
    double m_real_gdp;          // Nominal GDP adjusted for inflation
    double m_inflation_rate;    // Percentage change in price level
    double m_unemployment_rate; // Percentage of labor force without work
    double m_cpi;               // Consumer Price Index (tracking inflation)

    // ========== Demographic & Social ==========
    int m_total_population;
    int m_literacy_rate;     // Affects productivity and earnings potential
    double m_mortality_rate; // Natural deaths per period

public:
    // ========== Constructors ==========
    Government();

    // ========== Getters ==========
    double GetIncomeTaxRate() const { return m_income_tax_rate; }
    double GetCorporateTaxRate() const { return m_corporate_tax_rate; }
    double GetGovernmentSpending() const { return m_government_spending; }
    double GetMoneySupply() const { return m_money_supply; }
    double GetInterestRate() const { return m_interest_rate; }
    double GetMinimumWage() const { return m_minimum_wage; }
    double GetNominalGDP() const { return m_nominal_gdp; }
    double GetRealGDP() const { return m_real_gdp; }
    double GetInflationRate() const { return m_inflation_rate; }
    double GetUnemploymentRate() const { return m_unemployment_rate; }
    double GetCPI() const { return m_cpi; }
    double GetGovDebt() const { return m_government_debt; }
    double GetBudgetDeficit() const { return m_budget_deficit; }

    // ========== Fiscal Policy (Demand-side) ==========

    // Income tax: reduces worker disposable income
    // Shows: How taxes affect consumption and saving
    void SetIncomeTaxRate(double rate);

    // Corporate tax: reduces business profits and investment
    // Shows: Taxes reduce productive capacity over time
    void SetCorporateTaxRate(double rate);

    // Government spending: injects money into economy
    // G is component of GDP: GDP = C + I + G + (X - M)
    // Shows: Government can stimulate economy through spending
    void SetGovernmentSpending(double amount);

    // Collect taxes from population
    // Called each period to accumulate revenue
    void CollectTaxes(double total_income, double total_profits);

    // ========== Monetary Policy (Money supply) ==========

    // Quantity Theory: MV = PQ (Money Supply * Velocity = Price Level * Output)
    // Increasing money supply causes inflation if output constant
    void SetMoneySupply(double amount);

    // Interest rate: affects investment and saving decisions
    // Higher interest: discourages borrowing, reduces investment
    // Lower interest: encourages borrowing, increases investment
    void SetInterestRate(double rate);

    // ========== Labor Market Policy ==========

    // Set minimum wage: if above equilibrium, causes unemployment
    // Shows: How price floors (minimum wage) create surplus labor (unemployment)
    // Demonstrates labor market effects of policy
    void SetMinimumWage(double wage);

    // ========== Subsidies & Support ==========

    // Grant subsidy to sector: reduces their costs
    // Shifts their supply curve right -> more supply -> lower price
    // Example: agricultural subsidy to farmers
    void GrantSubsidy(const std::string &sector, double amount);

    // ========== GDP & Price Level Calculation ==========

    // Calculate nominal GDP: sum of all production values at current prices
    void CalculateNominalGDP(double total_production_value);

    // Calculate real GDP: nominal GDP adjusted for inflation
    // Real GDP = Nominal GDP / (CPI / 100)
    // Shows: True economic growth vs inflation illusion
    void CalculateRealGDP(double cpi_index);

    // Calculate inflation: percentage change in price level
    // Shows: Effects of monetary policy and supply shocks
    void CalculateInflation(double previous_cpi, double current_cpi);

    // Update CPI: Consumer Price Index tracking inflation
    void UpdateCPI(double new_cpi);

    // ========== Employment Tracking ==========

    // Track unemployment: from labor market imbalances
    void UpdateUnemploymentRate(int unemployed, int total_labor_force);

    // ========== Demographic Tracking ==========

    void SetPopulation(int population) { m_total_population = population; }
    void SetLiteracyRate(int rate) { m_literacy_rate = rate; }
    void SetMortalityRate(double rate) { m_mortality_rate = rate; }

    // ========== Budget Constraint ==========

    // Calculate budget: Revenue from taxes vs Spending + Debt service
    // Deficit: spending > revenue (financed by debt)
    // Surplus: revenue > spending (pay down debt)
    void UpdateBudget();

    // ========== PPF (Production Possibility Frontier) ==========

    // Calculate the PPF: shows tradeoff between producing Rice vs Ice Cream
    // If government moves resources from rice to ice cream production:
    // rice output falls, ice cream output rises
    // This demonstrates: opportunity cost, resource constraints
    std::vector<double> CalculatePPF(double total_labor, double total_capital);

    // ========== Display ==========
    std::string GetInfoString() const;
};
