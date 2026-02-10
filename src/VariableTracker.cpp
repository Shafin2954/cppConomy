#include "VariableTracker.h"
#include <chrono>
#include <algorithm>

using namespace std;

// ========== VariableRecord::ToString ==========
string VariableTracker::VariableRecord::ToString(bool showRelations) const
{
    ostringstream oss;
    oss << fixed << setprecision(2);

    // Calculate percent change
    double percentChange = 0;
    if (oldValue != 0)
    {
        percentChange = ((newValue - oldValue) / abs(oldValue)) * 100;
    }

    // Color/arrow based on direction
    string arrow = newValue > oldValue ? "↑" : (newValue < oldValue ? "↓" : "→");
    string symbol = "";

    if (newValue > oldValue)
        symbol = "🟢 ";
    else if (newValue < oldValue)
        symbol = "🔴 ";
    else
        symbol = "⚪ ";

    oss << symbol << name << " [" << category << "]\n";
    oss << "   " << oldValue << " " << arrow << " " << newValue;
    oss << " (" << arrow << " " << percentChange << "%)\n";

    if (!reason.empty())
    {
        oss << "   Reason: " << reason << "\n";
    }

    if (showRelations)
    {
        if (!affectedBy.empty())
        {
            oss << "   Caused by: ";
            for (size_t i = 0; i < affectedBy.size(); i++)
            {
                if (i > 0)
                    oss << ", ";
                oss << affectedBy[i];
            }
            oss << "\n";
        }

        if (!affects.empty())
        {
            oss << "   Affects: ";
            for (size_t i = 0; i < affects.size(); i++)
            {
                if (i > 0)
                    oss << ", ";
                oss << affects[i];
            }
            oss << "\n";
        }
    }

    oss << "   Time: " << timestamp << "\n";

    return oss.str();
}

// ========== Relationship::ToString ==========
string VariableTracker::Relationship::ToString() const
{
    ostringstream oss;
    oss << fixed << setprecision(1);

    string effectStr = effect;
    if (effect == "increases")
        effectStr = "↑ increases";
    else if (effect == "decreases")
        effectStr = "↓ decreases";

    oss << source << " → " << effectStr << " → " << target;
    if (!equation.empty())
    {
        oss << " [" << equation << "]";
    }
    oss << " (strength: " << (strength * 100) << "%)";

    return oss.str();
}

// ========== Constructor ==========
VariableTracker::VariableTracker()
    : maxHistorySize(1000), enableCascadeCalculation(true)
{
    // Pre-register common relationships for economic models
    RegisterCommonRelationships();
}

// ========== Recording Changes ==========
void VariableTracker::RecordNumericChange(
    const string &variableName,
    const string &category,
    double oldValue,
    double newValue,
    const string &reason)
{
    if (oldValue == newValue)
        return; // No change, don't record

    VariableRecord record;
    record.name = variableName;
    record.category = category;
    record.oldValue = oldValue;
    record.newValue = newValue;
    record.percentChange = CalculatePercentChange(oldValue, newValue);
    record.timestamp = GetCurrentTimestamp();
    record.reason = reason;
    record.cascadeLevel = 0;

    changeHistory.push_back(record);

    // Calculate cascade effects if enabled
    if (enableCascadeCalculation)
    {
        CalculateCascadeEffects(variableName);
    }

    TrimHistory();
}

void VariableTracker::RecordCategoricalChange(
    const string &variableName,
    const string &category,
    const string &oldValue,
    const string &newValue,
    const string &reason)
{
    if (oldValue == newValue)
        return; // No change, don't record

    // For categorical changes, we store them as strings in oldValue/newValue
    // This is a bit of a hack but allows us to use the same system
    VariableRecord record;
    record.name = variableName;
    record.category = category;
    record.oldValue = 0; // Placeholder
    record.newValue = 1; // Placeholder
    record.timestamp = GetCurrentTimestamp();
    record.reason = reason;
    record.cascadeLevel = 0;

    changeHistory.push_back(record);

    if (enableCascadeCalculation)
    {
        CalculateCascadeEffects(variableName);
    }

    TrimHistory();
}

