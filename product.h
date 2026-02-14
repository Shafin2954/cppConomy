#pragma once
#include <string>

struct product {
    std::string name;
    double decayRate;       // How fast it spoils
    double eta;             // Income Elasticity (Rich vs Poor behavior)
    double baseConsumption; // Average person's daily need
    double growthRate;      // NEW: Units produced per acre of land (0.0 if not a crop)
};

// ==========================================
//           NON-FARM GOODS (Growth = 0)
// ==========================================

// Transport
inline product localBus = {"Local Bus", 1.0, -0.6, 2.0, 0.0};

// Manufacturing
inline product cloth = {"Cloth", 0.01, 0.8, 6.0, 0.0};
inline product phone = {"Phone", 0.001, 1.4, 0.8, 0.0};
inline product computer = {"Computer", 0.001, 2.2, 0.1, 0.0};
inline product steel = {"Steel", 0.0001, 1.9, 50.0, 0.0};
inline product car = {"Car", 0.0001, 4.0, 0.002, 0.0};


// Rice (Paddy): ~40-50 Mon per acre -> ~1600 kg
// This means 1 acre feeds ~3,200 "person-days" (enough for 1 family for 2 years).
inline product rice = {"Rice", 0.01, 0.15, 0.5, 1600.0}; 

// Potato: Yields are massive by weight (~8-10 tons/acre).
// ~8000 kg per acre.
inline product potato = {"Potato", 0.05, -0.2, 0.2, 8000.0}; 

// Banana: Very dense biomass. ~12 tons/acre.
// ~12000 kg per acre.
inline product banana = {"Banana", 0.2, 0.5, 0.1, 12000.0}; 

// Corn (Maize): Higher yield than rice. ~2500 kg/acre.
inline product corn = {"Corn", 0.01, 0.0, 0.05, 2500.0}; 

// Jute: Fiber is lighter than food grains. ~800 kg/acre.
inline product jute = {"Jute", 0.001, 1.0, 0.0, 800.0};