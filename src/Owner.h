#pragma once

#include "Consumer.h"


// ============================================================================
// Owner.h - Inherits from Consumer (The Business Owner/Manufacturer)
//
// Maps to: Production Function, Cost Curves, Monopoly, Shutdown Condition,
//          Marginal Cost, Marginal Revenue, Profit Maximization,
//          Returns to Scale, Market Power
// ============================================================================

class Owner : public Consumer
{
private:
    // ========== Production Inputs (Factors of Production) ==========
    double capital_machines; // Capital stock (machinery, equipment)
    int labor_hired;         // Number of workers
    double technology_level; // Affects productivity: higher = more output per input

    std::string product_type;      // What they produce: "cloth", "ice_cream", etc.
    double production_output; // Current production quantity

    // ========== Market Power ==========
    bool is_monopoly;     // If true: sets own price (price maker)
                          // If false: takes market price (price taker)
    double current_price; // Price they charge (or market price if competitive)
    double revenue;       // Price * Quantity

    // ========== Cost Structure (Cost Curves) ==========
    // Shows: Total Cost, Average Cost, Marginal Cost curves
    double total_fixed_cost;      // TFC: machinery rent, overhead (doesn't change with output)
    double total_variable_cost;   // TVC: wages, materials (changes with output)
    double total_cost;            // TC = TFC + TVC
    double average_fixed_cost;    // AFC = TFC / Q
    double average_variable_cost; // AVC = TVC / Q
    double average_cost;          // AC = TC / Q
    double marginal_cost;         // MC = change in TC / change in Q

    // ========== Revenue Analysis ==========
    double marginal_revenue; // MR = change in TR / change in Q

    // Profit
    double profit; // TR - TC (or can be negative = loss)

    // ========== Shutdown Decision ==========
    // Short run: produce if Price >= AVC (covers variable costs)
    // Long run: produce if Price >= AC (covers all costs)
    bool is_operating; // False if price too low -> shutdown

public:
    // ========== Constructors ==========
    Owner(int id, const std::string &name, double initial_capital,
          const std::string &product_type, bool is_monopoly);

    // ========== Getters ==========
    double GetCapital() const { return capital_machines; }
    int GetLaborHired() const { return labor_hired; }
    double GetTechnologyLevel() const { return technology_level; }
    std::string GetProductType() const { return product_type; }
    double GetProduction() const { return production_output; }
    bool IsMonopoly() const { return is_monopoly; }
    double GetPrice() const { return current_price; }
    double GetRevenue() const { return revenue; }
    double GetTotalFixedCost() const { return total_fixed_cost; }
    double GetTotalVariableCost() const { return total_variable_cost; }
    double GetTotalCost() const { return total_cost; }
    double GetAverageCost() const { return average_cost; }
    double GetAverageVariableCost() const { return average_variable_cost; }
    double GetMarginalCost() const { return marginal_cost; }
    double GetMarginalRevenue() const { return marginal_revenue; }
    double GetProfit() const { return profit; }
    bool IsOperating() const { return is_operating; }

    // ========== Setters ==========
    void SetPrice(double price) { current_price = price; }

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

    // ========== Economic Decision Methods (for Propagation Engine) ==========

    // Decide production level based on profitability and market conditions
    // Implements profit maximization: produce where MR = MC
    // Low profit → reduce output, high profit → expand output
    void DecideProduction();

    // Decide capital investment based on expected returns
    // High profit → invest in capital (expand capacity)
    // This drives long-run economic growth through capital accumulation
    void DecideInvestment();

    // Decide whether to exit the market (shutdown decision)
    // Short run: exit if P < AVC (can't cover variable costs)
    // Long run: exit if P < AC (can't cover total costs)
    // Returns true if firm shuts down
    bool DecideMarketExit();

    // ========== Display ==========
    std::string GetInfoString() const override;
};
