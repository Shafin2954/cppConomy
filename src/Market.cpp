#include "Market.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cmath>
using namespace std;

Market::Market(const string &product_name)
    : product_name(product_name),
      quantity_demanded(0.0),
      quantity_supplied(0.0),
      price_elasticity(1.0),
      current_price(0.0),
      equilibriuprice(0.0),
      equilibriuquantity(0.0),
      last_price(10.0), // Default to avoid division by zero
      shortage_surplus(0.0),
      tax_rate(0.0),
      subsidy_rate(0.0),
      min_wage(0.0),
      price_ceiling(0.0),
      price_floor(0.0),
      total_sales_value(0.0),
      consumer_surplus(0.0),
      producer_surplus(0.0),
      deadweight_loss(0.0)
{
    // Initialize default demand and supply curves
    // Rice (inelastic): Qd = 100 - 0.5*P, Qs = 10 + 1.5*P
    demand.intercept_a = 100.0;
    demand.slope_b = 0.5;
    supply.intercept_c = 10.0;
    supply.slope_d = 1.5;
}

void Market::FindEquilibrium()
{
    // Store previous price before calculating new equilibrium
    last_price = current_price;

    // Use EconomicEquations to find equilibrium
    auto [eq_price, eq_quantity] = EconomicEquations::FindEquilibrium(demand, supply);

    equilibriuprice = eq_price;
    equilibriuquantity = eq_quantity;
    current_price = eq_price;

    // Calculate quantities at equilibrium
    quantity_demanded = demand.Calculate(equilibriuprice);
    quantity_supplied = supply.Calculate(equilibriuprice);
    shortage_surplus = 0; // At equilibrium, shortage/surplus = 0

    // Calculate welfare measures
    CalculateWelfare();

    // Record the price
    RecordPrice(equilibriuprice);
}

void Market::AdjustPrice()
{
    shortage_surplus = quantity_demanded - quantity_supplied;

    // Only adjust price if there's actual supply/demand activity
    if (quantity_demanded > 0 || quantity_supplied > 0)
    {
        if (shortage_surplus > 0)
        {
            // Shortage: price rises
            current_price *= 1.02;
        }
        else if (shortage_surplus < 0)
        {
            // Surplus: price falls
            current_price *= 0.98;
        }

        // Recalculate quantities based on new price
        quantity_demanded = demand.Calculate(current_price);
        quantity_supplied = supply.Calculate(current_price);
    }
}

void Market::ApplyTaxAndSubsidy()
{
    // Taxes shift supply curve LEFT (supply decreases)
    // This increases equilibrium price and decreases equilibrium quantity
    // Effect: Qs_new = (c + d*P) * (1 - tax_rate)

    if (tax_rate > 0.0)
    {
        supply.intercept_c *= (1.0 - tax_rate);
        supply.slope_d *= (1.0 - tax_rate);
    }

    // Subsidies shift supply curve RIGHT (supply increases)
    // This decreases equilibrium price and increases equilibrium quantity
    // Effect: Qs_new = (c + d*P) * (1 + subsidy_rate)

    if (subsidy_rate > 0.0)
    {
        supply.intercept_c *= (1.0 + subsidy_rate);
        supply.slope_d *= (1.0 + subsidy_rate);
    }

    // Recalculate equilibrium after tax/subsidy
    FindEquilibrium();
}

void Market::EnforcePriceCeiling()
{
    if (price_ceiling > 0.0 && current_price > price_ceiling)
    {
        double old_price = current_price;
        current_price = price_ceiling;

        // Recalculate quantities at the ceiling price
        quantity_demanded = demand.Calculate(price_ceiling);
        quantity_supplied = supply.Calculate(price_ceiling);
        shortage_surplus = quantity_demanded - quantity_supplied;

        // A binding price ceiling creates a shortage (Qd > Qs)
        // This is deadweight loss
        if (shortage_surplus > 0)
        {
            // Only Qs units can be sold at the ceiling (quantity constrained)
            deadweight_loss = 0.5 * (old_price - price_ceiling) * shortage_surplus;
        }
    }
}

void Market::EnforcePriceFloor()
{
    if (price_floor > 0.0 && current_price < price_floor)
    {
        double old_price = current_price;
        current_price = price_floor;

        // Recalculate quantities at the floor price
        quantity_demanded = demand.Calculate(price_floor);
        quantity_supplied = supply.Calculate(price_floor);
        shortage_surplus = quantity_demanded - quantity_supplied;

        // A binding price floor creates a surplus (Qs > Qd)
        if (shortage_surplus < 0) // negative means surplus
        {
            // Only Qd units can be sold at the floor (quantity constrained)
            deadweight_loss = 0.5 * (price_floor - old_price) * abs(shortage_surplus);
        }
    }
}

