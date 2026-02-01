#pragma once

#include "../utils/Config.h"
#include <cmath>

// Quantity Theory of Money: MV = PQ
// M = Money supply, V = Velocity, P = Price level, Q = Real output
class QuantityTheory
{
public:
    // Calculate implied price level
    static double calculatePriceLevel(double moneySupply,
                                      double velocity,
                                      double realOutput)
    {
        if (realOutput <= 0)
            return 1.0;

        // P = MV / Q
        return (moneySupply * velocity) / realOutput;
    }

    // Calculate implied real output (GDP)
    static double calculateRealOutput(double moneySupply,
                                      double velocity,
                                      double priceLevel)
    {
        if (priceLevel <= 0)
            return 0.0;

        // Q = MV / P
        return (moneySupply * velocity) / priceLevel;
    }

    // Calculate required money supply for target price level
    static double calculateRequiredMoneySupply(double targetPriceLevel,
                                               double velocity,
                                               double realOutput)
    {
        // M = PQ / V
        return (targetPriceLevel * realOutput) / velocity;
    }

    // Calculate inflation from money growth (long-run)
    // Assumes constant velocity and output growth
    static double calculateInflationFromMoneyGrowth(double moneyGrowthRate,
                                                    double outputGrowthRate)
    {
        // π ≈ money growth - output growth (in long run)
        return moneyGrowthRate - outputGrowthRate;
    }

    // Cambridge equation: Md = kPY (money demand)
    static double calculateMoneyDemand(double priceLevel,
                                       double realIncome,
                                       double cambridgeK = 0.25)
    {
        return cambridgeK * priceLevel * realIncome;
    }

    // Calculate velocity of money
    static double calculateVelocity(double nominalGdp, double moneySupply)
    {
        if (moneySupply <= 0)
            return Config::MONEY_VELOCITY;
        return nominalGdp / moneySupply;
    }
};
