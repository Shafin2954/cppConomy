#pragma once

namespace Config
{
    // Simulation
    constexpr int INITIAL_POPULATION = 1000;
    constexpr int INITIAL_FIRMS = 50;
    constexpr int TICKS_PER_YEAR = 12; // Monthly ticks

    // Economic
    constexpr double INITIAL_MONEY_SUPPLY = 1e12;
    constexpr double NATURAL_UNEMPLOYMENT = 0.04;
    constexpr double TARGET_INFLATION = 0.02;
    constexpr double INITIAL_WAGE = 50000.0;
    constexpr double INITIAL_PRICE = 100.0;

    // Behavioral
    constexpr double SAVINGS_RATE_YOUNG = -0.05; // Borrow
    constexpr double SAVINGS_RATE_MIDDLE = 0.15; // Save
    constexpr double SAVINGS_RATE_OLD = -0.08;   // Dissave

    // Age brackets
    constexpr int YOUNG_AGE_MAX = 30;
    constexpr int MIDDLE_AGE_MAX = 60;
    constexpr int MAX_AGE = 100;
    constexpr int WORKING_AGE_MIN = 18;
    constexpr int RETIREMENT_AGE = 65;

    // Stochastic
    constexpr double SHOCK_PROBABILITY = 0.02;
    constexpr int RANDOM_SEED = 42; // For reproducibility

    // Phillips Curve parameters
    constexpr double PHILLIPS_BETA = 0.5;
    constexpr double EXPECTED_INFLATION = 0.02;

    // Okun's Law coefficient
    constexpr double OKUN_GAMMA = 2.5;

    // Quantity Theory velocity
    constexpr double MONEY_VELOCITY = 4.0;

    // Production function (Cobb-Douglas)
    constexpr double CAPITAL_SHARE = 0.3; // alpha
    constexpr double TFP = 1.0;           // Total Factor Productivity

    // GUI update interval (ms)
    constexpr int GUI_UPDATE_INTERVAL = 100;
    constexpr int SIMULATION_TICK_INTERVAL = 500;
}
