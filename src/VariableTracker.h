#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

// ============================================================================
// VariableTracker.h - Comprehensive system for tracking variable changes
//
// This system logs all changes to economic variables and shows how changes in
// one variable propagate to affect other variables. It provides:
// - Complete audit trail of all changes
// - Relationship mapping (if X changes, then Y is affected)
// - Visualized dependency chains
// - Automatic calculation of secondary effects
// ============================================================================

class VariableTracker
{
public:
    // ========== Variable Record ==========
    struct VariableRecord
    {
        std::string name;
        std::string category; // "Market", "Macro", "Agent", "Policy", etc.
        double oldValue;
        double newValue;
        double percentChange; // Percentage change
        std::string timestamp;
        std::string reason;                  // Why did it change?
        std::vector<std::string> affectedBy; // What caused this change
        std::vector<std::string> affects;    // What this change impacts
        int cascadeLevel;                    // 0 = direct change, 1+ = secondary effect

        std::string ToString(bool showRelations = true) const;
    };

    // ========== Relationship Definition ==========
    struct Relationship
    {
        std::string source;   // Variable that changes
        std::string target;   // Variable that is affected
        std::string effect;   // "increases", "decreases", "both"
        std::string equation; // Mathematical relationship (for reference)
        double strength;      // 0-1, how strong the effect is

        std::string ToString() const;
    };

    // ========== Change Categories ==========
    enum class Category
    {
        MARKET_DEMAND,     // Demand-side changes
        MARKET_SUPPLY,     // Supply-side changes
        MARKET_PRICE,      // Price changes
        ELASTICITY,        // Elasticity changes
        COSTS,             // Cost structure changes
        PRODUCTION,        // Production changes
        MACRO_POLICY,      // Government policy changes
        MONETARY_POLICY,   // Central bank policy changes
        NATURAL_EVENT,     // Weather, disasters, etc.
        CONSUMER_BEHAVIOR, // Consumer preference changes
        FIRBEHAVIOR,       // Firm decision changes
        OTHER
    };

private:
    std::vector<VariableRecord> changeHistory;                       // Complete log
    std::map<std::string, Relationship> relationshipMap;             // var1->var2 relationships
    std::map<std::string, std::vector<std::string>> dependencyGraph; // What depends on what

    size_t maxHistorySize = 1000;         // Limit history to prevent memory explosion
    bool enableCascadeCalculation = true; // Automatically calculate secondary effects

public:
    VariableTracker();
    ~VariableTracker() = default;

    // ========== Recording Changes ==========

    // Record a numeric variable change
    void RecordNumericChange(
        const std::string &variableName,
        const std::string &category,
        double oldValue,
        double newValue,
        const std::string &reason = "");

    // Record a categorical variable change (e.g., "Rain" -> "Drought")
    void RecordCategoricalChange(
        const std::string &variableName,
        const std::string &category,
        const std::string &oldValue,
        const std::string &newValue,
        const std::string &reason = "");

    // ========== Relationship Management ==========

    // Define how variables relate to each other
    // Example: RegisterRelationship("Price of Rice", "Demand for Rice", "decreases");
    void RegisterRelationship(
        const std::string &sourceVar,
        const std::string &targetVar,
        const std::string &effect,
        const std::string &equation = "",
        double strength = 1.0);

    // Automatically calculate cascade effects when a variable changes
    void CalculateCascadeEffects(const std::string &changedVariable);

    // ========== History Access ==========

    // Get all changes in history
    const std::vector<VariableRecord> &GetHistory() const { return changeHistory; }

    // Get changes for a specific variable
    std::vector<VariableRecord> GetChangesForVariable(const std::string &variableName) const;

    // Get all changes in a specific time range
    std::vector<VariableRecord> GetChangesInLastNEvents(size_t n) const;

    // Get change count
    size_t GetChangeCount() const { return changeHistory.size(); }

    // ========== Analysis & Reporting ==========

    // Get summary of most recent changes
    std::string GetRecentChangesSummary(size_t numChanges = 10) const;

    // Get the dependency chain for a variable
    // Shows: X changed -> Y was affected -> Z was affected -> ...
    std::vector<std::string> GetDependencyChain(const std::string &variable) const;

    // Generate a visual report of recent changes
    std::string GenerateChangeReport() const;

    // Get all variables that depend on a given variable
    std::vector<std::string> GetDependents(const std::string &variable) const;

    // Get all variables that affect a given variable
    std::vector<std::string> GetDependencies(const std::string &variable) const;

    // ========== Configuration ==========

    void SetMaxHistorySize(size_t size) { maxHistorySize = size; }
    void SetEnableCascadeCalculation(bool enable) { enableCascadeCalculation = enable; }

    // ========== Clearing ==========

    void ClearHistory() { changeHistory.clear(); }
    void ClearRelationships()
    {
        relationshipMap.clear();
        dependencyGraph.clear();
    }

private:
    // Helper to get current time as std::string
    std::string GetCurrentTimestamp() const;

    // Helper to calculate percentage change
    double CalculatePercentChange(double oldVal, double newVal) const;

    // Helper to trim history if it gets too large
    void TrimHistory();

    // Register common relationships used in economic models
    void RegisterCommonRelationships();
};
