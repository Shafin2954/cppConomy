#pragma once
#include <vector>
#include <algorithm>
#include <map>
#include "consumer.h"
#include "farmer.h"
#include "product.h"

class market {
public:
    double price = 0.0;
    product* prod;
    
    // Aggregate demand curve: p = c - mQ
    struct demandCurve {
        double m; 
        double c;  
    };
    
    // Aggregate supply curve: p = c + mQ
    struct supplyCurve {
        double m;  
        double c;  
    };
    
    demandCurve aggregateDemand;
    supplyCurve aggregateSupply;

    market(product* prod) : prod(prod), aggregateDemand({0, 0}), aggregateSupply({0, 0}) {}

    // Aggregate all consumer demand curves
    // Q = sum(c/m) - sum(1/m) * p
    void calculateAggregateDemand(std::vector<consumer>& consumers) {
        if (consumers.empty()) {
            aggregateDemand = {0, 0};
            return;
        }

        double totalInvM = 0.0; // sum of 1/m
        double cByM= 0.0; // sum of c/m

        for (auto& c : consumers) {
            if (c.dd.find(prod) != c.dd.end()) {
                totalInvM += 1.0 / c.dd[prod].m;
                cByM += c.dd[prod].c / c.dd[prod].m;
            }
        }

        aggregateDemand.m = 1.0 / totalInvM;
        aggregateDemand.c = aggregateDemand.m * cByM;
    }

    // Aggregate all farmer supply curves
    // Q = sum(c/m) + sum(1/m) * p
    void calculateAggregateSupply(std::vector<farmer>& farmers) {
        if (farmers.empty()) {
            aggregateSupply = {0, 0};
            return;
        }

        double totalInvM = 0.0;
        double cByM = 0.0;

        for (auto& f : farmers) {
            if (f.ss.find(prod) != f.ss.end()) { // Check if farmer produces this crop
                totalInvM += 1.0 / f.ss[prod].m;
                cByM += f.ss[prod].c / f.ss[prod].m;
            }
        }

        aggregateSupply.m = 1.0 / totalInvM;
        aggregateSupply.c = aggregateSupply.m * cByM;
    }

    // Find equilibrium price and quantity
    struct equilibrium {
        double price;
        double quantity;
    };

    equilibrium findEquilibrium() {
        // Demand: p = cd - md * Q
        // Supply: p = cs + ms * Q
        // At equilibrium: cd - md * Q = cs + ms * Q
        // Solve for Q: Q = (cd - cs) / (md + ms)
        
        double denominator = aggregateDemand.m + aggregateSupply.m;
        if (denominator < 0.0001) {
            return {0, 0};  // No equilibrium
        }

        double Q = (aggregateDemand.c - aggregateSupply.c) / denominator;
        double P = aggregateDemand.c - (aggregateDemand.m * Q);

        return {P, Q};
    }
};