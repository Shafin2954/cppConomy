#pragma once

#include "Consumer.h"


// ============================================================================
// Farmer.h - Inherits from Consumer
//
// Maps to: Production Function, Diminishing Returns, PPF, Bumper Harvest,
//          Fixed/Variable Costs, Technological Change, Agricultural Economics,
//          Marginal Product, Total/Average/Marginal Cost curves
// ============================================================================

class Farmer : public Consumer
{
private:
    // ========== Production Inputs ==========
    double land_size;        // Acres of land (fixed input in short run)
    int labor_hired;         // Number of workers (variable input)
    double technology_level; // 1.0 = base technology (shifts production function)
    double fertilizer_units; // Fertilizer used (variable input)

    // ========== Weather & Environmental Factors ==========
    double weather_factor; // 1.0 = normal, 1.5 = bumper harvest, 0.2 = disaster

    // ========== Output & Revenue ==========
    std::string crop;                     // "rice", "wheat", etc.
    double output_quantity;          // Total output this period
    double previous_output_quantity; // Previous period output (for tracking changes)
    double output_price;             // Market price for the output
    double revenue;                  // Price * Quantity

    // ========== Cost Analysis (Cost Curves) ==========
    double fixed_cost;    // TFC: land rent, equipment (doesn't change with Q)
    double variable_cost; // TVC: wages, fertilizer (changes with Q)
    double total_cost;    // TC = TFC + TVC
    double average_cost;  // AC = TC / Q
    double marginal_cost; // MC = change in TC / change in Q

    // Profit
    double profit; // Revenue - Total Cost

    // ========== Production Function Parameters ==========
    // Output = f(Labor, Land, Technology) * Weather_Factor
    // Simplified: Output = (labor_hired * technology_level) * land_size * weather_factor
    // With diminishing returns: each additional worker adds less output

public:
    // ========== Constructors ==========
    Farmer(int id, const std::string &name, double initial_income,
           double land, const std::string &crop);

    // ========== Getters ==========
    double GetLandSize() const { return land_size; }
    int GetLaborHired() const { return labor_hired; }
    double GetTechnologyLevel() const { return technology_level; }
    double GetFertilizerUnits() const { return fertilizer_units; }
    double GetWeatherFactor() const { return weather_factor; }
    std::string GetCrop() const { return crop; }
    double GetOutputQuantity() const { return output_quantity; }
    double GetRevenue() const { return revenue; }
    double GetFixedCost() const { return fixed_cost; }
    double GetVariableCost() const { return variable_cost; }
    double GetTotalCost() const { return total_cost; }
    double GetAverageCost() const { return average_cost; }
    double GetMarginalCost() const { return marginal_cost; }
    double GetProfit() const { return profit; }

    // ========== Production Actions ==========

    // Hire workers: increases variable cost, increases output (with diminishing returns)
    void Hire(int workers);

    // Fire workers: decreases variable cost, decreases output
    void Fire(int workers);

    // Plant crop: prepares for harvest
    void Plant(const std::string &new_crop);

    // Harvest: executes production function
    // Output = (labor_hired * technology_level) * land_size * weather_factor
    // Demonstrates:
    // - Bumper harvest: weather_factor = 1.5 -> floods market -> price crashes
    // - Diminishing returns: each worker adds less to output
    void Harvest();

    // Add fertilizer: increases variable cost, increases output
    void AddFertilizer(double units);

    // ========== Technology & Productivity ==========

    // Upgrade technology: shifts production function upward
    // Higher technology_level means same inputs produce more output
    void UpgradeTechnology();

    // ========== Cost Calculations ==========

    // Calculate all costs based on inputs
    // TFC = fixed (land rent, base equipment)
    // TVC = labor * wage + fertilizer * price
    void CalculateCosts(double wage_rate, double fertilizer_price);

    // Calculate production: implements production function with diminishing returns
    double CalculateProduction();

    // Calculate profit: revenue - total_cost
    void CalculateProfit(double output_price);

    // ========== Economic Concepts Demonstrated ==========

    // Marginal Product of Labor: how much output increases with one more worker
    // Shows diminishing returns: MP decreases as more workers hired
    double GetMarginalProductOfLabor() const;

    // Marginal Product of Fertilizer
    double GetMarginalProductOfFertilizer() const;

    // ========== Economic Decision Methods (for Propagation Engine) ==========

    // Get previous period's output (for tracking changes in propagation events)
    double GetPreviousOutputQuantity() const { return previous_output_quantity; }

    // Decide next period's production target based on profit signals
    // Low profit → reduce output, high profit → increase output
    // This is the core microeconomic response: firms adjust production to maximize profit
    void DecideNextOutputLevel();

    // Decide labor hiring/firing based on profitability
    // Implements the MR > MC hiring rule:
    //   If marginal revenue of additional worker > wage, hire
    //   If profit negative, fire workers to cut costs
    void DecideLaborDemand();

    // Decide investment in technology upgrades
    // High sustained profits → invest in better technology
    // This demonstrates long-run growth through productivity improvements
    void DecideInvestment();

    // ========== Display ==========
    std::string GetInfoString() const override;
};
