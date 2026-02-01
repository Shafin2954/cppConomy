#pragma once

#include "../utils/Config.h"
#include <string>

class Government
{
public:
    static Government &getInstance()
    {
        static Government instance;
        return instance;
    }

    // Policy actions
    void adjustTax(double rate, const std::string &target = "all");
    void setInterestRate(double rate);
    void injectMoney(double amount);
    void grantStimulus(double amount, const std::string &sector = "all");
    void collectTaxes(double income);

    // Getters
    double getTaxRate() const { return m_taxRate; }
    double getCorporateTaxRate() const { return m_corporateTaxRate; }
    double getInterestRate() const { return m_interestRate; }
    double getMoneySupply() const { return m_moneySupply; }
    double getBudget() const { return m_budget; }
    double getDebt() const { return m_debt; }
    double getSpending() const { return m_spending; }
    double getTaxRevenue() const { return m_taxRevenue; }

    // Setters for direct manipulation
    void setMoneySupply(double supply) { m_moneySupply = supply; }
    void addToBudget(double amount) { m_budget += amount; }
    void spend(double amount);

    // Reset monthly tracking
    void resetMonthlyStats()
    {
        m_taxRevenue = 0.0;
        m_spending = 0.0;
    }

private:
    Government();
    Government(const Government &) = delete;
    Government &operator=(const Government &) = delete;

    double m_taxRate;          // Income tax
    double m_corporateTaxRate; // Corporate tax
    double m_interestRate;     // Central bank rate
    double m_moneySupply;      // M2
    double m_budget;           // Treasury balance
    double m_debt;             // National debt
    double m_spending;         // Current period spending
    double m_taxRevenue;       // Current period revenue
};