// ========== Relationship Management ==========
void VariableTracker::RegisterRelationship(
    const string &sourceVar,
    const string &targetVar,
    const string &effect,
    const string &equation,
    double strength)
{
    string relationshipKey = sourceVar + "->" + targetVar;

    Relationship rel;
    rel.source = sourceVar;
    rel.target = targetVar;
    rel.effect = effect;
    rel.equation = equation;
    rel.strength = strength;

    relationshipMap[relationshipKey] = rel;
    dependencyGraph[sourceVar].push_back(targetVar);
}

void VariableTracker::CalculateCascadeEffects(const string &changedVariable)
{
    // Find all variables affected by the change
    vector<string> affected = GetDependents(changedVariable);

    // For now, just mark them as affected in the last recorded change
    if (!changeHistory.empty())
    {
        changeHistory.back().affects = affected;
    }
}

// ========== History Access ==========
vector<VariableTracker::VariableRecord> VariableTracker::GetChangesForVariable(
    const string &variableName) const
{
    vector<VariableRecord> result;

    for (const auto &record : changeHistory)
    {
        if (record.name == variableName)
        {
            result.push_back(record);
        }
    }

    return result;
}

vector<VariableTracker::VariableRecord> VariableTracker::GetChangesInLastNEvents(
    size_t n) const
{
    vector<VariableRecord> result;

    size_t start = changeHistory.size() > n ? changeHistory.size() - n : 0;

    for (size_t i = start; i < changeHistory.size(); i++)
    {
        result.push_back(changeHistory[i]);
    }

    return result;
}

// ========== Analysis & Reporting ==========
string VariableTracker::GetRecentChangesSummary(size_t numChanges) const
{
    ostringstream oss;

    auto recent = GetChangesInLastNEvents(numChanges);

    if (recent.empty())
    {
        oss << "No changes recorded yet.\n";
        return oss.str();
    }

    oss << "═══════════════════════════════════════════════════════════╗\n";
    oss << "               RECENT VARIABLE CHANGES                    ║\n";
    oss << "═══════════════════════════════════════════════════════════╝\n";

    for (const auto &change : recent)
    {
        oss << change.ToString(true);
        oss << "\n";
    }

    return oss.str();
}

vector<string> VariableTracker::GetDependencyChain(
    const string &variable) const
{
    vector<string> chain;
    chain.push_back(variable);

    // BFS to find all dependencies
    vector<string> queue = GetDependents(variable);

    while (!queue.empty())
    {
        string current = queue[0];
        queue.erase(queue.begin());

        chain.push_back(current);

        vector<string> nextLevel = GetDependents(current);
        for (const auto &var : nextLevel)
        {
            if (find(chain.begin(), chain.end(), var) == chain.end())
            {
                queue.push_back(var);
            }
        }
    }

    return chain;
}

string VariableTracker::GenerateChangeReport() const
{
    ostringstream oss;

    oss << "═══════════════════════════════════════════════════════════╗\n";
    oss << "           ECONOMIC VARIABLE CHANGE REPORT                ║\n";
    oss << "═══════════════════════════════════════════════════════════╝\n";
    oss << "\nTotal Changes Recorded: " << changeHistory.size() << "\n\n";

    // Group changes by category
    map<string, int> changesByCategory;
    for (const auto &record : changeHistory)
    {
        changesByCategory[record.category]++;
    }

    oss << "Changes by Category:\n";
    for (const auto &entry : changesByCategory)
    {
        oss << "  • " << entry.first << ": " << entry.second << " changes\n";
    }

    oss << "\n"
        << GetRecentChangesSummary(5);

    return oss.str();
}

vector<string> VariableTracker::GetDependents(
    const string &variable) const
{
    vector<string> dependents;

    auto it = dependencyGraph.find(variable);
    if (it != dependencyGraph.end())
    {
        dependents = it->second;
    }

    return dependents;
}

