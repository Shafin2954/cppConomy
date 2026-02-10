#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cmath>
#include <algorithm>

// ============================================================================
// EconomicEquations.h - Core system for economic relationships and formulas
//
// This file defines the mathematical relationships between economic variables.
// When one variable changes, we can automatically track and calculate impacts
// on related variables using these equations.
// ============================================================================

class EconomicEquations
{
public:
    // ========== Variable Change Log Entry ==========
    struct VariableChange
    {
        std::string variableName;
        double oldValue;
        double newValue;
        double percentChange;
        std::string description;
        std::vector<std::string> affectedVariables; // What this change impacts

        std::string ToString() const;
    };

    // ========== Market Equilibrium & Price Discovery ==========
    // Demand Curve: Qd = a - b*P (downward sloping)
    // Supply Curve: Qs = c + d*P (upward sloping)
    // Equilibrium: Qd = Qs

    struct DemandFunction
    {
        double intercept_a; // Quantity demanded at P=0
        double slope_b;     // How much quantity falls per $1 price increase

        double Calculate(double price) const
        {
            return intercept_a - slope_b * price;
        }
    };

    struct SupplyFunction
    {
        double intercept_c; // Quantity supplied at P=0
        double slope_d;     // How much quantity rises per $1 price increase

        double Calculate(double price) const
        {
            return intercept_c + slope_d * price;
        }
    };

    // ========== Equilibrium Solver ==========
    // Solves: Qd = Qs
    // Returns: std::pair<equilibriuprice, equilibriuquantity>
    static std::pair<double, double> FindEquilibrium(
        const DemandFunction &demand,
        const SupplyFunction &supply)
    {
        // Qd = Qs
        // a - b*P = c + d*P
        // a - c = d*P + b*P
        // a - c = (d + b)*P
        // P = (a - c) / (b + d)

        double eq_price = (demand.intercept_a - supply.intercept_c) /
                          (supply.slope_d + demand.slope_b);
        double eq_quantity = demand.Calculate(eq_price);

        return {eq_price, eq_quantity};
    }

    // ========== Price Elasticity of Demand ==========
    // PED = (% Change in Quantity Demanded) / (% Change in Price)
    // |PED| > 1: Elastic (quantity very sensitive to price)
    // |PED| < 1: Inelastic (quantity not very sensitive to price)

    static double CalculatePriceElasticity(
        double initialQuantity,
        double finalQuantity,
        double initialPrice,
        double finalPrice)
    {
        double quantityChange = finalQuantity - initialQuantity;
        double priceChange = finalPrice - initialPrice;

        double avgQuantity = (initialQuantity + finalQuantity) / 2.0;
        double avgPrice = (initialPrice + finalPrice) / 2.0;

        if (avgPrice == 0)
            return 0;

        // Mid-point method
        double percentQuantityChange = quantityChange / avgQuantity;
        double percentPriceChange = priceChange / avgPrice;

        if (percentPriceChange == 0)
            return 0;

        return abs(percentQuantityChange / percentPriceChange);
    }

    // ========== Income Elasticity of Demand ==========
    // YED = (% Change in Quantity Demanded) / (% Change in Income)
    // YED > 1: Normal good (luxury), demand increases with income
    // 0 < YED < 1: Normal good (necessity), demand increases slowly with income
    // YED < 0: Inferior good, demand decreases as income rises

    static double CalculateIncomeElasticity(
        double initialQuantity,
        double finalQuantity,
        double initialIncome,
        double finalIncome)
    {
        double quantityChange = finalQuantity - initialQuantity;
        double incomeChange = finalIncome - initialIncome;

        double avgQuantity = (initialQuantity + finalQuantity) / 2.0;
        double avgIncome = (initialIncome + finalIncome) / 2.0;

        if (avgIncome == 0)
            return 0;

        double percentQuantityChange = quantityChange / avgQuantity;
        double percentIncomeChange = incomeChange / avgIncome;

        if (percentIncomeChange == 0)
            return 0;

        return percentQuantityChange / percentIncomeChange;
    }

    // ========== Cross-Price Elasticity (Substitutes/Complements) ==========
    // XED = (% Change in Qty of Good A) / (% Change in Price of Good B)
    // XED > 0: Substitutes (when B gets expensive, demand for A increases)
    // XED < 0: Complements (when B gets expensive, demand for A decreases)

