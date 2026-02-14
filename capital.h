#pragma once

class capital {
public:
    double rentalRate;
    double efficiency; // how much output per unit of capital

    capital(double rentalRate, double efficiency) : rentalRate(rentalRate), efficiency(efficiency) {}
};