vector<string> VariableTracker::GetDependencies(
    const string &variable) const
{
    vector<string> dependencies;

    // Search through all relationships to find those that target this variable
    for (const auto &entry : relationshipMap)
    {
        if (entry.second.target == variable)
        {
            dependencies.push_back(entry.second.source);
        }
    }

    return dependencies;
}

// ========== Private Helpers ==========
string VariableTracker::GetCurrentTimestamp() const
{
    auto now = chrono::system_clock::now();
    auto time = chrono::system_clock::to_time_t(now);

    ostringstream oss;
    oss << put_time(localtime(&time), "%Y-%m-%d %H:%M:%S");

    return oss.str();
}

double VariableTracker::CalculatePercentChange(double oldVal, double newVal) const
{
    if (oldVal == 0)
    {
        return newVal != 0 ? 100.0 : 0.0;
    }

    return ((newVal - oldVal) / abs(oldVal)) * 100.0;
}

void VariableTracker::TrimHistory()
{
    if (changeHistory.size() > maxHistorySize)
    {
        // Remove oldest entries
        changeHistory.erase(
            changeHistory.begin(),
            changeHistory.begin() + (changeHistory.size() - maxHistorySize));
    }
}

// ========== Register Common Relationships ==========
void VariableTracker::RegisterCommonRelationships()
{
    // ========== MARKET RELATIONSHIPS ==========
    RegisterRelationship("Price", "Quantity Demanded", "decreases", "Law of Demand", 0.8);
    RegisterRelationship("Price", "Quantity Supplied", "increases", "Law of Supply", 0.8);
    RegisterRelationship("Income", "Quantity Demanded", "increases", "Normal Good", 0.6);
    RegisterRelationship("Taste/Preference", "Quantity Demanded", "increases", "Preference Shift", 0.7);
    RegisterRelationship("Substitute Price", "Quantity Demanded", "increases", "Substitutes", 0.5);

    // ========== ELASTICITY RELATIONSHIPS ==========
    RegisterRelationship("Price Elasticity", "Revenue Recommendation", "changes", "PED Analysis", 0.9);
    RegisterRelationship("Demand Elasticity", "Quantity Demanded", "changes", "Elastic Demand", 0.7);

    // ========== SUPPLY SHOCK RELATIONSHIPS ==========
    RegisterRelationship("Supply Shock", "Quantity Supplied", "changes", "Supply Curve Shift", 0.9);
    RegisterRelationship("Supply Shock", "Price", "changes", "Market Adjustment", 0.8);
    RegisterRelationship("Technology Improvement", "Supply", "increases", "Tech Shift", 0.8);

    // ========== POLICY RELATIONSHIPS ==========
    RegisterRelationship("Income Tax Rate", "Disposable Income", "decreases", "Tax Impact", 0.9);
    RegisterRelationship("Minimum Wage", "Unemployment", "increases", "Labor Market Floor", 0.6);
    RegisterRelationship("Tax on Good", "Price", "increases", "Tax Incidence", 0.7);
    RegisterRelationship("Subsidy", "Price", "decreases", "Subsidy Effect", 0.7);

    // ========== PRODUCTION RELATIONSHIPS ==========
    RegisterRelationship("Labor Employed", "Output", "increases", "Production Function", 0.7);
    RegisterRelationship("Capital", "Output", "increases", "Production Function", 0.7);
    RegisterRelationship("Technology Level", "Output", "increases", "Productivity", 0.8);

    // ========== MACRO RELATIONSHIPS ==========
    RegisterRelationship("Money Supply", "Price Level", "increases", "Quantity Theory", 0.8);
    RegisterRelationship("Real Output", "Employment", "increases", "Production", 0.7);
    RegisterRelationship("Interest Rate", "Investment", "decreases", "Cost of Capital", 0.7);
    RegisterRelationship("Aggregate Demand", "Price Level", "increases", "AD/AS Model", 0.6);
    RegisterRelationship("Unemployment", "Inflation", "decreases", "Phillips Curve", 0.5);
}
