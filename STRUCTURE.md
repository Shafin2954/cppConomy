# CppConomy v2 - Class Structure Setup Complete

## Summary of Created Headers

### 1. **Product.h** 
Enum and struct for goods being traded
- ProductType: RICE, ICE_CREAM, CURD, FERTILIZER, LABOR, CLOTH
- Product struct: price, quantity, elasticity, addiction, substitutes
- Maps to: Substitution, Elasticity, Addiction, CPI

### 2. **Person.h** (Base Class)
Economic agent with budget constraint and utility
- Income, Savings, Spending, Budget line
- Total Utility, Addiction level
- Preferences (indifference curves)
- Methods: Consume, UpdateUtility, SubstituteGood, PayTax, AgeTick
- Maps to: Utility, Budget Constraint, Marginal Utility, Consumer Surplus, Taxes

### 3. **Worker.h** (Inherits Person)
Labor market participant
- Skill level, Minimum acceptable wage
- Employment status, Current wage, Employer
- Hours willing to work
- Methods: OfferLabor, AcceptJob, LoseJob, RespondToMinimumWage, UpdateLaborSupply
- Maps to: Employment, Unemployment, Minimum Wage, Labor Supply Curve, Income/Substitution Effect

### 4. **Farmer.h** (Inherits Person)
Agricultural producer
- Land size, Crop type, Yield
- Labor hired, Fertilizer units, Technology level
- Weather factor (for bumper harvest)
- Fixed/Variable/Total/Average/Marginal Cost
- Output quantity, Revenue, Profit
- Methods: Hire, Fire, Plant, Harvest, AddFertilizer, UpgradeTechnology, CalculateCosts, CalculateProduction
- Maps to: Production Function, Diminishing Returns, PPF, Bumper Harvest, Cost Curves, Technology, Marginal Product

### 5. **Owner.h** (Inherits Person)
Business owner/manufacturer
- Capital machines, Labor hired, Technology level
- Product type, Production output
- Monopoly status, Price setting
- Cost structure (TFC, TVC, AC, MC) and Revenue analysis
- Marginal Revenue, Profit, Shutdown decision
- Methods: Hire, Fire, Produce, CalculateCosts, CalculateRevenue, CalculateProfit, GetOptimalQuantity, CheckShutdownCondition, ExpandFactory, SetMonopolyPrice
- Maps to: Cost Curves, Profit Maximization (MR=MC), Monopoly, Shutdown Condition, Returns to Scale, Production Function

### 6. **Market.h**
Place where transactions occur and prices determined
- Quantity demanded, Quantity supplied
- Current price, Equilibrium price
- Shortage/Surplus
- Tax rate, Subsidy rate, Price ceiling, Price floor
- Total sales value, Price history
- Methods: FindEquilibrium, AdjustPrice, ApplyTaxAndSubsidy, EnforcePriceCeiling, EnforcePriceFloor, CalculatePriceElasticity, ApplyElasticity, AddToGDP, CalculateInflation
- Maps to: Supply/Demand, Equilibrium, Elasticity, Price Controls, Tax Effects, GDP, Inflation, Consumer Surplus

### 7. **Government.h**
Policy maker controlling fiscal and monetary policy
- Income tax rate, Corporate tax rate, Government spending
- Money supply, Interest rate
- Minimum wage, Subsidies
- Nominal/Real GDP, Inflation rate, Unemployment rate, CPI
- Methods: SetIncomeTaxRate, SetCorporateTaxRate, SetGovernmentSpending, CollectTaxes, SetMoneySupply, SetInterestRate, SetMinimumWage, GrantSubsidy, CalculateNominalGDP, CalculateRealGDP, CalculateInflation, UpdateUnemploymentRate, UpdateBudget, CalculatePPF
- Maps to: Fiscal Policy, Monetary Policy, Minimum Wage Effects, Subsidies, PPF, GDP, Inflation, Unemployment, CPI

### 8. **Simulation.h**
Central controller managing entire simulation
- Vector of Workers, Farmers, Owners
- Map of Markets
- Government instance
- Selection system for UI
- Methods: Initialize, Tick, Reset, AddWorker/Farmer/Owner, FindWorker/Farmer/Owner, CreateMarket, SelectWorker/Farmer/Owner/Market, CalculateTotalProduction, CalculateUnemploymentRate
- Coordinates all agents and markets

---

## Economic Concepts Mapped to Variables/Methods