double Market::CalculatePriceElasticity()
{
    // First find current equilibrium
    auto [eq_price, eq_qty] = EconomicEquations::FindEquilibrium(demand, supply);

    // Slightly increase price and see how quantity responds
    double new_price = eq_price * 1.01; // 1% price increase
    double new_qty = demand.Calculate(new_price);

    // Calculate elasticity using percentage changes
    price_elasticity = EconomicEquations::CalculatePriceElasticity(
        eq_qty, new_qty, eq_price, new_price);

    return price_elasticity;
}

void Market::ApplyElasticityToDemand(double newPrice)
{
    // When price changes, quantity demanded changes based on elasticity
    // Elastic goods (PED > 1): big change in quantity for small price change
    // Inelastic goods (PED < 1): small change in quantity for big price change

    double priceChange = (newPrice - current_price) / current_price;
    double quantityChange = priceChange * (-price_elasticity); // Negative because demand curve slopes down

    double oldQty = quantity_demanded;
    quantity_demanded *= (1.0 + quantityChange);
    current_price = newPrice;
}

void Market::CalculateWelfare()
{
    // Consumer Surplus = Area under demand curve - Amount paid
    // For linear demand: area of triangle = 0.5 * base * height
    // height = (demand intercept - actual price)
    // base = actual quantity

    double demand_intercept = demand.intercept_a / demand.slope_b; // P when Q=0
    consumer_surplus = EconomicEquations::CalculateConsumerSurplus(
        demand_intercept, equilibriuprice, equilibriuquantity);

    // Producer Surplus = Amount received - Cost to produce
    // For linear supply: area of triangle = 0.5 * base * height
    // height = actual price - (supply intercept)
    // base = actual quantity

    double supply_intercept = -supply.intercept_c / supply.slope_d; // P when Q=0
    producer_surplus = EconomicEquations::CalculateProducerSurplus(
        supply_intercept, equilibriuprice, equilibriuquantity);

    // Total welfare loss from market inefficiency (from taxes, subsidies, etc.)
    // If we're not at equilibrium, there's DWL
    if (abs(shortage_surplus) > 0.01)
    {
        deadweight_loss = 0.5 * abs(current_price - equilibriuprice) *
                          abs(shortage_surplus);
    }
}

string Market::GetDemandEquation() const
{
    ostringstream oss;
    oss << fixed << setprecision(2);

    oss << "Demand Curve: Qd = " << demand.intercept_a
        << " - " << demand.slope_b << "*P";

    return oss.str();
}

string Market::GetSupplyEquation() const
{
    ostringstream oss;
    oss << fixed << setprecision(2);

    oss << "Supply Curve: Qs = " << supply.intercept_c
        << " + " << supply.slope_d << "*P";

    return oss.str();
}

void Market::AddToGDP(double transaction_value)
{
    total_sales_value += transaction_value;
}

double Market::CalculateInflation()
{
    if (last_price <= 0)
        return 0.0;

    return EconomicEquations::CalculateInflation(current_price, last_price);
}

void Market::RecordPrice(double price)
{
    price_history.push_back(price);
    last_price = current_price;
    current_price = price;
}

void Market::TriggerBumperHarvest()
{
    // Bumper harvest: Supply curve shifts RIGHT
    // Effect: Qs_new = (c + d*P) * 1.3  (30% increase in supply)
    supply.intercept_c *= 1.3;
    supply.slope_d *= 1.3;

    // Recalculate equilibrium
    FindEquilibrium();
}

void Market::TriggerNaturalDisaster()
{
    // Natural disaster: Supply curve shifts LEFT
    // Effect: Qs_new = (c + d*P) * 0.5  (50% decrease in supply)
    supply.intercept_c *= 0.5;
    supply.slope_d *= 0.5;

    // Recalculate equilibrium
    FindEquilibrium();
}

string Market::GetInfoString() const
{
    ostringstream ss;
    ss << fixed << setprecision(2);

    ss << "╔════════════════════════════════════════════════╗\n";
    ss << "║         Market: " << left << setw(30) << product_name << " ║\n";
    ss << "╠════════════════════════════════════════════════╣\n";
    ss << "║ Current Price............... $" << setw(28) << current_price << " ║\n";
    ss << "║ Equilibrium Price........... $" << setw(28) << equilibriuprice << " ║\n";
    ss << "║ Quantity Demanded.......... " << setw(30) << quantity_demanded << " ║\n";
    ss << "║ Quantity Supplied.......... " << setw(30) << quantity_supplied << " ║\n";
    ss << "║ Shortage/Surplus........... " << setw(30) << shortage_surplus << " ║\n";
    ss << "╠════════════════════════════════════════════════╣\n";
    ss << "║ Tax Rate..................... " << setw(27) << (tax_rate * 100) << "% ║\n";
    ss << "║ Subsidy Rate................. " << setw(27) << (subsidy_rate * 100) << "% ║\n";
    ss << "║ Total Sales Value........... $" << setw(28) << total_sales_value << " ║\n";
    ss << "╚════════════════════════════════════════════════╝\n";

    return ss.str();
}

