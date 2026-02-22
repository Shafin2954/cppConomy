#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <cmath>

#include "product.h"

// demand curve: p = c - mQ
struct demandLine
{
    double m;
    double c;
};

class consumer
{
public:
    int id;
    std::string name;
    int ageInDays;
    bool isAlive;
    double savings;
    double expenses;
    double incomePerDay; // Budget per day

    double muPerTk = getMUperTk();
    std::vector<product> needs; // goods needed (n)

    //              ╭ pointer to the same stored products
    std::map<product *, demandLine> dd; // Map product pointer to demand line
    std::map<product *, double> substitutionRatios;
    std::map<product *, double> consumed;

    // dd and consumed are keyed by global product pointers (e.g. &rice).
    // needs stores copies, so &need != &rice. This helper finds the real key by name.
    product *findKey(const std::string &name)
    {
        for (auto &[ptr, line] : dd)
            if (ptr && ptr->name == name)
                return ptr;
        return nullptr;
    }

    consumer(int id, const std::string &name, int ageInYears) : id(id), name(name), ageInDays(ageInYears * 365)
    {
        isAlive = true;
    }

    double getMUperTk()
    {
        double wealth = savings + (incomePerDay * 30); // consider 30 days of income as part of wealth
        if (wealth < 1)
            wealth = 1;      // high MU per Tk
        return 1.0 / wealth; // inverse relationship between wealth and MU per Tk
    }

    double getMarginalUtility(product product)
    { // wtp * muPerTk // wtp = p (Willingness To Pay)
        double wtp = dd[&product].c - (dd[&product].m * consumed[&product]);
        return wtp * muPerTk;
    }

    virtual void pass_day(double gdpPerCapita, const std::map<std::string, double> &prices = {})
    {
        double oldIncome = incomePerDay;

        // Age the consumer
        ageInDays++;

        // Calculate consumption and expenses
        expenses = 0.0;
        for (auto &need : needs)
        {
            // dd and consumed are keyed by global pointers (e.g. &rice), not &need.
            // findKey() matches by product name to get the correct key.
            product *key = findKey(need.name);
            if (!key)
                continue;

            // Calculate how much to consume (uses the correct key into dd)
            double consumeAmount = consumptionRate(need, gdpPerCapita, key);
            consumed[key] += consumeAmount;

            // Use actual market price if provided, otherwise fall back to WTP
            double price;
            auto it = prices.find(need.name);
            if (it != prices.end() && it->second > 0.01)
                price = it->second;
            else
                price = std::max(0.01, dd[key].c - dd[key].m * consumed[key]);

            expenses += price * consumeAmount;

            // Decay previously consumed amount
            consumed[key] -= need.decayRate;
            if (consumed[key] < 0.0)
                consumed[key] = 0.0;
        }

        // Update finances
        savings += incomePerDay - expenses;

        // Update demand curves if income changed
        double incomeChange = incomePerDay - oldIncome;
        if (std::abs(incomeChange) > 0.01)
        {
            updateDemandForIncomeChange(incomeChange);
        }

        // Update MU per Tk based on new wealth
        muPerTk = getMUperTk();

        // Update substitution ratios
        for (auto &need : needs)
        {
            product *key = findKey(need.name);
            if (key)
                substitutionRatios[key] = updateSubRatio(need);
        }
    }

    double consumerSurplus(product product, double marketPrice)
    {
        return .5 * (dd[&product].c - marketPrice) * consumed[&product]; // .5 * c * quantity consumer
    }

    void die()
    {
        isAlive = false;
        needs.clear();
    }
    double updateSubRatio(product product)
    {
        return getMarginalUtility(product) / getMarginalUtility(rice);
    }

    // key must be the global product pointer (from findKey), not a local copy's address
    double consumptionRate(const product &prod, double gdpPerCapita, product *key)
    {
        double wealth = savings + incomePerDay * 365;
        double wealthRatio = wealth / std::max(1.0, gdpPerCapita);

        // Base consumption adjusted by income elasticity
        double baseRate = prod.baseConsumption * std::pow(wealthRatio, prod.eta);

        // Budget constraint: can't spend more than 30% of daily income on one good
        double intercept = (dd.count(key) && dd[key].c > 0.01) ? dd[key].c : 1.0;
        double maxAffordable = (incomePerDay * 0.3) / intercept;

        return std::min(baseRate, maxAffordable);
    }

    // Update demand curve based on price changes (substitution effect)
    void updateDemandForPriceChange(product *prod, double newPrice)
    {
        if (dd.find(prod) != dd.end())
        {
            // Price increase reduces willingness to pay ceiling
            double priceShock = newPrice / std::max(0.1, dd[prod].c);
            if (priceShock > 1.2) // Significant price increase
            {
                dd[prod].c *= 0.95; // Reduce reservation price
            }
        }
    }

    // Shift demand curve based on income change
    void updateDemandForIncomeChange(double incomeChange)
    {
        for (auto &need : needs)
        {
            if (dd.find(&need) != dd.end())
            {
                // Normal goods: demand increases with income
                if (need.eta > 0)
                {
                    dd[&need].c += incomeChange * 0.05 * need.eta;
                }
                // Inferior goods: demand decreases with income
                else if (need.eta < 0)
                {
                    dd[&need].c += incomeChange * 0.02 * need.eta;
                    dd[&need].c = std::max(0.5, dd[&need].c); // Floor
                }
            }
        }
    }

    std::string getStyledDetails() const
    {
        using namespace styledTerminal;
        std::stringstream ss;

        ss << Header("CONSUMER: " + name) << "\n";
        ss << KeyValue("Age", std::to_string(ageInDays / 365) + " years") << "\n";
        ss << KeyValue("Savings", "Tk " + std::to_string(twoDecimal(savings))) << "\n";
        ss << KeyValue("Daily Income", "Tk " + std::to_string(twoDecimal(incomePerDay))) << "\n";
        ss << KeyValue("MU per Tk", std::to_string(twoDecimal(muPerTk))) << "\n\n";

        ss << Styled("CONSUMPTION:\n", Theme::Primary);
        for (const auto &need : needs)
        {
            if (consumed.find(const_cast<product *>(&need)) != consumed.end())
            {
                ss << "  • " << need.name << ": "
                   << twoDecimal(consumed.at(const_cast<product *>(&need))) << " units\n";
            }
        }

        return ss.str();
    }
};