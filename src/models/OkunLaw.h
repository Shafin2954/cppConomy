#pragma once

#include "../utils/Config.h"
#include <cmath>

// Okun's Law: (Y - Y*) / Y* = -γ(u - u*)
// Relates GDP gap to unemployment gap
class OkunLaw
{
public:
    // Calculate potential GDP given actual GDP and unemployment
    static double calculatePotentialGdp(double actualGdp,
                                        double unemployment,
                                        double naturalUnemployment)
    {
        double gamma = Config::OKUN_GAMMA;
        double unemploymentGap = unemployment - naturalUnemployment;

        // Rearranged: Y* = Y / (1 - γ(u - u*))
        double denominator = 1.0 - gamma * unemploymentGap;
        if (std::abs(denominator) < 0.01)
        {
            denominator = 0.01 * (denominator >= 0 ? 1 : -1);
        }

        return actualGdp / denominator;
    }

    // Calculate GDP gap percentage
    static double calculateGdpGap(double actualGdp, double potentialGdp)
    {
        if (potentialGdp <= 0)
            return 0.0;
        return (actualGdp - potentialGdp) / potentialGdp;
    }

    // Calculate implied unemployment from GDP gap
    static double calculateUnemployment(double gdpGap,
                                        double naturalUnemployment)
    {
        double gamma = Config::OKUN_GAMMA;

        // Rearranged: u = u* - (Y - Y*) / (γ * Y*)
        double unemployment = naturalUnemployment - gdpGap / gamma;

        return std::max(0.0, std::min(1.0, unemployment));
    }

    // Estimate GDP loss from unemployment above natural rate
    static double calculateGdpLoss(double unemployment,
                                   double naturalUnemployment,
                                   double potentialGdp)
    {
        double gamma = Config::OKUN_GAMMA;
        double unemploymentGap = unemployment - naturalUnemployment;

        // GDP loss = γ * (u - u*) * Y*
        return gamma * unemploymentGap * potentialGdp;
    }
};
