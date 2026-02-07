#pragma once

#include "Person.h"

// ============================================================================
// Farmer.h - Inherits from Person
//
// Maps to: Production Function, Diminishing Returns, PPF, Bumper Harvest,
//          Fixed/Variable Costs, Technological Change, Agricultural Economics,
//          Marginal Product, Total/Average/Marginal Cost curves
// ============================================================================

class Farmer : public Person
{
private:
    // ========== Production Inputs ==========
    double m_land_size;        // Acres of land (fixed input in short run)
    int m_labor_hired;         // Number of workers (variable input)
    double m_technology_level; // 1.0 = base technology (shifts production function)
    double m_fertilizer_units; // Fertilizer used (variable input)

    // ========== Weather & Environmental Factors ==========
    double m_weather_factor; // 1.0 = normal, 1.5 = bumper harvest, 0.2 = disaster

    // ========== Output & Revenue ==========
    std::string m_crop;       // "rice", "wheat", etc.
    double m_output_quantity; // Total output this period
    double m_output_price;    // Market price for the output
    double m_revenue;         // Price * Quantity

    // ========== Cost Analysis (Cost Curves) ==========
    double m_fixed_cost;    // TFC: land rent, equipment (doesn't change with Q)
    double m_variable_cost; // TVC: wages, fertilizer (changes with Q)
    double m_total_cost;    // TC = TFC + TVC
    double m_average_cost;  // AC = TC / Q
    double m_marginal_cost; // MC = change in TC / change in Q

    // Profit
    double m_profit; // Revenue - Total Cost

    // ========== Production Function Parameters ==========
    // Output = f(Labor, Land, Technology) * Weather_Factor
    // Simplified: Output = (labor_hired * technology_level) * land_size * weather_factor
    // With diminishing returns: each additional worker adds less output

public:
    // ========== Constructors ==========
    Farmer(int id, const std::string &name, double initial_income,
           double land, const std::string &crop);

    // ========== Getters ==========
    double GetLandSize() const { return m_land_size; }
    int GetLaborHired() const { return m_labor_hired; }
    double GetTechnologyLevel() const { return m_technology_level; }
    double GetFertilizerUnits() const { return m_fertilizer_units; }
    double GetWeatherFactor() const { return m_weather_factor; }
    std::string GetCrop() const { return m_crop; }
    double GetOutputQuantity() const { return m_output_quantity; }
    double GetRevenue() const { return m_revenue; }
    double GetFixedCost() const { return m_fixed_cost; }
    double GetVariableCost() const { return m_variable_cost; }
    double GetTotalCost() const { return m_total_cost; }
    double GetAverageCost() const { return m_average_cost; }
    double GetMarginalCost() const { return m_marginal_cost; }
    double GetProfit() const { return m_profit; }

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

    // ========== Display ==========
    std::string GetInfoString() const override;
};
