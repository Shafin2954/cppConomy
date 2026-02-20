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

    double muPerDollar = getMUperDollar();
    std::vector<product> needs; // goods needed (n)

    //              ╭ pointer to the same stored products
    std::map<product *, demandLine> dd; // Map product pointer to demand line
    std::map<product *, double> substitutionRatios;
    std::map<product *, double> consumed;

    consumer(int id, const std::string &name, int ageInYears) : id(id), name(name), ageInDays(ageInYears * 365)
    {
        isAlive = true;
    }

    double getMUperDollar()
    {
        double wealth = savings + (incomePerDay * 30); // consider 30 days of income as part of wealth
        if (wealth < 1)
            wealth = 1;      // high MU per dollar
        return 1.0 / wealth; // inverse relationship between wealth and MU per dollar
    }

    double getMarginalUtility(product product)
    { // wtp * muPerDollar // wtp = p (Willingness To Pay)
        double wtp = dd[&product].c - (dd[&product].m * consumed[&product]);
        return wtp * muPerDollar;
    }

    virtual void pass_day(double gdpPerCapita)
    {
        double oldIncome = incomePerDay;

        // Age the consumer
        ageInDays++;

        // Calculate consumption and expenses
        expenses = 0.0;
        for (auto &need : needs)
        {
            // Calculate how much to consume
            double consumeAmount = consumptionRate(need, gdpPerCapita);
            consumed[&need] += consumeAmount;

            // Calculate spending (WTP at current consumption level)
            double wtp = dd[&need].c - (dd[&need].m * consumed[&need]);
            expenses += wtp * consumeAmount;

            // Decay previously consumed amount
            consumed[&need] -= need.decayRate;
            if (consumed[&need] < 0.0)
                consumed[&need] = 0.0;
        }

        // Update finances
        savings += incomePerDay - expenses;

        // Update demand curves if income changed
        double incomeChange = incomePerDay - oldIncome;
        if (std::abs(incomeChange) > 0.01)
        {
            updateDemandForIncomeChange(incomeChange);
        }

        // Update MU per dollar based on new wealth
        muPerDollar = getMUperDollar();

        // Update substitution ratios
        for (auto &need : needs)
        {
            substitutionRatios[&need] = updateSubRatio(need);
        }
    }

    double consumerSurplus(product product, double marketPrice)
    {
        return .5 * dd[&product].c - marketPrice * consumed[&product]; // .5 * c * quantity consumer
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

    double consumptionRate(product product, double gdpPerCapita)
    {
        double wealth = savings + incomePerDay * 365;
        double wealthRatio = wealth / std::max(1.0, gdpPerCapita);

        // Base consumption adjusted by income elasticity
        double baseRate = product.baseConsumption * std::pow(wealthRatio, product.eta);

        // Budget constraint: can't consume more than you can afford
        double maxAffordable = (incomePerDay * 0.3) / std::max(0.1, dd[&product].c);

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
        ss << KeyValue("Savings", "$" + std::to_string(twoDecimal(savings))) << "\n";
        ss << KeyValue("Daily Income", "$" + std::to_string(twoDecimal(incomePerDay))) << "\n";
        ss << KeyValue("MU per Dollar", std::to_string(twoDecimal(muPerDollar))) << "\n\n";

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