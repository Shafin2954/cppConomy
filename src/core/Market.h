#pragma once

#include <string>
#include <vector>
#include <map>
#include "Product.h"

// ============================================================================
// Market.h - The marketplace where transactions occur and prices are determined
//
// Maps to: Supply & Demand, Equilibrium, Elasticity effects, Consumer Surplus,
//          Tax effects, GDP, Inflation, Shortage/Surplus, CPI
// ============================================================================

class Market
{
private:
    std::string m_product_name; // What is traded: "rice", "cloth", etc.

    // ========== Supply & Demand ==========
    double m_quantity_demanded; // How much consumers want to buy
    double m_quantity_supplied; // How much producers want to sell

    // ========== Price Discovery ==========
    double m_current_price;     // Actual market price
    double m_equilibrium_price; // Where Qd = Qs
    double m_last_price;        // Previous period price (for inflation)

    // ========== Shortage/Surplus ==========
    double m_shortage_surplus; // Qd - Qs (positive = shortage, negative = surplus)

    // ========== Market Policies ==========
    double m_tax_rate;      // Tax on goods (shifts supply curve up = higher price)
    double m_subsidy_rate;  // Subsidy on goods (shifts supply curve down = lower price)
    double m_min_wage;      // For labor market specifically
    double m_price_ceiling; // Maximum allowed price (causes shortage if binding)
    double m_price_floor;   // Minimum allowed price (causes surplus if binding)

    // ========== Market Aggregates (for GDP & Inflation) ==========
    double m_total_sales_value;          // Sum of all transactions: Σ(Price * Quantity)
    std::vector<double> m_price_history; // Track prices over time for inflation calc

    // Inventory tracking
    std::map<std::string, double> m_inventory;

public:
    // ========== Constructors ==========
    Market(const std::string &product_name);

    // ========== Getters ==========
    std::string GetProductName() const { return m_product_name; }
    double GetQuantityDemanded() const { return m_quantity_demanded; }
    double GetQuantitySupplied() const { return m_quantity_supplied; }
    double GetCurrentPrice() const { return m_current_price; }
    double GetEquilibriumPrice() const { return m_equilibrium_price; }
    double GetShortageSurplus() const { return m_shortage_surplus; }
    double GetTaxRate() const { return m_tax_rate; }
    double GetSubsidyRate() const { return m_subsidy_rate; }
    double GetTotalSalesValue() const { return m_total_sales_value; }

    // ========== Setters ==========
    void SetDemand(double quantity) { m_quantity_demanded = quantity; }
    void SetSupply(double quantity) { m_quantity_supplied = quantity; }
    void SetPrice(double price) { m_current_price = price; }
    void SetTaxRate(double rate) { m_tax_rate = rate; }
    void SetSubsidyRate(double rate) { m_subsidy_rate = rate; }
    void SetMinWage(double wage) { m_min_wage = wage; }
    void SetPriceCeiling(double price) { m_price_ceiling = price; }
    void SetPriceFloor(double price) { m_price_floor = price; }

    // ========== Market Mechanism ==========

    // Find equilibrium: where Quantity Demanded = Quantity Supplied
    // Shows: intersection of demand and supply curves
    // Calculates equilibrium price (where market clears)
    void FindEquilibrium();

    // Adjust price toward equilibrium
    // If Qd > Qs (shortage): price rises (inventory depletes)
    // If Qd < Qs (surplus): price falls (inventory builds)
    // This demonstrates price mechanism as solution to shortage/surplus
    void AdjustPrice();

    // Apply market policies
    // Tax: shifts supply curve left (S decreases) -> price up, quantity down
    // Subsidy: shifts supply curve right (S increases) -> price down, quantity up
    // Shows deadweight loss from price controls
    void ApplyTaxAndSubsidy();

    // Enforce price ceiling
    // If market price would exceed ceiling: shortage develops
    // Demonstrates why price controls create shortages
    void EnforcePriceCeiling();

    // Enforce price floor
    // If market price would fall below floor: surplus develops
    // Demonstrates surplus from price floors (e.g., minimum wage unemployment)
    void EnforcePriceFloor();

    // ========== Elasticity Effects ==========

    // Calculate price elasticity of demand
    // Shows how responsive quantity demanded is to price changes
    // If elastic (>1): quantity very responsive, elastic goods
    // If inelastic (<1): quantity not responsive, inelastic goods (rice)
    double CalculatePriceElasticity(double price_change_percent,
                                    double quantity_change_percent);

    // Apply elasticity to demand curve
    // Different goods have different elasticities
    // Rice: inelastic (people must eat)
    // Ice cream: elastic (luxury, substitutes available)
    void ApplyElasticity(double elasticity_coefficient);

    // ========== GDP & Inflation Calculations ==========

    // Add to total production value (for GDP calculation)
    void AddToGDP(double transaction_value);

    // Calculate inflation for this market
    // Inflation = (Current CPI - Last CPI) / Last CPI * 100%
    // Shows: how prices change over time across economy
    double CalculateInflation();

    // Track price history for inflation calculation
    void RecordPrice(double price);

    // ========== Display ==========
    std::string GetInfoString() const;
    std::string GetEquilibriumAnalysis() const;
};
