#pragma once

#include <string>

using namespace std;

// ============================================================================
// Product.h - Represents goods/commodities that are traded in the economy
//
// Maps to: Substitution, Elasticity, CPI, Addiction, PPF
// ============================================================================

enum class ProductType
{
    RICE,       // Agricultural good - inelastic demand
    ICE_CREAM,  // Consumer good - elastic demand
    CURD,       // Alternative to ice cream - substitution effect
    FERTILIZER, // Input for farming
    LABOR,      // Factor of production (special case)
    CLOTH       // Generic manufactured good
};

struct Product
{
    ProductType type;
    string name;
    double price;          // Current market price
    double quantity;       // Available quantity
    bool is_inferior_good; // For income effect demo (cheaper alternative)
    bool is_addictive;     // For addiction logic (vertical demand curve)
    double elasticity;     // Price elasticity of demand (0.0 - 2.0)
    string substitute;     // Name of substitute product (for substitution effect)

    // Constructor
    Product(ProductType t, const string &n, double p, double q)
        : type(t), name(n), price(p), quantity(q),
          is_inferior_good(false), is_addictive(false),
          elasticity(1.0), substitute("")
    {
    }

    // Default constructor
    Product() : type(ProductType::RICE), name(""), price(0), quantity(0),
                is_inferior_good(false), is_addictive(false),
                elasticity(1.0), substitute("")
    {
    }
};
