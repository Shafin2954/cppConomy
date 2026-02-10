#include "Consumer.h"
#include <sstream>
#include <algorithm>
#include <cmath>
using namespace std;

// ============================================================================
// Consumer.cpp - Economic agent with consumer behavior
// ============================================================================

Consumer::Consumer(int id, const string &name, double initial_income)
    : id(id), name(name), age(18), is_alive(true),
      literacy_score(0.5), wallet(initial_income), savings(0.0),
      monthly_income(initial_income), total_utility(0.0),
      addiction_level(0.0), income_tax_paid(0.0)
{
    // Default preferences (can be updated from commands)
    preferences["rice"] = 1.0;
    preferences["ice_cream"] = 1.0;
    preferences["curd"] = 1.0;
    preferences["cloth"] = 1.0;

    // Default willingness to pay (max prices)
    willingness_to_pay["rice"] = 100.0;
    willingness_to_pay["ice_cream"] = 50.0;
    willingness_to_pay["curd"] = 80.0;
    willingness_to_pay["cloth"] = 150.0;

    // Default substitution ratios (how easily to substitute between products)
    substitution_ratios[{"rice", "curd"}] = 0.8;      // Rice and curd are somewhat substitutable
    substitution_ratios[{"ice_cream", "curd"}] = 0.6; // Ice cream and curd less substitutable
    substitution_ratios[{"rice", "cloth"}] = 0.2;     // Cloth and food are not very substitutable
}

double Consumer::GetQuantityDemanded(const string &product) const
{
    auto it = quantity_demanded.find(product);
    if (it != quantity_demanded.end())
        return it->second;
    return 0.0;
}

double Consumer::GetWillingnessToPay(const string &product) const
{
    auto it = willingness_to_pay.find(product);
    if (it != willingness_to_pay.end())
        return it->second;
    return 0.0;
}

double Consumer::GetSubstitutionRatio(const string &product1, const string &product2) const
{
    auto it = substitution_ratios.find({product1, product2});
    if (it != substitution_ratios.end())
        return it->second;

    // Try reverse order
    it = substitution_ratios.find({product2, product1});
    if (it != substitution_ratios.end())
        return it->second;

    return 0.0; // Not substitutable
}

void Consumer::SetPreference(const string &product, double weight)
{
    preferences[product] = max(0.0, weight);
}

void Consumer::SetWillingnessToPay(const string &product, double max_price)
{
    willingness_to_pay[product] = max(0.0, max_price);
}

void Consumer::SetSubstitutionRatio(const string &product1, const string &product2, double ratio)
{
    substitution_ratios[{product1, product2}] = max(0.0, ratio);
}

void Consumer::SetupDemandCurve(const string &product, double max_quantity, double price_sensitivity)
{
    if (max_quantity <= 0.0 || price_sensitivity <= 0.0)
        return;

    vector<DemandPoint> curve;

    // Generate demand curve points (linear: Qd = max_quantity - price_sensitivity * P)
    for (double price = 0.0; price <= willingness_to_pay[product]; price += 5.0)
    {
        double qty = max_quantity - price_sensitivity * price;
        qty = max(0.0, qty);
        curve.push_back({price, qty});
    }

    demand_curves[product] = curve;
}

double Consumer::CalculateDemandAtPrice(const string &product, double price)
{
    // If no demand curve defined, use simple rule based on willingness to pay
    if (demand_curves.find(product) == demand_curves.end())
    {
        double wtp = GetWillingnessToPay(product);
        if (price > wtp)
            return 0.0;

        // Simple linear demand: Qd = preference * (willingness_to_pay - price) / willingness_to_pay
        double preference = 1.0;
        auto it = preferences.find(product);
        if (it != preferences.end())
            preference = it->second;

        return preference * (wtp - price) / wtp * 10.0; // Scale to reasonable quantity
    }

    // Interpolate from demand curve
    const auto &curve = demand_curves[product];
    if (curve.empty())
        return 0.0;

    // Find appropriate points and interpolate
    if (price <= curve.front().price)
        return curve.front().quantity;
    if (price >= curve.back().price)
        return curve.back().quantity;

    for (size_t i = 0; i < curve.size() - 1; ++i)
    {
        if (price >= curve[i].price && price <= curve[i + 1].price)
        {
            // Linear interpolation
            double t = (price - curve[i].price) / (curve[i + 1].price - curve[i].price);
            return curve[i].quantity + t * (curve[i + 1].quantity - curve[i].quantity);
        }
    }

    return 0.0;
}

