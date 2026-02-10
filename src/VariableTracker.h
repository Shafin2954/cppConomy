#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace std;

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
        string name;
        string category; // "Market", "Macro", "Agent", "Policy", etc.
        double oldValue;
        double newValue;
        double percentChange; // Percentage change
        string timestamp;
        string reason;             // Why did it change?
        vector<string> affectedBy; // What caused this change
        vector<string> affects;    // What this change impacts
        int cascadeLevel;          // 0 = direct change, 1+ = secondary effect

        string ToString(bool showRelations = true) const;
    };

    // ========== Relationship Definition ==========
    struct Relationship
    {
        string source;   // Variable that changes
        string target;   // Variable that is affected
        string effect;   // "increases", "decreases", "both"
        string equation; // Mathematical relationship (for reference)
        double strength; // 0-1, how strong the effect is

        string ToString() const;
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
    vector<VariableRecord> changeHistory;        // Complete log
    map<string, Relationship> relationshipMap;   // var1->var2 relationships
    map<string, vector<string>> dependencyGraph; // What depends on what

    size_t maxHistorySize = 1000;         // Limit history to prevent memory explosion
    bool enableCascadeCalculation = true; // Automatically calculate secondary effects

public:
    VariableTracker();
    ~VariableTracker() = default;

    // ========== Recording Changes ==========

    // Record a numeric variable change
    void RecordNumericChange(
        const string &variableName,
        const string &category,
        double oldValue,
        double newValue,
        const string &reason = "");

    // Record a categorical variable change (e.g., "Rain" -> "Drought")
    void RecordCategoricalChange(
        const string &variableName,
        const string &category,
        const string &oldValue,
        const string &newValue,
        const string &reason = "");

    // ========== Relationship Management ==========

    // Define how variables relate to each other
    // Example: RegisterRelationship("Price of Rice", "Demand for Rice", "decreases");
    void RegisterRelationship(
        const string &sourceVar,
        const string &targetVar,
        const string &effect,
        const string &equation = "",
        double strength = 1.0);

    // Automatically calculate cascade effects when a variable changes
    void CalculateCascadeEffects(const string &changedVariable);

    // ========== History Access ==========

    // Get all changes in history
    const vector<VariableRecord> &GetHistory() const { return changeHistory; }

    // Get changes for a specific variable
    vector<VariableRecord> GetChangesForVariable(const string &variableName) const;

    // Get all changes in a specific time range
    vector<VariableRecord> GetChangesInLastNEvents(size_t n) const;

    // Get change count
    size_t GetChangeCount() const { return changeHistory.size(); }

    // ========== Analysis & Reporting ==========

    // Get summary of most recent changes
    string GetRecentChangesSummary(size_t numChanges = 10) const;

    // Get the dependency chain for a variable
    // Shows: X changed -> Y was affected -> Z was affected -> ...
    vector<string> GetDependencyChain(const string &variable) const;

    // Generate a visual report of recent changes
    string GenerateChangeReport() const;

    // Get all variables that depend on a given variable
    vector<string> GetDependents(const string &variable) const;

    // Get all variables that affect a given variable
    vector<string> GetDependencies(const string &variable) const;

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
    // Helper to get current time as string
    string GetCurrentTimestamp() const;

    // Helper to calculate percentage change
    double CalculatePercentChange(double oldVal, double newVal) const;

    // Helper to trim history if it gets too large
    void TrimHistory();

    // Register common relationships used in economic models
    void RegisterCommonRelationships();
};
