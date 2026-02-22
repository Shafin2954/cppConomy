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

    double getCropMetricByName(const std::map<product *, double> &metric, const std::string &cropName) const
    {
        for (const auto &[cropPtr, value] : metric)
        {
            if (cropPtr && cropPtr->name == cropName)
                return value;
        }
        return 0.0;
    }

    void pass_day(double perCapita, const std::map<std::string, double> &prices = {}) override
{
    consumer::pass_day(perCapita, prices);

    // Weather varies with some persistence
    double weatherChange = ((double)rand() / RAND_MAX - 0.5) * 0.3;
    weather = std::max(0.2, std::min(0.95, weather + weatherChange));

    // IMPORTANT: iterate ss map (correct original product* keys), NOT crops vector.
    // crops vector stores copies, so &crop != &rice — using &crop as key silently
    // creates a new zero-entry and the real maxOutput[&rice] never updates.
    for (auto& [cropPtr, supply] : ss)
    {
        double weatherBonus  = (weather > 0.6) ? (weather - 0.6) * 20.0 : 0.0;
        maxOutput[cropPtr]  += growthRate[cropPtr] + weatherBonus;

        double weatherPenalty = (weather < 0.5) ? (0.5 - weather) * 50.0 : 0.0;
        maxOutput[cropPtr]  -= decay[cropPtr] + weatherPenalty;

        if (maxOutput[cropPtr] < 0.0) maxOutput[cropPtr] = 0.0;

        updateSupplyCurve(cropPtr);
    }
}

void updateSupplyCurve(product* crop)
{
    if (ss.find(crop) != ss.end())
    {
        // Tech improvement lowers marginal cost (shifts supply right/down)
        double techEffect = techLevel * 2.0;
        ss[crop].c = std::max(1.0, ss[crop].c - techEffect * 0.1);
        
        // Bad weather increases marginal cost (shifts supply left/up)
        double weatherEffect = (1.0 - weather) * 3.0;
        ss[crop].c += weatherEffect;
        
        // Tax increases cost
        ss[crop].c += tax * 5.0;
        
        // Land scarcity increases slope (harder to produce more)
        ss[crop].m = 0.1 + (100.0 / std::max(1.0, land)) * 0.02;
    }
}

double calculateSupply(product* crop, double marketPrice)
{
    if (ss.find(crop) == ss.end()) return 0.0;
    
    // Effective marginal cost includes all factors
    double effectiveMC = ss[crop].c;
    
    // Adjust slope for weather difficulty
    double effectiveSlope = ss[crop].m * (2.0 - weather);  // Bad weather steepens
    
    // Won't produce if price below MC
    if (marketPrice <= effectiveMC) return 0.0;
    
    // Q = (P - MC) / m
    double quantity = (marketPrice - effectiveMC) / effectiveSlope;
    
    // Physical capacity constraint
    quantity = std::min(quantity, maxOutput[crop]);
    
    return std::max(0.0, quantity);
}

std::string getStyledDetails() const
{
    using namespace styledTerminal;
    std::stringstream ss;
    
    ss << Header("FARMER: " + name) << "\n";
    ss << KeyValue("Age", std::to_string(ageInDays / 365) + " years") << "\n";
    ss << KeyValue("Land", std::to_string(twoDecimal(land)) + " acres") << "\n";
    ss << KeyValue("Tech Level", std::to_string(twoDecimal(techLevel * 100)) + "%") << "\n";
    ss << KeyValue("Weather", std::to_string(twoDecimal(weather * 100)) + "%") << "\n";
    ss << KeyValue("Tax Rate", std::to_string(twoDecimal(tax * 100)) + "%") << "\n\n";
    
    ss << Styled("CROPS:\n", Theme::Primary);
    for (const auto& crop : crops)
    {
        double max = getCropMetricByName(maxOutput, crop.name);
        ss << "  • " << crop.name 
           << " (Max: " << twoDecimal(max) << " units)\n";
    }
    
    return ss.str();
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
