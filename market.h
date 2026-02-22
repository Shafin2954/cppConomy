#pragma once
#include <vector>
#include <algorithm>
#include <map>
#include "consumer.h"
#include "farmer.h"
#include "laborer.h"
#include "firm.h"
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

    // Aggregate demand curves from consumers, farmers, and laborers
    // Q = sum(c/m) - sum(1/m) * p
    void calculateAggregateDemand(const std::vector<consumer> &consumers,
                                  const std::vector<farmer> &farmers,
                                  const std::vector<laborer> &laborers)
    {
        double totalInvM = 0.0; // sum of 1/m
        double cByM = 0.0;      // sum of c/m

        auto collectDemand = [&](const consumer &agent)
        {
            auto it = agent.dd.find(prod);
            if (it == agent.dd.end())
                return;

            double slope = it->second.m;
            if (slope <= 0.000001)
                return;

            totalInvM += 1.0 / slope;
            cByM += it->second.c / slope;
        };

        for (const auto &c : consumers)
            collectDemand(c);

        for (const auto &f : farmers)
            collectDemand(static_cast<const consumer &>(f));

        for (const auto &l : laborers)
            collectDemand(static_cast<const consumer &>(l));

        if (totalInvM <= 0.000001)
        {
            aggregateDemand = {0, 0};
        }
        else
        {
            aggregateDemand.m = 1.0 / totalInvM;
            aggregateDemand.c = aggregateDemand.m * cByM;
        }
    }

    // Aggregate all farmer supply curves
    // Q = sum(c/m) + sum(1/m) * p  (horizontal summation)
    void calculateAggregateSupply(const std::vector<farmer> &farmers,
                                   const std::vector<firm>   &firms = {})
    {
        double totalInvM = 0.0;
        double cByM      = 0.0;

        // ── Farmer supply ─────────────────────────────────────────────────
        for (const auto &f : farmers)
        {
            auto it = f.ss.find(prod);
            if (it == f.ss.end()) continue;
            double slope = it->second.m;
            if (slope <= 0.000001) continue;
            totalInvM += 1.0 / slope;
            cByM      += it->second.c / slope;
        }

        // ── Firm supply ───────────────────────────────────────────────────
        // Each firm that makes this product contributes a linearised supply
        // curve derived from its current MC.
        // Scale: 1 production-function unit ≈ 80 market units (calibration).
        static constexpr double OUTPUT_SCALE = 80.0;

        for (const auto &fi : firms)
        {
            // Does this firm make this product?
            bool makes = false;
            for (const auto &p : fi.products)
                if (p.name == prod->name) { makes = true; break; }
            if (!makes) continue;
            if (fi.currentOutput < 0.001) continue;

            // Effective per-market-unit MC
            double effMC = fi.marginalCost / OUTPUT_SCALE;
            if (effMC < 0.5) effMC = fi.wage / OUTPUT_SCALE;  // fallback

            // Upward-sloping supply: P = intercept + slope*Q
            // Intercept = effMC*0.5 (sell once price covers half of MC — overhead covered by revenue)
            // Slope      = effMC / (fi.currentOutput * OUTPUT_SCALE)
            double intercept = effMC * 0.5;
            double slope     = effMC / (fi.currentOutput * OUTPUT_SCALE);
            if (slope <= 0.000001) continue;

            totalInvM += 1.0 / slope;
            cByM      += intercept / slope;
        }

        if (totalInvM <= 0.000001)
        {
            aggregateSupply = {0, 0};
        }
        else
        {
            aggregateSupply.m = 1.0 / totalInvM;
            aggregateSupply.c = aggregateSupply.m * cByM;
        }
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
    std::vector<double> priceHistory;  // Price recorded each pass_day

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

        // Record history (keep last 30 days)
        priceHistory.push_back(price);
        if (priceHistory.size() > 30)
            priceHistory.erase(priceHistory.begin());
    }

    std::string getStyledDetails() const
    {
        using namespace styledTerminal;
        std::stringstream ss;

        ss << Header("MARKET: " + prod->name) << "\n";
        ss << KeyValue("Current Price", "Tk " + std::to_string(twoDecimal(price))) << "\n";
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

void clearMarket(const std::vector<consumer> &allConsumers,
                 const std::vector<farmer> &allFarmers,
                 const std::vector<laborer> &allLaborers)
{
    calculateAggregateDemand(allConsumers, allFarmers, allLaborers);
    calculateAggregateSupply(allFarmers);
    
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
    ss << KeyValue("Equilibrium Price", "Tk " + std::to_string(twoDecimal(eq.price))) << "\n";
    ss << KeyValue("Equilibrium Quantity", std::to_string(twoDecimal(eq.quantity)) + " units") << "\n";
    ss << KeyValue("Market Value", "Tk " + std::to_string(twoDecimal(eq.price * eq.quantity))) << "\n";
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