| Concept | Class | Variable/Method |
|---------|-------|-----------------|
| **Budget Constraint** | Person | m_wallet, m_monthly_income |
| **Indifference Curve** | Person | m_preferences (map of product preferences) |
| **Marginal Utility** | Person | CalculateMarginalUtility() |
| **Utility Maximization** | Person | Consume() with equimarginal principle |
| **Income Effect** | Worker | UpdateLaborSupply(), Consume() with income change |
| **Substitution Effect** | Person | SubstituteGood(), m_preferences |
| **Consumer Surplus** | Person, Market | Wallet vs Price vs Utility |
| **Addiction** | Product, Person | is_addictive flag, m_addiction_level |
| **Production Function** | Farmer, Owner | CalculateProduction(), Hire(), AddFertilizer() |
| **Diminishing Returns** | Farmer | GetMarginalProductOfLabor() decreases with more workers |
| **PPF** | Government | CalculatePPF() showing resource tradeoff |
| **Cost Curves** | Farmer, Owner | m_fixed_cost, m_variable_cost, GetAverageCost(), GetMarginalCost() |
| **Shutdown Condition** | Owner | CheckShutdownCondition(): if Price < AVC, stop producing |
| **Profit Maximization** | Owner | GetOptimalQuantity() where MR = MC |
| **Monopoly** | Owner | m_is_monopoly, SetMonopolyPrice() |
| **Returns to Scale** | Owner | ExpandFactory(): double inputs, check if output > doubles |
| **Supply & Demand** | Market | m_quantity_supplied, m_quantity_demanded |
| **Equilibrium** | Market | FindEquilibrium(), m_equilibrium_price |
| **Elasticity** | Product, Market | is_elastic, CalculatePriceElasticity(), ApplyElasticity() |
| **Shortage/Surplus** | Market | m_shortage_surplus = Qd - Qs |
| **Price Ceiling** | Market | m_price_ceiling, EnforcePriceCeiling() |
| **Price Floor** | Market | m_price_floor, EnforcePriceFloor() |
| **Minimum Wage** | Government, Worker | m_minimum_wage, RespondToMinimumWage() causes unemployment |
| **Tax Effects** | Government, Market | SetIncomeTaxRate(), SetCorporateTaxRate(), m_tax_rate |
| **Subsidy** | Government, Market | GrantSubsidy(), m_subsidy_rate |
| **GDP (Nominal)** | Government, Market | CalculateNominalGDP(), AddToGDP() |
| **GDP (Real)** | Government | CalculateRealGDP() adjusted for inflation |
| **Inflation** | Government, Market | CalculateInflation(), m_price_history |
| **CPI** | Government, Market | m_cpi, UpdateCPI() |
| **Monetary Policy** | Government | SetMoneySupply(), SetInterestRate() |
| **Fiscal Policy** | Government | SetTaxRate(), SetGovernmentSpending() |
| **Unemployment** | Government, Worker | CalculateUnemploymentRate(), m_is_employed |
| **Labor Supply Curve** | Worker | UpdateLaborSupply(): hours work responds to wage |
| **Bumper Harvest** | Farmer | m_weather_factor = 1.5, Harvest() increases output dramatically |
| **Technology** | Farmer, Owner | m_technology_level, UpgradeTechnology(): shifts production function |
| **Mortality** | Person | m_age, m_is_alive, AgeTick() |
| **Literacy** | Person | m_literacy_score affects earning potential |
| **Child Labor** | Worker | Age affects min_acceptable_wage |

---

## Next Steps

1. **Implement .cpp files** for each class (start with Person, then Worker, Farmer, Owner, Market, Government, Simulation)
2. **Create Simulation.cpp** with all the initialization and coordination logic
3. **Update Commands** - rewrite CommandParser and CommandExecutor for new class structure
4. **Update GUI** - create new panels for 4-column display and command list
5. **Test basic operations** - create agents, run ticks, verify outputs

---

## File Status

✅ Created: Product.h, Person.h, Worker.h, Farmer.h, Owner.h, Market.h, Government.h, Simulation.h
⏳ Pending: All .cpp implementation files
⏳ Pending: Update main.cpp, CommandParser.cpp, CommandExecutor.cpp, MainFrame.cpp
❌ Deleted: All old complex classes (Agent, Firm, Economy, and models)

**CMakeLists.txt updated** to only include new header structure.
