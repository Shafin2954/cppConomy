#include "Owner.h"
#include <sstream>
#include <cmath>
#include <algorithm>
#include "Logger.h"

using namespace std;

Owner::Owner(int id, const string &name, double initial_capital,
             const string &product_type, bool is_monopoly)
    : Consumer(id, name, initial_capital),
      capital_machines(initial_capital),
      labor_hired(0),
      technology_level(1.0),
      product_type(product_type),
      production_output(0.0),
      is_monopoly(is_monopoly),
      current_price(0.0),
      revenue(0.0),
      total_fixed_cost(200.0),
      total_variable_cost(0.0),
      total_cost(0.0),
      average_fixed_cost(0.0),
      average_variable_cost(0.0),
      average_cost(0.0),
      marginal_cost(0.0),
      marginal_revenue(0.0),
      profit(0.0),
      is_operating(true)
{
}

void Owner::Hire()
{
    labor_hired += 1;
}

void Owner::Fire()
{
    if (labor_hired > 0)
        labor_hired -= 1;
}

void Owner::Produce()
{
    // Simplified production function: Q = (labor * tech) * sqrt(capital)
    double capital_factor = sqrt(max(1.0, capital_machines));
    production_output = (labor_hired * technology_level) * capital_factor;
}

void Owner::CalculateCosts(double wage_rate, double material_cost_per_unit)
{
    total_variable_cost = (labor_hired * wage_rate) + (production_output * material_cost_per_unit);
    total_cost = total_fixed_cost + total_variable_cost;

    if (production_output > 0)
    {
        average_fixed_cost = total_fixed_cost / production_output;
        average_variable_cost = total_variable_cost / production_output;
        average_cost = total_cost / production_output;
        marginal_cost = total_variable_cost / production_output;
    }
    else
    {
        average_fixed_cost = 0.0;
        average_variable_cost = 0.0;
        average_cost = 0.0;
        marginal_cost = 0.0;
    }
}

void Owner::CalculateRevenue()
{
    revenue = current_price * production_output;
}

void Owner::CalculateProfit()
{
    profit = revenue - total_cost;
}

int Owner::GetOptimalQuantity(double market_price)
{
    // Simple rule: produce where price ~= marginal cost
    current_price = market_price;

    if (marginal_cost <= 0.0)
        return 0;

    // If MC < Price, produce more; if MC > Price, produce less
    // Here, just approximate optimal quantity as current output
    return static_cast<int>(production_output);
}

void Owner::CheckShutdownCondition(double market_price)
{
    current_price = market_price;

    // Shutdown if price < AVC
    if (average_variable_cost > 0.0 && market_price < average_variable_cost)
    {
        is_operating = false;
        production_output = 0.0;
    }
    else
    {
        is_operating = true;
    }
}

void Owner::ExpandFactory()
{
    // Double capital and labor
    capital_machines *= 2.0;
    labor_hired *= 2;
    technology_level *= 1.1;
}

void Owner::SetMonopolyPrice()
{
    if (!is_monopoly)
        return;

    // Simple monopoly pricing: set price above MC
    if (marginal_cost > 0.0)
        current_price = marginal_cost * 1.5;
}

// ========== Economic Decision Methods ==========

void Owner::DecideProduction()
{
    // Adjust production level based on profitability
    // This implements the MR = MC profit maximization rule

    if (revenue <= 0.0)
        return; // No market data yet

    double profitMargin = profit / revenue;

    if (profitMargin < 0.05 && labor_hired > 0)
    {
        // Very low or negative profit: cut production by firing a worker
        // This demonstrates the connection:
        //   Low prices → low revenue → low profit → job cuts
        Fire();
        LOG_INFO("Owner " + name + " reduced production (profit margin: " +
                 to_string(profitMargin * 100) + "%)");
    }
    else if (profitMargin > 0.25 && labor_hired < 20)
    {
        // High profit: expand production by hiring
        // MR > MC, so hiring more workers increases profit
        Hire();
        LOG_INFO("Owner " + name + " expanded production (profit margin: " +
                 to_string(profitMargin * 100) + "%)");
    }

    // Recalculate output with new labor level
    Produce();
}

void Owner::DecideInvestment()
{
    // Invest in capital when profits justify it
    // This drives capital accumulation and long-run growth

    if (profit > 1000.0 && capital_machines < 50000.0)
    {
        // Strong profits: invest 10% of profit in new capital
        // Capital investment increases production capacity
        double investment = profit * 0.10;
        capital_machines += investment;

        LOG_SUCCESS("Owner " + name + " invested " +
                    to_string(investment) + " in capital (total: " +
                    to_string(capital_machines) + ")");
    }
}

bool Owner::DecideMarketExit()
{
    // Implement shutdown decision
    // This is the classic microeconomic choice:
    //   Short run: shut down if P < AVC (can't cover variable costs)
    //   Long run: exit if P < AC (can't cover all costs)

    if (average_variable_cost <= 0.0)
        return false; // No cost data yet

    // Short-run shutdown condition
    if (current_price < average_variable_cost)
    {
        // Price below AVC: better to shut down and avoid variable costs
        is_operating = false;
        production_output = 0.0;

        LOG_WARNING("Owner " + name + " shut down (P < AVC: " +
                    to_string(current_price) + " < " +
                    to_string(average_variable_cost) + ")");
        return true;
    }

    // Long-run exit condition (would need sustained losses)
    if (profit < -500.0)
    {
        // Sustained large losses: consider exiting market
        LOG_WARNING("Owner " + name + " considering market exit (profit: " +
                    to_string(profit) + ")");
        // In a full simulation, would actually remove the firm
        return true;
    }

    return false;
}

string Owner::GetInfoString() const
{
    ostringstream ss;
    ss << "Owner: " << name << " (ID=" << id << ")\n";
    ss << "Product: " << product_type << ", Monopoly: " << (is_monopoly ? "Yes" : "No") << "\n";
    ss << "Capital: " << capital_machines << ", Labor: " << labor_hired << "\n";
    ss << "Output: " << production_output << ", Price: " << current_price << "\n";
    ss << "Revenue: " << revenue << ", Profit: " << profit << "\n";
    ss << "Costs - TC: " << total_cost << ", AC: " << average_cost
       << ", MC: " << marginal_cost << "\n";
    return ss.str();
}
