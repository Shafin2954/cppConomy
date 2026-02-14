#pragma once
#include <map>
#include <random>
#include "consumer.h"
#include "product.h"

// supply curve: p = c + mQ
struct supplyLine {
    double m;
    double c;
};

class farmer : public consumer {
public:
    double land;
    double techLevel;
    double weather; // 0 - 1 random number

    double tax;

    std::vector<product> crops; // crops produced (n)
    std::map<product*, supplyLine> ss;
    std::map<product*, double> growthRate;
    std::map<product*, double> decay;
    std::map<product*, double> maxOutput;

    farmer(int id, const std::string& name, int age, double land, double techLevel) : consumer(id, name, age), land(land), techLevel(techLevel) {

    }
    void addCrop(product* prod, supplyLine supply, double growth, double decayRate, double initialMax) {
        crops.push_back(*prod);
        ss[prod] = supply;
        growthRate[prod] = growth;
        decay[prod] = decayRate;
        maxOutput[prod] = initialMax;
    }

    void upgradeTech(double newTechLevel) {
        techLevel = newTechLevel;
    }
    void pass_day(double perCapita) override { // override consumer's pass_day to include weather effect and output
        consumer::pass_day(perCapita); // age and consume products

        // Weather effect (randomly adjust weather each day)
        weather = ((double)rand() / RAND_MAX); // random value between 0 and 1

        for (auto crop : crops) {
            // Adjust max output based on growth and decay
            maxOutput[&crop] += growthRate[&crop]; // grow the crop
            maxOutput[&crop] -= decay[&crop] * weather; // decay based on weather

            if (maxOutput[&crop] < 0.0) maxOutput[&crop] = 0.0; // cap at 0
        }
    } 

    double calculateCropOutput(product* crop) {
        // 1. Base Potential: Land * Growth Rate
        // If you have 10 acres and rice grows at 50 units/acre, potential is 500.
        double basePotential = land * growthRate[crop];

        // 2. Weather Impact (The "Shock")
        // Weather is 0.0 to 1.0. 
        // Bad weather (0.2) destroys crops. Good weather (0.8+) boosts slightly.
        // Formula: We map 0.5 to "Normal". <0.5 is loss, >0.5 is bonus.
        double weatherEffect = 0.5 + (weather * 0.8); // Range: 0.5 to 1.3x

        // 3. Tech Level (The Multiplier)
        // Tech 1.0 = Normal. Tech 2.0 = Double Yield (Fertilizer/Tractors).
        // Diminishing returns apply (sqrt helps flatten extreme tech).
        double techMultiplier = 1.0 + std::sqrt(techLevel);

        // 4. Calculate Gross Harvest
        double grossHarvest = basePotential * weatherEffect * techMultiplier;

        // 5. Cap at Maximum Output
        // Even with perfect weather, land has a physical limit per acre.
        double physicalLimit = land * maxOutput[crop]; 
        double actualHarvest = std::min(grossHarvest, physicalLimit);

        // 6. Subtract Decay & Tax
        // Decay happens *during* the season (pests, rot).
        // Tax is taken by the government/landlord.
        double netHarvest = actualHarvest * (1.0 - decay[crop]) * (1.0 - tax);

        return std::max(0.0, netHarvest);
    }
    double calculateSupply(product* crop, double marketPrice){
        double effectiveMC = ss[crop].c + tax - (techLevel * 2.0);
        
        // 2. ADJUST THE SLOPE (Changes in Difficulty/Nature)
        // Bad weather makes the slope steeper (harder to produce more)
        double effectiveSlope = ss[crop].m * weather;

        // 3. CHECK PROFITABILITY
        // If Market Price < Cost to produce 1st unit, produce NOTHING.
        if (marketPrice <= effectiveMC) {
            return 0.0;
        }

        // 4. CALCULATE Q (Inverting the formula P = MC + mQ)
        // Q = (P - MC) / m
        double quantity = (marketPrice - effectiveMC) / effectiveSlope;

        // 5. CAP AT MAX CAPACITY
        if (quantity > maxOutput[crop]) {
            quantity = maxOutput[crop];
        }

        return quantity;
    };

    double getRealisticLandSize() {
        // Random number generator
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        
        double p = dis(gen); // Roll a dice (0.0 to 1.0)

        // 1. Marginal Farmers (45%) -> 0.05 to 0.49 acres
        if (p < 0.45) {
            return 0.05 + (dis(gen) * 0.44); 
        }
        // 2. Small Farmers (45%) -> 0.50 to 2.49 acres
        else if (p < 0.90) {
            return 0.50 + (dis(gen) * 1.99);
        }
        // 3. Medium Farmers (9%) -> 2.50 to 7.49 acres
        else if (p < 0.99) {
            return 2.50 + (dis(gen) * 4.99);
        }
        // 4. Large Farmers (Top 1%) -> 7.50 to 15.0 acres
        else {
            return 7.50 + (dis(gen) * 7.50);
        }
    }
};