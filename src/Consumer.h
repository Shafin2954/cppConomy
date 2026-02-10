#pragma once

#include <string>
#include <map>
#include <vector>
#include "Product.h"

// ============================================================================
// Consumer.h - Base class for all economic agents
//
// Maps to: Utility, Indifference Curves, Budget Line, Literacy, Mortality,
//          Consumer Surplus, Income Effect, Marginal Utility, Tax effects,
//          Demand Curves, Elasticity, Substitution Effects
// ============================================================================

struct DemandPoint
{
    double price;
    double quantity;
};

class Consumer
{
protected:
    int id;
    std::string name;
    int age;               // For mortality/child labor
    bool is_alive;         // For natural disasters
    double literacy_score; // 0.0 - 1.0: affects income potential

    // ========== Budget Constraint (Budget Line) ==========
    double wallet;         // Current disposable income
    double savings;        // Accumulated wealth
    double monthly_income; // Regular income (from wages, etc.)

    // ========== Utility & Preferences (Indifference Curves) ==========
    double total_utility;   // Overall happiness/satisfaction
    double addiction_level; // 0.0 - 1.0: how addicted to certain goods

    // Preferences for different products (how much they value each)
    std::map<std::string, double> preferences; // product_name -> preference_weight

    // Goods owned (for calculating consumer surplus)
    std::map<std::string, double> goods_owned; // product_name -> quantity

    // Taxation
    double income_tax_paid; // Track taxes paid this period

    // ========== Consumer Demand Properties ==========
    // Demand curves for each product: price -> quantity demanded
    std::map<std::string, std::vector<DemandPoint>> demand_curves; // product -> demand curve points

    // Quantity demanded at current market prices
    std::map<std::string, double> quantity_demanded; // product_name -> quantity at current price

    // Substitution elasticity between products (how easily to switch)
    std::map<std::pair<std::string, std::string>, double> substitution_ratios; // (product1, product2) -> elasticity

    // Willingness to pay (max price willing to pay for each product)
    std::map<std::string, double> willingness_to_pay; // product_name -> max_price

public:
    // ========== Constructors & Destructors ==========
    Consumer(int id, const std::string &name, double initial_income);
    virtual ~Consumer() = default;

    // ========== Getters ==========
    int GetId() const { return id; }
    std::string GetName() const { return name; }
    int GetAge() const { return age; }
    bool IsAlive() const { return is_alive; }
    double GetLiteracy() const { return literacy_score; }
    double GetWallet() const { return wallet; }
    double GetSavings() const { return savings; }
    double GetMonthlyIncome() const { return monthly_income; }
    double GetTotalUtility() const { return total_utility; }
    double GetAddictionLevel() const { return addiction_level; }
    double GetQuantityDemanded(const std::string &product) const;
    double GetWillingnessToPay(const std::string &product) const;
    double GetSubstitutionRatio(const std::string &product1, const std::string &product2) const;

    // ========== Setters ==========
    void SetMonthlyIncome(double income) { monthly_income = income; }
    void SetLiteracy(double score) { literacy_score = score; }
    void SetPreference(const std::string &product, double weight);
    void SetWillingnessToPay(const std::string &product, double max_price);
    void SetSubstitutionRatio(const std::string &product1, const std::string &product2, double ratio);

    // ========== Demand Curve Methods ==========

    // Initialize demand curve for a product (linear: Qd = a - b*P)
    void SetupDemandCurve(const std::string &product, double max_quantity, double price_sensitivity);

    // Calculate quantity demanded at a specific price
    double CalculateDemandAtPrice(const std::string &product, double price);

    // Update quantity demanded based on current market price
    void UpdateQuantityDemanded(const std::string &product, double market_price);

    // ========== Economic Actions ==========

    // Consuming goods: demonstrates marginal utility and indifference curves
    // Implements equimarginal principle: MUx/Px = MUy/Py (choosing optimal bundle)
    virtual void Consume(const std::string &product, double quantity, double price);

    // Calculate total utility from consumption bundle (indifference curve position)
    virtual void UpdateUtility();

    // Substitution effect: switch to cheaper alternative based on substitution ratio
    virtual void SubstituteGood(const std::string &froproduct,
                                const std::string &to_product,
                                double quantity);

    // Pay taxes: reduces disposable income (shows fiscal policy effect)
    void PayTax(double tax_amount);

    // Age one period: affects literacy, mortality
    virtual void AgeTick();

    // ========== Display ==========
    virtual std::string GetInfoString() const;

protected:
    // Helper: Calculate marginal utility for a good
    double CalculateMarginalUtility(const std::string &product, double quantity);

    // Helper: Get budget remaining
    double GetBudgetRemaining() const { return wallet; }
};
