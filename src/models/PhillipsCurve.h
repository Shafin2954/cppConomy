#pragma once

#include "../utils/Config.h"
#include <cmath>

// Phillips Curve: π = π^e - β(u - u*)
// Inverse relationship between inflation and unemployment
class PhillipsCurve
{
public:
    // Calculate inflation given unemployment
    static double calculate(double unemployment,
                            double naturalUnemployment,
                            double expectedInflation)
    {
        double beta = Config::PHILLIPS_BETA;
        double unemploymentGap = unemployment - naturalUnemployment;

        // π = π^e - β(u - u*)
        double inflation = expectedInflation - beta * unemploymentGap;

        // Inflation can't go below some floor (deflation limit)
        return std::max(-0.05, inflation);
    }

    // Calculate expected unemployment given target inflation
    static double calculateUnemployment(double targetInflation,
                                        double naturalUnemployment,
                                        double expectedInflation)
    {
        double beta = Config::PHILLIPS_BETA;

        // Rearranged: u = u* + (π^e - π) / β
        double unemployment = naturalUnemployment +
                              (expectedInflation - targetInflation) / beta;

        return std::max(0.0, unemployment);
    }

    // Adaptive expectations: update expected inflation based on actual
    static double updateExpectations(double expectedInflation,
                                     double actualInflation,
                                     double adaptationSpeed = 0.3)
    {
        return expectedInflation + adaptationSpeed * (actualInflation - expectedInflation);
    }
};
