#pragma once

#include "Person.h"

// ============================================================================
// Owner.h - Inherits from Person (The Business Owner/Manufacturer)
//
// Maps to: Production Function, Cost Curves, Monopoly, Shutdown Condition,
//          Marginal Cost, Marginal Revenue, Profit Maximization,
//          Returns to Scale, Market Power
// ============================================================================

class Owner : public Person
{
private:
    // ========== Production Inputs (Factors of Production) ==========
    double m_capital_machines; // Capital stock (machinery, equipment)
    int m_labor_hired;         // Number of workers
    double m_technology_level; // Affects productivity: higher = more output per input

    std::string m_product_type; // What they produce: "cloth", "ice_cream", etc.
    double m_production_output; // Current production quantity

    // ========== Market Power ==========
    bool m_is_monopoly;     // If true: sets own price (price maker)
                            // If false: takes market price (price taker)
    double m_current_price; // Price they charge (or market price if competitive)
    double m_revenue;       // Price * Quantity

    // ========== Cost Structure (Cost Curves) ==========
    // Shows: Total Cost, Average Cost, Marginal Cost curves
    double m_total_fixed_cost;      // TFC: machinery rent, overhead (doesn't change with output)
    double m_total_variable_cost;   // TVC: wages, materials (changes with output)
    double m_total_cost;            // TC = TFC + TVC
    double m_average_fixed_cost;    // AFC = TFC / Q
    double m_average_variable_cost; // AVC = TVC / Q
    double m_average_cost;          // AC = TC / Q
    double m_marginal_cost;         // MC = change in TC / change in Q

    // ========== Revenue Analysis ==========
    double m_marginal_revenue; // MR = change in TR / change in Q

    // Profit
    double m_profit; // TR - TC (or can be negative = loss)

    // ========== Shutdown Decision ==========
    // Short run: produce if Price >= AVC (covers variable costs)
    // Long run: produce if Price >= AC (covers all costs)
    bool m_is_operating; // False if price too low -> shutdown

public:
    // ========== Constructors ==========
    Owner(int id, const std::string &name, double initial_capital,
          const std::string &product_type, bool is_monopoly);

    // ========== Getters ==========
    double GetCapital() const { return m_capital_machines; }
    int GetLaborHired() const { return m_labor_hired; }
    double GetTechnologyLevel() const { return m_technology_level; }
    std::string GetProductType() const { return m_product_type; }
    double GetProduction() const { return m_production_output; }
    bool IsMonopoly() const { return m_is_monopoly; }
    double GetPrice() const { return m_current_price; }
    double GetRevenue() const { return m_revenue; }
    double GetTotalFixedCost() const { return m_total_fixed_cost; }
    double GetTotalVariableCost() const { return m_total_variable_cost; }
    double GetTotalCost() const { return m_total_cost; }
    double GetAverageCost() const { return m_average_cost; }
    double GetAverageVariableCost() const { return m_average_variable_cost; }
    double GetMarginalCost() const { return m_marginal_cost; }
    double GetMarginalRevenue() const { return m_marginal_revenue; }
    double GetProfit() const { return m_profit; }
    bool IsOperating() const { return m_is_operating; }

    // ========== Setters ==========
    void SetPrice(double price) { m_current_price = price; }

    // ========== Production Decisions ==========

    // Hire worker: increases variable cost and production
    void Hire();

    // Fire worker: decreases variable cost and production
    void Fire();

    // Produce: executes production function
    // Output = f(Capital, Labor, Technology)
    // Simplified: Output = (labor_hired * technology_level) * (capital_machines^0.5)
    void Produce();

    // ========== Cost & Revenue Calculations ==========

    // Calculate total costs
    void CalculateCosts(double wage_rate, double material_cost_per_unit);

    // Calculate revenue
    void CalculateRevenue();

    // Calculate profit
    void CalculateProfit();

    // ========== Profit Maximization (Core Decision Rule) ==========

    // Produce where Marginal Revenue = Marginal Cost (MR = MC rule)
    // If monopoly: can set price and quantity
    // If competitive: must take market price, choose quantity where P = MC
    int GetOptimalQuantity(double market_price);

    // ========== Shutdown Condition ==========

    // Short run shutdown: if Price < AVC, produce 0
    // Long run shutdown: if Price < AC, exit market
    // This demonstrates: why farmers continue through losses (fixed costs already paid)
    void CheckShutdownCondition(double market_price);

    // ========== Returns to Scale ==========

    // Expand factory: double capital and labor
    // If output more than doubles -> increasing returns to scale
    // If output doubles -> constant returns to scale
    // If output less than doubles -> decreasing returns to scale
    void ExpandFactory();

    // ========== Market Power (Monopoly) ==========

    // Monopolist can set price above marginal cost
    // Demonstrates deadweight loss, consumer surplus extraction
    void SetMonopolyPrice();

    // ========== Display ==========
    std::string GetInfoString() const override;
};
