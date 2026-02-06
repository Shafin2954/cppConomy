#pragma once

#include <string>
#include <map>
#include "Product.h"

// ============================================================================
// Person.h - Base class for all economic agents
//
// Maps to: Utility, Indifference Curves, Budget Line, Literacy, Mortality,
//          Consumer Surplus, Income Effect, Marginal Utility, Tax effects
// ============================================================================

class Person
{
protected:
    int m_id;
    std::string m_name;
    int m_age;               // For mortality/child labor
    bool m_is_alive;         // For natural disasters
    double m_literacy_score; // 0.0 - 1.0: affects income potential

    // ========== Budget Constraint (Budget Line) ==========
    double m_wallet;         // Current disposable income
    double m_savings;        // Accumulated wealth
    double m_monthly_income; // Regular income (from wages, etc.)

    // ========== Utility & Preferences (Indifference Curves) ==========
    double m_total_utility;   // Overall happiness/satisfaction
    double m_addiction_level; // 0.0 - 1.0: how addicted to certain goods

    // Preferences for different products (how much they value each)
    std::map<std::string, double> m_preferences; // product_name -> preference_weight

    // Goods owned (for calculating consumer surplus)
    std::map<std::string, double> m_goods_owned; // product_name -> quantity

    // Taxation
    double m_income_tax_paid; // Track taxes paid this period

public:
    // ========== Constructors & Destructors ==========
    Person(int id, const std::string &name, double initial_income);
    virtual ~Person() = default;

    // ========== Getters ==========
    int GetId() const { return m_id; }
    std::string GetName() const { return m_name; }
    int GetAge() const { return m_age; }
    bool IsAlive() const { return m_is_alive; }
    double GetLiteracy() const { return m_literacy_score; }
    double GetWallet() const { return m_wallet; }
    double GetSavings() const { return m_savings; }
    double GetMonthlyIncome() const { return m_monthly_income; }
    double GetTotalUtility() const { return m_total_utility; }
    double GetAddictionLevel() const { return m_addiction_level; }

    // ========== Setters ==========
    void SetMonthlyIncome(double income) { m_monthly_income = income; }
    void SetLiteracy(double score) { m_literacy_score = score; }
    void SetPreference(const std::string &product, double weight);

    // ========== Economic Actions ==========

    // Consuming goods: demonstrates marginal utility and indifference curves
    // Implements equimarginal principle: MUx/Px = MUy/Py (choosing optimal bundle)
    virtual void Consume(const std::string &product, double quantity, double price);

    // Calculate total utility from consumption bundle (indifference curve position)
    virtual void UpdateUtility();

    // Substitution effect: switch to cheaper alternative
    virtual void SubstituteGood(const std::string &from_product,
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
    double GetBudgetRemaining() const { return m_wallet; }
};