string Market::GetEquilibriumAnalysis() const
{
    ostringstream ss;
    ss << fixed << setprecision(2);

    ss << "╔════════════════════════════════════════════════════════════╗\n";
    ss << "║           MARKET EQUILIBRIUM ANALYSIS FOR " << left << setw(15) << product_name << "║\n";
    ss << "╠════════════════════════════════════════════════════════════╣\n";

    ss << "║ " << GetDemandEquation() << "\n";
    ss << "║ " << GetSupplyEquation() << "\n";

    ss << "╠════════════════════════════════════════════════════════════╣\n";
    ss << "║ EQUILIBRIUM\n";
    ss << "║   Price: $" << equilibriuprice << ",  Quantity: " << equilibriuquantity << " units\n";
    ss << "║\n";
    ss << "║ CURRENT STATE\n";
    ss << "║   Price: $" << current_price << "\n";
    ss << "║   Qd: " << quantity_demanded << ",  Qs: " << quantity_supplied << "\n";
    ss << "║   Shortage/Surplus: " << shortage_surplus << " units\n";

    if (abs(shortage_surplus) < 0.01)
    {
        ss << "║   Status: ✓ EQUILIBRIUM (Market Clears)\n";
    }
    else if (shortage_surplus > 0)
    {
        ss << "║   Status: ⚠️  SHORTAGE - Price should rise\n";
    }
    else
    {
        ss << "║   Status: ⚠️  SURPLUS - Price should fall\n";
    }

    ss << "╚════════════════════════════════════════════════════════════╝\n";

    return ss.str();
}

string Market::GetElasticityAnalysis() const
{
    ostringstream ss;
    ss << fixed << setprecision(2);

    ss << "╔════════════════════════════════════════════════════════════╗\n";
    ss << "║         PRICE ELASTICITY ANALYSIS FOR " << left << setw(20) << product_name << " ║\n";
    ss << "╠════════════════════════════════════════════════════════════╣\n";

    ss << "Price Elasticity of Demand (PED): " << price_elasticity << "\n";
    ss << "\n";

    if (price_elasticity > 1.0)
    {
        ss << "Classification: ELASTIC (|PED| > 1)\n";
        ss << "• Consumers are very responsive to price changes\n";
        ss << "• Luxury goods (ice cream, curd)\n";
        ss << "• Recommendation: LOWER PRICES to increase Total Revenue\n";
    }
    else if (price_elasticity < 1.0)
    {
        ss << "Classification: INELASTIC (|PED| < 1)\n";
        ss << "• Consumers are not very responsive to price changes\n";
        ss << "• Necessities (rice, salt, tobacco)\n";
        ss << "• Recommendation: INCREASE PRICES to increase Total Revenue\n";
    }
    else
    {
        ss << "Classification: UNIT ELASTIC (|PED| = 1)\n";
        ss << "• Price changes don't affect Total Revenue\n";
    }

    ss << "\n";
    double totalRevenue = current_price * quantity_demanded;
    ss << "Current Total Revenue: $" << totalRevenue << "\n";
    return ss.str();
}

string Market::GetWelfareAnalysis() const
{
    ostringstream ss;
    ss << fixed << setprecision(2);

    ss << "╔════════════════════════════════════════════════════════════╗\n";
    ss << "║         CONSUMER & PRODUCER WELFARE FOR " << left << setw(20) << product_name << "║\n";
    ss << "╠════════════════════════════════════════════════════════════╣\n";

    ss << "║ CONSUMER SURPLUS: $" << consumer_surplus << "\n";
    ss << "║   • Area below demand curve, above price paid\n";
    ss << "║   • Benefit consumers get from trade\n";
    ss << "║\n";
    ss << "║ PRODUCER SURPLUS: $" << producer_surplus << "\n";
    ss << "║   • Area above supply curve, below price received\n";
    ss << "║   • Benefit producers get from trade\n";
    ss << "║\n";
    ss << "║ TOTAL WELFARE: $" << (consumer_surplus + producer_surplus) << "\n";
    ss << "║   • Sum of consumer and producer surplus\n";
    ss << "║\n";

    if (deadweight_loss > 0.01)
    {
        ss << "║ DEADWEIGHT LOSS: $" << deadweight_loss << "\n";
        ss << "║   ⚠️  Market inefficiency detected!\n";
        ss << "║   • Caused by: taxes, subsidies, price controls, or monopoly\n";
        ss << "║   • Loss of economic efficiency\n";
    }
    else
    {
        ss << "║ DEADWEIGHT LOSS: $0.00\n";
        ss << "║   ✓ Market is efficient (perfect competition)\n";
    }

    ss << "╚════════════════════════════════════════════════════════════╝\n";

    return ss.str();
}
