#pragma once

#include "Consumer.h"


// ============================================================================
// Worker.h - Inherits from Consumer
//
// Maps to: Employment, Unemployment, Minimum Wage, Labor Supply,
//          Wage effect on behavior, Child Labor
// ============================================================================

class Worker : public Consumer
{
private:
    double skill_level;         // 0.0 - 1.0: affected by literacy
    double min_acceptable_wage; // Supply curve: won't work below this
    bool is_employed;           // Current employment status
    double current_wage;        // Wage offered by employer
    std::string employer;            // Name of current employer (if employed)

    // For labor supply curve
    int hours_willing_to_work; // Changes with wage (labor-leisure tradeoff)

public:
    // ========== Constructors ==========
    Worker(int id, const std::string &name, double initial_income, double skill);

    // ========== Getters ==========
    double GetSkillLevel() const { return skill_level; }
    double GetMinAcceptableWage() const { return min_acceptable_wage; }
    bool IsEmployed() const { return is_employed; }
    double GetCurrentWage() const { return current_wage; }
    std::string GetEmployer() const { return employer; }
    int GetHoursWillingToWork() const { return hours_willing_to_work; }

    // ========== Setters ==========
    void SetSkillLevel(double skill) { skill_level = skill; }
    void SetMinAcceptableWage(double wage) { min_acceptable_wage = wage; }

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

    // ========== Economic Decision Methods (for Propagation Engine) ==========

    // Decide whether to search for work based on wage offers and reservation wage
    // Unemployed workers search when market wage > reservation wage
    // This demonstrates labor market friction and job search behavior
    void DecideJobSearch();

    // Update reservation wage based on unemployment duration and market conditions
    // Long unemployment → lower reservation wage (desperation)
    // Tight labor market → higher reservation wage (can be picky)
    void UpdateReservationWage();

    // ========== Display ==========
    std::string GetInfoString() const override;
};
