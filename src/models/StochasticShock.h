#pragma once

#include "../utils/Logger.h"
#include "../utils/Statistics.h"
#include <string>
#include <map>

class Economy;
class Firm;
class Person;

enum class ShockType
{
    PANDEMIC,
    MARKET_CRASH,
    TECH_BOOM,
    NATURAL_DISASTER,
    OIL_SHOCK,
    TRADE_WAR,
    POLICY_CHANGE
};

struct ShockEffect
{
    double gdpImpact;          // Multiplier (0.9 = -10%)
    double unemploymentImpact; // Additive (0.05 = +5%)
    double inflationImpact;    // Additive
    double wealthImpact;       // Multiplier
    double productivityImpact; // Multiplier
    int duration;              // Ticks
};

class StochasticShock
{
public:
    // Trigger a shock by name
    static void triggerShock(Economy &economy,
                             const std::string &type,
                             double severity = 1.0);

    // Get shock effect parameters
    static ShockEffect getShockEffect(ShockType type, double severity);

    // Monte Carlo probability check
    static bool shouldTrigger(double baseProbability)
    {
        return Statistics::Random::getInstance().bernoulli(baseProbability);
    }

private:
    static void applyPandemic(Economy &economy, double severity);
    static void applyMarketCrash(Economy &economy, double severity);
    static void applyTechBoom(Economy &economy, double severity);
    static void applyNaturalDisaster(Economy &economy, double severity);
    static void applyOilShock(Economy &economy, double severity);
};

// Implementation
inline void StochasticShock::triggerShock(Economy &economy,
                                          const std::string &type,
                                          double severity)
{
    severity = std::max(0.1, std::min(2.0, severity)); // Clamp severity

    if (type == "pandemic")
    {
        LOG_ALERT("SHOCK: Pandemic outbreak! Severity: " +
                  std::to_string(static_cast<int>(severity * 100)) + "%");
        applyPandemic(economy, severity);
    }
    else if (type == "market_crash")
    {
        LOG_ALERT("SHOCK: Market crash! Severity: " +
                  std::to_string(static_cast<int>(severity * 100)) + "%");
        applyMarketCrash(economy, severity);
    }
    else if (type == "tech_boom")
    {
        LOG_SUCCESS("SHOCK: Technology boom! Intensity: " +
                    std::to_string(static_cast<int>(severity * 100)) + "%");
        applyTechBoom(economy, severity);
    }
    else if (type == "natural_disaster")
    {
        LOG_ALERT("SHOCK: Natural disaster! Severity: " +
                  std::to_string(static_cast<int>(severity * 100)) + "%");
        applyNaturalDisaster(economy, severity);
    }
    else if (type == "oil_shock")
    {
        LOG_WARNING("SHOCK: Oil price shock! Severity: " +
                    std::to_string(static_cast<int>(severity * 100)) + "%");
        applyOilShock(economy, severity);
    }
    else
    {
        LOG_WARNING("Unknown shock type: " + type);
    }
}

inline ShockEffect StochasticShock::getShockEffect(ShockType type, double severity)
{
    ShockEffect effect = {1.0, 0.0, 0.0, 1.0, 1.0, 12};

    switch (type)
    {
    case ShockType::PANDEMIC:
        effect.gdpImpact = 1.0 - 0.10 * severity;
        effect.unemploymentImpact = 0.05 * severity;
        effect.inflationImpact = -0.01 * severity; // Deflationary
        effect.wealthImpact = 0.95;
        effect.duration = 24;
        break;

    case ShockType::MARKET_CRASH:
        effect.gdpImpact = 1.0 - 0.05 * severity;
        effect.unemploymentImpact = 0.03 * severity;
        effect.wealthImpact = 1.0 - 0.15 * severity;
        effect.duration = 12;
        break;

    case ShockType::TECH_BOOM:
        effect.gdpImpact = 1.0 + 0.03 * severity;
        effect.productivityImpact = 1.0 + 0.05 * severity;
        effect.duration = 24;
        break;

    case ShockType::NATURAL_DISASTER:
        effect.gdpImpact = 1.0 - 0.08 * severity;
        effect.wealthImpact = 1.0 - 0.10 * severity;
        effect.duration = 6;
        break;

    case ShockType::OIL_SHOCK:
        effect.inflationImpact = 0.02 * severity;
        effect.productivityImpact = 1.0 - 0.03 * severity;
        effect.duration = 12;
        break;

    default:
        break;
    }

    return effect;
}

// Forward declaration needed - actual implementation references Economy members
inline void StochasticShock::applyPandemic(Economy &economy, double severity)
{
    // Effects are applied through the economy's tick loop
    // Here we just set up the shock parameters
    auto &rng = Statistics::Random::getInstance();

    // Immediate effects: some firms close, workers lose jobs
    // This would need access to economy internals - simplified here
}

inline void StochasticShock::applyMarketCrash(Economy &economy, double severity)
{
    // Wealth destruction
}

inline void StochasticShock::applyTechBoom(Economy &economy, double severity)
{
    // Productivity boost
}

inline void StochasticShock::applyNaturalDisaster(Economy &economy, double severity)
{
    // Capital destruction
}

inline void StochasticShock::applyOilShock(Economy &economy, double severity)
{
    // Cost push inflation
}
