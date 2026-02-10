#pragma once

#include <string>
#include <vector>
#include <map>
#include "Product.h"
#include "EconomicEquations.h"

using namespace std;

// ============================================================================
// Market.h - The marketplace where transactions occur and prices are determined
//
// Maps to: Supply & Demand, Equilibrium, Elasticity effects, Consumer Surplus,
//          Tax effects, GDP, Inflation, Shortage/Surplus, CPI
// ============================================================================

class Market
{
private:
    string product_name; // What is traded: "rice", "cloth", etc.

    // ========== Supply & Demand Functions ==========
    EconomicEquations::DemandFunction demand; // Qd = a - b*P
    EconomicEquations::SupplyFunction supply; // Qs = c + d*P

    double quantity_demanded; // How much consumers want to buy
    double quantity_supplied; // How much producers want to sell
    double price_elasticity;  // Elasticity coefficient for this product

    // ========== Price Discovery ==========
    double current_price;      // Actual market price
    double equilibriuprice;    // Where Qd = Qs
    double equilibriuquantity; // Quantity at equilibrium
    double last_price;         // Previous period price (for inflation)

    // ========== Shortage/Surplus ==========
    double shortage_surplus; // Qd - Qs (positive = shortage, negative = surplus)

    // ========== Market Policies ==========
    double tax_rate;      // Tax on goods (shifts supply curve up = higher price)
    double subsidy_rate;  // Subsidy on goods (shifts supply curve down = lower price)
    double min_wage;      // For labor market specifically
    double price_ceiling; // Maximum allowed price (causes shortage if binding)
    double price_floor;   // Minimum allowed price (causes surplus if binding)

    // ========== Market Aggregates (for GDP & Inflation) ==========
    double total_sales_value;     // Sum of all transactions: Σ(Price * Quantity)
    vector<double> price_history; // Track prices over time for inflation calc

    // ========== Economic Metrics ==========
    double consumer_surplus; // Area under demand curve - price paid
    double producer_surplus; // Price received - area under supply curve
    double deadweight_loss;  // Loss from market inefficiency

    // Inventory tracking
    map<string, double> inventory;

public:
    // ========== Constructors ==========
    Market(const string &product_name);

    // ========== Demand & Supply Setup ==========
    // These define the shape of the demand and supply curves

    // Set demand function: Qd = intercept_a - slope_b * P
    // Higher intercept_a = more demand at every price
    // Higher slope_b = more elastic (responsive to price)
    void SetDemandFunction(double intercept_a, double slope_b)
    {
        demand.intercept_a = intercept_a;
        demand.slope_b = slope_b;
    }

    // Set supply function: Qs = intercept_c + slope_d * P
    void SetSupplyFunction(double intercept_c, double slope_d)
    {
        supply.intercept_c = intercept_c;
        supply.slope_d = slope_d;
    }

    // Get demand and supply functions
    const EconomicEquations::DemandFunction &GetDemandFunction() const { return demand; }
    const EconomicEquations::SupplyFunction &GetSupplyFunction() const { return supply; }

    // ========== Getters ==========
    string GetProductName() const { return product_name; }
    double GetQuantityDemanded() const { return quantity_demanded; }
    double GetQuantitySupplied() const { return quantity_supplied; }
    double GetCurrentPrice() const { return current_price; }
    double GetEquilibriumPrice() const { return equilibriuprice; }
    double GetEquilibriumQuantity() const { return equilibriuquantity; }
    double GetShortageSurplus() const { return shortage_surplus; }
    double GetTaxRate() const { return tax_rate; }
    double GetSubsidyRate() const { return subsidy_rate; }
    double GetTotalSalesValue() const { return total_sales_value; }
    double GetPriceElasticity() const { return price_elasticity; }
    double GetConsumerSurplus() const { return consumer_surplus; }
    double GetProducerSurplus() const { return producer_surplus; }
    double GetDeadweightLoss() const { return deadweight_loss; }

    // ========== Propagation Helpers ==========
    // Get last period's price (for detecting price changes in propagation)
    double GetLastPrice() const { return last_price; }

    // ========== Setters ==========
    void SetDemand(double quantity) { quantity_demanded = quantity; }
    void SetSupply(double quantity) { quantity_supplied = quantity; }
    void SetPrice(double price) { current_price = price; }
    void SetTaxRate(double rate) { tax_rate = rate; }
    void SetSubsidyRate(double rate) { subsidy_rate = rate; }
    void SetMinWage(double wage) { min_wage = wage; }
    void SetPriceCeiling(double price) { price_ceiling = price; }
    void SetPriceFloor(double price) { price_floor = price; }
    void SetPriceElasticity(double elasticity) { price_elasticity = elasticity; }

    // ========== Market Mechanism ==========

    // Find equilibrium: where Quantity Demanded = Quantity Supplied
    // Uses: EconomicEquations::FindEquilibrium
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

    // Calculate and apply price elasticity
    // Shows how responsive quantity demanded is to price changes
    double CalculatePriceElasticity();

    // Update demand based on elasticity and price shocks
    // Different goods have different elasticities
    // Rice: inelastic ~0.3 (people must eat)
    // Ice cream: elastic ~1.8 (luxury, substitute available - curd)
    // Tobacco (addictive): inelastic ~0.1
    void ApplyElasticityToDemand(double newPrice);

    // ========== Consumer & Producer Welfare ==========

    // Calculate consumer surplus, producer surplus, and deadweight loss
    void CalculateWelfare();

    // Display the demand and supply equations
    string GetDemandEquation() const;
    string GetSupplyEquation() const;

    // ========== GDP & Inflation Calculations ==========

    // Add to total production value (for GDP calculation)
    void AddToGDP(double transaction_value);

    // Calculate inflation for this market
    // Inflation = (Current Price - Last Price) / Last Price * 100%
    // Shows: how prices change over time across economy
    double CalculateInflation();

    // Track price history for inflation calculation
    void RecordPrice(double price);

    // ========== Supply Shocks (Bumper Harvest, Natural Disasters) ==========

    // Bumper harvest: Supply curve shifts right (supply increases)
    // Effect: Price falls, consumer benefits, producer loses revenue
    void TriggerBumperHarvest();

    // Natural disaster: Supply curve shifts left (supply decreases)
    // Effect: Price rises, producer benefits (higher P), consumer pays more
    void TriggerNaturalDisaster();

    // ========== Display ==========
    string GetInfoString() const;
    string GetEquilibriumAnalysis() const;
    string GetElasticityAnalysis() const;
    string GetWelfareAnalysis() const;
};
