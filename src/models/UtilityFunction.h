#pragma once

#include <cmath>

// Utility functions for consumer behavior
class UtilityFunction
{
public:
    // Marginal Utility: MU = α * Q^(-β)
    // Diminishing marginal utility
    static double marginalUtility(double quantity,
                                  double alpha = 1.0,
                                  double beta = 0.5)
    {
        if (quantity <= 0)
            return alpha;
        return alpha * std::pow(quantity, -beta);
    }

    // Total Utility: U = α * Q^(1-β) / (1-β) for β ≠ 1
    // Or U = α * ln(Q) for β = 1
    static double totalUtility(double quantity,
                               double alpha = 1.0,
                               double beta = 0.5)
    {
        if (quantity <= 0)
            return 0.0;

        if (std::abs(beta - 1.0) < 0.001)
        {
            return alpha * std::log(quantity);
        }
        return alpha * std::pow(quantity, 1.0 - beta) / (1.0 - beta);
    }

    // Cobb-Douglas utility: U = X^α * Y^(1-α)
    static double cobbDouglasUtility(double x, double y, double alpha = 0.5)
    {
        if (x <= 0 || y <= 0)
            return 0.0;
        return std::pow(x, alpha) * std::pow(y, 1.0 - alpha);
    }

    // Optimal consumption bundle (Cobb-Douglas)
    // Given budget M, prices Px and Py
    static std::pair<double, double> optimalBundle(double budget,
                                                   double priceX,
                                                   double priceY,
                                                   double alpha = 0.5)
    {
        if (priceX <= 0 || priceY <= 0 || budget <= 0)
        {
            return {0.0, 0.0};
        }

        // x* = αM / Px
        // y* = (1-α)M / Py
        double x = alpha * budget / priceX;
        double y = (1.0 - alpha) * budget / priceY;

        return {x, y};
    }

    // Demand function: Qd = A * P^(-ε) * I^η
    // P = price, I = income, ε = price elasticity, η = income elasticity
    static double demandFunction(double price,
                                 double income,
                                 double A = 100.0,
                                 double priceElasticity = 1.0,
                                 double incomeElasticity = 1.0)
    {
        if (price <= 0)
            return 0.0;
        return A * std::pow(price, -priceElasticity) *
               std::pow(std::max(1.0, income), incomeElasticity);
    }

    // Indirect utility function V(P, M) = M / P^α
    static double indirectUtility(double income, double priceIndex, double alpha = 1.0)
    {
        if (priceIndex <= 0)
            return 0.0;
        return income / std::pow(priceIndex, alpha);
    }
};
