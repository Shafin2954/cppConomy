#include "Farmer.h"
#include <sstream>
#include <algorithm>
#include <cmath>
#include "Logger.h"

using namespace std;

Farmer::Farmer(int id, const string &name, double initial_income,
               double land, const string &crop)
    : Consumer(id, name, initial_income),
      land_size(land),
      labor_hired(0),
      technology_level(1.0),
      fertilizer_units(0.0),
      weather_factor(1.0),
      crop(crop),
      output_quantity(0.0),
      previous_output_quantity(0.0),
      output_price(0.0),
      revenue(0.0),
      fixed_cost(100.0),
      variable_cost(0.0),
      total_cost(0.0),
      average_cost(0.0),
      marginal_cost(0.0),
      profit(0.0)
{
}

void Farmer::Hire(int workers)
{
    if (workers <= 0)
        return;
    labor_hired += workers;
}

void Farmer::Fire(int workers)
{
    if (workers <= 0)
        return;
    labor_hired = max(0, labor_hired - workers);
}

void Farmer::Plant(const string &new_crop)
{
    crop = new_crop;
}

void Farmer::Harvest()
{
    // Store previous output before calculating new harvest
    previous_output_quantity = output_quantity;
    output_quantity = CalculateProduction();
}

void Farmer::AddFertilizer(double units)
{
    if (units <= 0)
        return;
    fertilizer_units += units;
}

void Farmer::UpgradeTechnology()
{
    technology_level += 0.1;
}

void Farmer::CalculateCosts(double wage_rate, double fertilizer_price)
{
    variable_cost = (labor_hired * wage_rate) + (fertilizer_units * fertilizer_price);
    total_cost = fixed_cost + variable_cost;
    if (output_quantity > 0)
    {
        average_cost = total_cost / output_quantity;
    }
    else
    {
        average_cost = 0.0;
    }

    // Simplified marginal cost: variable cost per unit
    if (output_quantity > 0)
    {
        marginal_cost = variable_cost / output_quantity;
    }
    else
    {
        marginal_cost = 0.0;
    }
}

double Farmer::CalculateProduction()
{
    // Production function with diminishing returns
    // Output = land * tech * weather * sqrt(labor + fertilizer)
    double input = sqrt(static_cast<double>(labor_hired) + fertilizer_units + 1.0);
    return land_size * technology_level * weather_factor * input;
}

void Farmer::CalculateProfit(double output_price)
{
    output_price = output_price;
    revenue = output_quantity * output_price;
    profit = revenue - total_cost;
}

double Farmer::GetMarginalProductOfLabor() const
{
    // Approximate MP of labor by derivative of sqrt(x)
    if (labor_hired <= 0)
        return 0.0;
    return (land_size * technology_level * weather_factor) / (2.0 * sqrt(labor_hired));
}

double Farmer::GetMarginalProductOfFertilizer() const
{
    if (fertilizer_units <= 0)
        return 0.0;
    return (land_size * technology_level * weather_factor) / (2.0 * sqrt(fertilizer_units));
}

// ========== Economic Decision Methods ==========

void Farmer::DecideNextOutputLevel()
{
    // Farmers adjust production targets based on profitability
    // This demonstrates profit-maximization behavior

    if (revenue <= 0.0)
        return; // No data yet

    // Calculate profit margin (profit as % of revenue)
    double profitMargin = profit / revenue;

    // Decision rule:
    //   Profit margin < 10%: reduce output next period (cut losses)
    //   Profit margin > 30%: increase output next period (capture gains)
    //   Otherwise: maintain current level

    if (profitMargin < 0.10)
    {
        // Losing money or very low profit: reduce fertilizer to cut costs
        // This is a short-run cost-cutting response
        fertilizer_units *= 0.85; // Reduce fertilizer by 15%
        LOG_INFO("Farmer " + name + " reducing fertilizer due to low profit (margin: " +
                 to_string(profitMargin * 100) + "%)");
    }
    else if (profitMargin > 0.30)
    {
        // High profit: expand production by adding fertilizer
        // This is a profit-maximizing expansion
        fertilizer_units *= 1.15; // Increase fertilizer by 15%
        LOG_INFO("Farmer " + name + " increasing fertilizer due to high profit (margin: " +
                 to_string(profitMargin * 100) + "%)");
    }

    // Clamp fertilizer to reasonable bounds
    fertilizer_units = clamp(fertilizer_units, 0.0, land_size * 2.0);
}

void Farmer::DecideLaborDemand()
{
    // Hire or fire workers based on profitability
    // Implements the MR = MC optimization rule

    if (profit < -100.0 && labor_hired > 0)
    {
        // Significant losses: fire a worker to cut variable costs
        // This demonstrates the employment-profit connection:
        //   Low prices → low profit → layoffs → unemployment
        Fire(1);
        LOG_INFO("Farmer " + name + " fired 1 worker (profit: " +
                 to_string(profit) + ")");
    }
    else if (profit > 300.0 && labor_hired < 10)
    {
        // Strong profits and room to expand: hire a worker
        // Marginal product of labor is positive, so hiring increases output
        Hire(1);
        LOG_INFO("Farmer " + name + " hired 1 worker (profit: " +
                 to_string(profit) + ")");
    }
}

void Farmer::DecideInvestment()
{
    // Invest in technology upgrades if profitable over sustained period
    // This represents long-run productivity growth

    // Simple rule: if profit margin consistently high, upgrade technology
    // (In a real simulation, would track profit history)

    if (profit > 500.0 && technology_level < 2.0)
    {
        // High profits justify capital investment
        // Technology upgrade increases output per unit input (productivity growth)
        UpgradeTechnology();
        LOG_SUCCESS("Farmer " + name + " upgraded technology to level " +
                    to_string(technology_level));
    }
}

string Farmer::GetInfoString() const
{
    ostringstream ss;
    ss << "Farmer: " << name << " (ID=" << id << ")\n";
    ss << "Crop: " << crop << ", Land: " << land_size << " acres\n";
    ss << "Labor: " << labor_hired << ", Fertilizer: " << fertilizer_units << "\n";
    ss << "Output: " << output_quantity << ", Price: " << output_price << "\n";
    ss << "Revenue: " << revenue << ", Profit: " << profit << "\n";
    ss << "Costs - Fixed: " << fixed_cost << ", Variable: " << variable_cost
       << ", Total: " << total_cost << "\n";
    return ss.str();
}
