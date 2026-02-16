#pragma once
#include <vector>
#include <algorithm>
#include <map>
#include "consumer.h"
#include "farmer.h"
#include "product.h"

class market
{
public:
    double price = 0.0;
    product *prod;

    // Aggregate demand curve: p = c - mQ
    struct demandCurve
    {
        double m;
        double c;
    };

    // Aggregate supply curve: p = c + mQ
    struct supplyCurve
    {
        double m;
        double c;
    };

    demandCurve aggregateDemand;
    supplyCurve aggregateSupply;

    market(product *prod) : prod(prod), aggregateDemand({0, 0}), aggregateSupply({0, 0}) {}

    // Aggregate all consumer demand curves
    // Q = sum(c/m) - sum(1/m) * p
    void calculateAggregateDemand(std::vector<consumer> &consumers)
    {
        if (consumers.empty())
        {
            aggregateDemand = {0, 0};
            return;
        }

        double totalInvM = 0.0; // sum of 1/m
        double cByM = 0.0;      // sum of c/m

        for (auto &c : consumers)
        {
            if (c.dd.find(prod) != c.dd.end())
            {
                totalInvM += 1.0 / c.dd[prod].m;
                cByM += c.dd[prod].c / c.dd[prod].m;
            }
        }

        aggregateDemand.m = 1.0 / totalInvM;
        aggregateDemand.c = aggregateDemand.m * cByM;
    }

    // Aggregate all farmer supply curves
    // Q = sum(c/m) + sum(1/m) * p
    void calculateAggregateSupply(std::vector<farmer> &farmers)
    {
        if (farmers.empty())
        {
            aggregateSupply = {0, 0};
            return;
        }

        double totalInvM = 0.0;
        double cByM = 0.0;

        for (auto &f : farmers)
        {
            if (f.ss.find(prod) != f.ss.end())
            { // Check if farmer produces this crop
                totalInvM += 1.0 / f.ss[prod].m;
                cByM += f.ss[prod].c / f.ss[prod].m;
            }
        }

        aggregateSupply.m = 1.0 / totalInvM;
        aggregateSupply.c = aggregateSupply.m * cByM;
    }

    // Find equilibrium price and quantity
    struct equilibrium
    {
        double price;
        double quantity;
    };

    // Add to market class:

    double excessDemand = 0.0;         // Track disequilibrium
    double priceAdjustmentSpeed = 0.1; // How fast prices adjust

    // Replace findEquilibrium with dynamic adjustment
    equilibrium findEquilibrium()
    {
        double denominator = aggregateDemand.m + aggregateSupply.m;
        if (denominator < 0.0001)
            return {price, 0.0};

        double Q = (aggregateDemand.c - aggregateSupply.c) / denominator;
        double P = aggregateDemand.c - (aggregateDemand.m * Q);

        // Ensure non-negative
        Q = std::max(0.0, Q);
        P = std::max(0.1, P);

        excessDemand = getQuantityDemanded(price) - getQuantitySupplied(price);

        return {P, Q};
    }

    double getQuantityDemanded(double p)
    {
        if (aggregateDemand.m < 0.0001)
            return 0.0;
        double qd = (aggregateDemand.c - p) / aggregateDemand.m;
        return std::max(0.0, qd);
    }

    double getQuantitySupplied(double p)
    {
        if (aggregateSupply.m < 0.0001)
            return 0.0;
        double qs = (p - aggregateSupply.c) / aggregateSupply.m;
        return std::max(0.0, qs);
    }

    void adjustPrice()
    {
        // Walrasian tâtonnement: price rises with excess demand
        double priceChange = priceAdjustmentSpeed * excessDemand;
        price += priceChange;

        // Floor and ceiling
        price = std::max(0.5, std::min(1000.0, price));
    }

    std::string getStyledStatus() const
    {
        using namespace styledTerminal;
        std::stringstream ss;

        ss << Header("MARKET: " + prod->name) << "\n";
        ss << KeyValue("Current Price", "$" + std::to_string(twoDecimal(price))) << "\n";
        ss << KeyValue("Excess Demand", std::to_string(twoDecimal(excessDemand))) << "\n\n";

        ss << Styled("DEMAND CURVE:\n", Theme::Info);
        ss << "  P = " << twoDecimal(aggregateDemand.c) << " - "
           << twoDecimal(aggregateDemand.m) << "Q\n\n";

        ss << Styled("SUPPLY CURVE:\n", Theme::Success);
        ss << "  P = " << twoDecimal(aggregateSupply.c) << " + "
           << twoDecimal(aggregateSupply.m) << "Q\n";

        return ss.str();
    }

    // Add to market class:
double quantityTraded = 0.0;  // Track actual trades
double revenue = 0.0;         // Price × Quantity traded

void clearMarket(std::vector<consumer>& allConsumers, std::vector<farmer>& suppliers)
{
    calculateAggregateDemand(allConsumers);
    calculateAggregateSupply(suppliers);
    
    auto eq = findEquilibrium();
    
    // Record actual transaction
    quantityTraded = eq.quantity;
    revenue = eq.price * eq.quantity;
    price = eq.price;
    
    // Distribute goods to consumers (simplified)
    // In reality, you'd allocate based on willingness to pay
}

std::string getStyledEquilibrium()
{
    using namespace styledTerminal;
    std::stringstream ss;
    
    auto eq = findEquilibrium();
    
    ss << Header("MARKET EQUILIBRIUM: " + prod->name) << "\n\n";
    ss << KeyValue("Equilibrium Price", "$" + std::to_string(twoDecimal(eq.price))) << "\n";
    ss << KeyValue("Equilibrium Quantity", std::to_string(twoDecimal(eq.quantity)) + " units") << "\n";
    ss << KeyValue("Market Value", "$" + std::to_string(twoDecimal(eq.price * eq.quantity))) << "\n";
    ss << KeyValue("Excess Demand", std::to_string(twoDecimal(excessDemand))) << "\n\n";
    
    ss << Styled("Demand: P = ", Theme::Info) 
       << twoDecimal(aggregateDemand.c) << " - " 
       << twoDecimal(aggregateDemand.m) << "Q\n";
    ss << Styled("Supply: P = ", Theme::Success) 
       << twoDecimal(aggregateSupply.c) << " + " 
       << twoDecimal(aggregateSupply.m) << "Q\n";
    
    return ss.str();
}
};