    static double CalculateCrossPriceElasticity(
        double initialQtyA,
        double finalQtyA,
        double initialPriceB,
        double finalPriceB)
    {
        double qtyChangeA = finalQtyA - initialQtyA;
        double priceChangeB = finalPriceB - initialPriceB;

        double avgQtyA = (initialQtyA + finalQtyA) / 2.0;
        double avgPriceB = (initialPriceB + finalPriceB) / 2.0;

        if (avgPriceB == 0)
            return 0;

        double percentQtyChangeA = qtyChangeA / avgQtyA;
        double percentPriceChangeB = priceChangeB / avgPriceB;

        if (percentPriceChangeB == 0)
            return 0;

        return percentQtyChangeA / percentPriceChangeB;
    }

    // ========== Revenue from Price Changes ==========
    // Total Revenue = Price × Quantity
    // When elastic (PED > 1): Price decrease increases TR (lower P, much higher Q)
    // When inelastic (PED < 1): Price increase increases TR (higher P, little lower Q)

    static double CalculateTotalRevenue(double price, double quantity)
    {
        return price * quantity;
    }

    static std::string GetRevenueRecommendation(double elasticity, double currentPrice)
    {
        if (elasticity > 1.0)
        {
            return "ELASTIC: Price is too high. Reduce price to increase Total Revenue.";
        }
        else if (elasticity < 1.0)
        {
            return "INELASTIC: Price is too low. Increase price to increase Total Revenue.";
        }
        else
        {
            return "UNIT ELASTIC: Price changes don't affect Total Revenue.";
        }
    }

    // ========== Consumer Surplus & Producer Surplus ==========
    // Consumer Surplus = Area under demand curve - Price paid
    // Represents: Difference between max price consumer willing to pay vs actual price

    static double CalculateConsumerSurplus(
        double demandIntercept,
        double equilibriumPrice,
        double equilibriumQuantity)
    {
        // Triangle area = 0.5 * base * height
        // Height = demandIntercept - equilibriumPrice
        double height = demandIntercept - equilibriumPrice;
        return 0.5 * height * equilibriumQuantity;
    }

    static double CalculateProducerSurplus(
        double supplyIntercept,
        double equilibriumPrice,
        double equilibriumQuantity)
    {
        double height = equilibriumPrice - supplyIntercept;
        return 0.5 * height * equilibriumQuantity;
    }

    // ========== Cost Curves ==========
    // Total Cost: TC = TFC + TVC
    // Average Total Cost: ATC = TC / Q
    // Marginal Cost: MC = ΔTC / ΔQ
    // Average Variable Cost: AVC = TVC / Q

    struct CostAnalysis
    {
        double totalFixedCost;
        double totalVariableCost;
        double totalCost;
        double averageTotalCost;
        double averageVariableCost;
        double marginalCost;
        double quantity;

        static CostAnalysis Calculate(
            double tfc,
            double tvc,
            double quantity)
        {
            CostAnalysis ca;
            ca.totalFixedCost = tfc;
            ca.totalVariableCost = tvc;
            ca.totalCost = tfc + tvc;
            ca.quantity = quantity;

            if (quantity > 0)
            {
                ca.averageTotalCost = ca.totalCost / quantity;
                ca.averageVariableCost = tvc / quantity;
            }

            return ca;
        }

        std::string ToString(double price) const;
    };

    // ========== Marginal Utility & Equi-Marginal Principle ==========
    // MU = Change in Total Utility / Change in Quantity
    // Consumer optimizes when: MU_A / Price_A = MU_B / Price_B
    // This means: last dollar spent on each good gives equal satisfaction

    static double CalculateMarginalUtility(
        double initialUtility,
        double finalUtility,
        double initialQuantity,
        double finalQuantity)
    {
        double utilityChange = finalUtility - initialUtility;
        double quantityChange = finalQuantity - initialQuantity;

        if (quantityChange == 0)
            return 0;

        return utilityChange / quantityChange;
    }

    // ========== Production Function ==========
    // Output = A × L^α × K^β × T^γ (Cobb-Douglas)
    // Simplified: Output = Technology × sqrt(Labor) × sqrt(Capital)
    // With diminishing marginal product of labor

    static double CalculateProductionOutput(
        double labor,
        double capital,
        double technologyLevel)
    {
        if (labor <= 0 || capital <= 0)
            return 0;

        // Cobb-Douglas style: Q = A * L^0.5 * K^0.5
        return technologyLevel * sqrt(labor) * sqrt(capital);
    }

    // Marginal Product of Labor: How much output increases with one more worker
    static double CalculateMarginalProductOfLabor(
        double currentLabor,
        double technologyLevel,
        double capital)
    {
        if (currentLabor <= 0)
            return 0;

        double currentOutput = CalculateProductionOutput(currentLabor, capital, technologyLevel);
        double previousOutput = CalculateProductionOutput(currentLabor - 1, capital, technologyLevel);

        return currentOutput - previousOutput;
    }

