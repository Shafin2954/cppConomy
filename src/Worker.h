#pragma once

#include "Person.h"

// ============================================================================
// Worker.h - Inherits from Person
//
// Maps to: Employment, Unemployment, Minimum Wage, Labor Supply,
//          Wage effect on behavior, Child Labor
// ============================================================================

class Worker : public Person
{
private:
    double m_skill_level;         // 0.0 - 1.0: affected by literacy
    double m_min_acceptable_wage; // Supply curve: won't work below this
    bool m_is_employed;           // Current employment status
    double m_current_wage;        // Wage offered by employer
    std::string m_employer;       // Name of current employer (if employed)

    // For labor supply curve
    int m_hours_willing_to_work; // Changes with wage (labor-leisure tradeoff)

public:
    // ========== Constructors ==========
    Worker(int id, const std::string &name, double initial_income, double skill);

    // ========== Getters ==========
    double GetSkillLevel() const { return m_skill_level; }
    double GetMinAcceptableWage() const { return m_min_acceptable_wage; }
    bool IsEmployed() const { return m_is_employed; }
    double GetCurrentWage() const { return m_current_wage; }
    std::string GetEmployer() const { return m_employer; }
    int GetHoursWillingToWork() const { return m_hours_willing_to_work; }

    // ========== Setters ==========
    void SetSkillLevel(double skill) { m_skill_level = skill; }
    void SetMinAcceptableWage(double wage) { m_min_acceptable_wage = wage; }

    // ========== Labor Market Actions ==========

    // Offer labor to market: will work only if wage >= min_acceptable_wage
    // If wage < min_acceptable_wage: unemployment (labor supply decision)
    // If wage > min_acceptable_wage: more hours offered (backward/forward bending curve)
    void OfferLabor(double wage_offered, const std::string &employer);

    // Accept job offer: sets employment status and wage
    void AcceptJob(double wage, const std::string &employer);

    // Lose job: unemployment effect
    void LoseJob();

    // Minimum wage effect: if government sets min_wage > current_wage,
    // worker becomes unemployed unless employer matches
    void RespondToMinimumWage(double government_min_wage);

    // ========== Behavioral Effects ==========

    // Income effect from higher wage: more income -> may buy more goods
    // Substitution effect: higher wage makes leisure more expensive
    void UpdateLaborSupply(double new_wage);

    // ========== Display ==========
    std::string GetInfoString() const override;
};
