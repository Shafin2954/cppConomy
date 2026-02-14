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
    { // renew mu by decay rate for all products, consume products
        for (auto &need : needs)
        {
            substitutionRatios[&need] = getMarginalUtility(need) / getMarginalUtility(rice);
            expenses += (dd[&need].c - dd[&need].m * consumed[&need]) * muPerDollar; // approximate daily spending based on WTP and MU per dollar
        }
        savings += incomePerDay - expenses; // receive daily income
        expenses = 0.0;
        ageInDays++;
        for (auto &product : needs)
        {
            consumed[&product] += consumptionRate(product, gdpPerCapita); // consume based on consumption rate and GDP per capita
            consumed[&product] -= product.decayRate; // fade consumed amount by decay rate
            if (consumed[&product] < 0.0)
                consumed[&product] = 0.0;
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
        
        double consumptionRate = product.baseConsumption * pow((wealth / gdpPerCapita), product.eta); // Increase consumption with GDP per capita
        std::cout << "DEBUG: " << name << " consumption rate for " << product.name << ": " << consumptionRate << " (Wealth: " << wealth << ", GDP per Capita: " << gdpPerCapita << ")\n";
        return consumptionRate; 
    }
};