void Consumer::UpdateQuantityDemanded(const string &product, double market_price)
{
    quantity_demanded[product] = CalculateDemandAtPrice(product, market_price);
}

void Consumer::Consume(const string &product, double quantity, double price)
{
    if (!is_alive || quantity <= 0.0 || price <= 0.0)
        return;

    double total_cost = quantity * price;

    // Budget constraint: cannot spend more than wallet
    if (total_cost > wallet)
    {
        // Buy as much as possible with remaining budget
        quantity = wallet / price;
        total_cost = quantity * price;
    }

    // Spend and update owned goods
    wallet -= total_cost;
    goods_owned[product] += quantity;

    // Update utility after consumption
    UpdateUtility();
}

void Consumer::UpdateUtility()
{
    // Simple diminishing marginal utility:
    // Utility = sum over goods of preference * log(1 + quantity)
    double utility = 0.0;
    for (const auto &item : goods_owned)
    {
        const string &product = item.first;
        double quantity = item.second;
        double preference = 1.0;

        auto it = preferences.find(product);
        if (it != preferences.end())
            preference = it->second;

        utility += preference * log(1.0 + quantity);
    }

    // Addiction: if addiction level is high, utility from addictive goods is boosted
    if (addiction_level > 0.0)
    {
        auto it = goods_owned.find("ice_cream");
        if (it != goods_owned.end())
            utility += addiction_level * log(1.0 + it->second);
    }

    total_utility = utility;
}

void Consumer::SubstituteGood(const string &froproduct,
                              const string &to_product,
                              double quantity)
{
    if (quantity <= 0.0)
        return;

    double &froqty = goods_owned[froproduct];
    double actual = min(froqty, quantity);

    froqty -= actual;
    goods_owned[to_product] += actual;

    UpdateUtility();
}

void Consumer::PayTax(double tax_amount)
{
    if (tax_amount <= 0.0)
        return;

    double paid = min(wallet, tax_amount);
    wallet -= paid;
    income_tax_paid += paid;
}

void Consumer::AgeTick()
{
    if (!is_alive)
        return;

    age++;

    // Simple mortality rule: small chance after 60
    if (age > 60)
    {
        double death_chance = 0.01 * (age - 60); // grows with age
        if (death_chance > 0.5)
            death_chance = 0.5;

        // Deterministic mortality for now (no RNG) – die at 90
        if (age >= 90)
            is_alive = false;
    }

    // Literacy slowly improves with time (simplified)
    literacy_score = min(1.0, literacy_score + 0.005);
}

string Consumer::GetInfoString() const
{
    ostringstream ss;
    ss << "Consumer: " << name << " (ID=" << id << ")\n";
    ss << "Age: " << age << ", Alive: " << (is_alive ? "Yes" : "No") << "\n";
    ss << "Income: " << monthly_income << ", Wallet: " << wallet
       << ", Savings: " << savings << "\n";
    ss << "Utility: " << total_utility << ", Literacy: " << literacy_score << "\n";
    ss << "Addiction: " << addiction_level << ", Tax Paid: " << income_tax_paid << "\n";

    ss << "Goods Owned: ";
    if (goods_owned.empty())
    {
        ss << "(none)";
    }
    else
    {
        bool first = true;
        for (const auto &item : goods_owned)
        {
            if (!first)
                ss << ", ";
            ss << item.first << "=" << item.second;
            first = false;
        }
    }
    ss << "\n";

    return ss.str();
}

double Consumer::CalculateMarginalUtility(const string &product, double quantity)
{
    if (quantity <= 0.0)
        return 0.0;

    double preference = 1.0;
    auto it = preferences.find(product);
    if (it != preferences.end())
        preference = it->second;

    // MU = preference / (1 + quantity) – diminishing with quantity
    return preference / (1.0 + quantity);
}