    // ========== Macroeconomic Aggregates ==========
    // GDP (Expenditure Approach): GDP = C + I + G + (X - M)
    // C: Consumer spending
    // I: Investment spending
    // G: Government spending
    // X: Exports
    // M: Imports

    static double CalculateGDP(
        double consumption,
        double investment,
        double governmentSpending,
        double exports,
        double imports)
    {
        return consumption + investment + governmentSpending + (exports - imports);
    }

    // ========== Inflation & CPI ==========
    // Inflation = (CPI_current - CPI_last) / CPI_last × 100%
    // CPI = (Cost of basket today / Cost of basket in base year) × 100

    static double CalculateInflation(double currentCPI, double previousCPI)
    {
        if (previousCPI == 0)
            return 0;
        return ((currentCPI - previousCPI) / previousCPI) * 100.0;
    }

    // ========== Money & Velocity ==========
    // Quantity Theory: M × V = P × Q
    // M: Money supply
    // V: Velocity of money (times money changes hands)
    // P: Price level
    // Q: Real quantity of output

    static double CalculatePriceLevel(
        double moneySupply,
        double velocity,
        double realOutput)
    {
        if (realOutput == 0)
            return 0;
        return (moneySupply * velocity) / realOutput;
    }

    // ========== Phillips Curve: Inflation vs Unemployment ==========
    // Inflation = Natural Rate - α × (Unemployment - Natural Rate)
    // Trade-off between inflation and unemployment in short run

    static double CalculateInflationFromPhillipsCurve(
        double naturalUnemploymentRate,
        double actualUnemploymentRate,
        double naturalInflationRate,
        double sensitivityAlpha)
    {
        double unemploymentGap = actualUnemploymentRate - naturalUnemploymentRate;
        return naturalInflationRate - sensitivityAlpha * unemploymentGap;
    }

    // ========== Gini Coefficient (Inequality Measure) ==========
    // 0 = Perfect equality
    // 1 = Perfect inequality
    // Formula: G = (2 * Σ(i * income_i)) / (n * Σ income_i) - (n+1)/n

    static double CalculateGiniCoefficient(const std::vector<double> &incomes)
    {
        if (incomes.empty())
            return 0;

        int n = incomes.size();
        double sumWeightedIncomes = 0;
        double totalIncome = 0;

        // Sort incomes (in practice, from lowest to highest)
        std::vector<double> sortedIncomes = incomes;
        sort(sortedIncomes.begin(), sortedIncomes.end());

        for (int i = 0; i < n; i++)
        {
            sumWeightedIncomes += (i + 1) * sortedIncomes[i];
            totalIncome += sortedIncomes[i];
        }

        if (totalIncome == 0)
            return 0;

        double gini = (2.0 * sumWeightedIncomes) / (n * totalIncome) - (n + 1.0) / n;
        return gini;
    }

    // ========== Labor Market Equilibrium ==========
    // Equilibrium wage where Labor Demanded = Labor Supplied
    // If Min Wage > Equilibrium Wage: Unemployment increases

    static double CalculateUnemploymentFromMinWage(
        double equilibriumWage,
        double minimumWage,
        double laborSupply)
    {
        // If min wage > equilibrium, unemployment rises
        if (minimumWage <= equilibriumWage)
            return 0;

        // Simplified: unemployment = (MinWage - EqWage) / EqWage × some % of labor supply
        double wageGap = (minimumWage - equilibriumWage) / equilibriumWage;
        return wageGap * laborSupply;
    }

    // ========== PPF (Production Possibility Frontier) ==========
    // Trade-off: To produce more of X, must produce less of Y
    // PPF: X + Y = Total Resources × Productivity
    // Productive Efficiency: On the frontier (not inside)

    struct PPFPoint
    {
        double productionX;
        double productionY;

        // Check if point is on the frontier
        static bool IsOnFrontier(double x, double y, double maxX, double maxY)
        {
            // Linear PPF: x/maxX + y/maxY = 1
            double pointOnFrontier = x / maxX + y / maxY;
            return abs(pointOnFrontier - 1.0) < 0.01; // Allow small tolerance
        }

        // Opportunity cost: How much Y must be given up to get one more X
        static double GetOpportunityCostX(double maxX, double maxY)
        {
            return maxY / maxX; // units of Y per unit of X
        }
    };
};
