# Economic Propagation: Code Structure & Examples

## File Structure Overview

```
src/
├── EconomicPropagation.h        (NEW - Core relationship engine)
├── EconomicPropagation.cpp      (NEW - Implementation)
├── Simulation.h                 (MODIFIED - Add Step() method, use propagator)
├── Simulation.cpp               (MODIFIED - Refactor loop)
└── (other existing files unchanged)
```

---

## 1. EconomicPropagation.h - Complete Structure

```cpp
#pragma once

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <functional>
#include <memory>

// ============================================================================
// EconomicPropagation.h - Engine for variable relationships
//
// Manages how changes in one variable trigger cascading effects in others.
// Separates relationship logic from simulation control logic.
// ============================================================================

// Forward declarations
class Simulation;

// ============================================================================
// Economic Event: Records a variable change that triggers propagation
// ============================================================================

struct EconomicEvent
{
    string source;          // "farmer.harvest", "government.tax_increase"
    string variable;        // Full variable path
    double oldValue;
    double newValue;
    double changeRatio;          // newValue / oldValue (or percentage change)
    int propagationLevel;        // 0 = original event, 1+ = cascading effects
    int tick;                    // When this event occurred
    string description;     // "Farmer harvested due to bumper weather"

    string ToString() const;
};

// ============================================================================
// Propagation Rule: Defines how Variable A affects Variable B
// ============================================================================

struct PropagationRule
{
    // WHAT TRIGGERS THIS RULE
    string trigger;         // Pattern: "farmer.*.output_quantity" (supports wildcards)

    // WHAT GETS AFFECTED
    string targets;         // Can be multiple: "market.supply,market.price,..."

    // HOW IT PROPAGATES
    // Lambda takes: (Simulation*, EconomicEvent, target_variable) -> new_value
    using PropagationFunction = function<double(Simulation*, const EconomicEvent&, const string&)>;
    PropagationFunction rule;

    // MODIFIERS
    double strength;             // 0-1: diminishing effect if < 1.0
    int delay;                   // Ticks before this effect activates
    bool isImmediate;            // Should this run in Tier 1 (same tick)?
    string description;     // For documentation: "Bumper harvest causes price drop"
};

// ============================================================================
// Dependency Graph: Maps variable relationships
// ============================================================================

class DependencyGraph
{
private:
    // trigger pattern -> all affected variables
    map<string, vector<string>> outgoing;

    // affected variable -> all triggers
    map<string, vector<string>> incoming;

public:
    void AddRelation(const string& trigger, const string& target);
    vector<string> GetAffected(const string& trigger) const;
    vector<string> GetCauses(const string& variable) const;
    void PrintChain(const string& variable, int depth = 5) const;
    void ExportDot(const string& filename) const; // For Graphviz visualization
};

// ============================================================================
// Main Economic Propagation Engine
// ============================================================================

class EconomicPropagation
{
private:
    Simulation* simulation;           // Back-reference to simulation

    // RULE MANAGEMENT
    vector<PropagationRule> rules;
    map<string, vector<PropagationRule*>> rulesByTrigger;

    // EVENT QUEUE
    queue<EconomicEvent> eventQueue;
    vector<EconomicEvent> eventHistory;    // For audit trail
    int currentTick = 0;

    // DEPENDENCY TRACKING
    DependencyGraph dependencyGraph;

public:
    // ========== Constructor ==========
    EconomicPropagation(Simulation* sim);

    // ========== Rule Registration ==========

    // Register a simple propagation rule
    void RegisterRule(const PropagationRule& rule);

    // Bulk register multiple rules (call once at startup)
    void RegisterAllRules();

    // ========== Event Queueing ==========

    // Queue an event to propagate
    void QueueEvent(const EconomicEvent& event);

    // Helper: Create and queue an event from value change
    void QueueValueChange(const string& variable,
                         double oldValue,
                         double newValue,
                         const string& description);

    // ========== Processing ==========

    // Process all IMMEDIATE effects (Tier 1): price adjustments, etc.
    void ProcessImmediateEffects();

    // Process agent DECISION effects (Tier 2): farmers decide on next harvest
    void ProcessAgentDecisions();

    // Process DELAYED effects (Tier 3): systemic shifts, unemployment effects
    void ProcessDelayedEffects();

    // Helper: Apply a single rule to an event
    void ApplyRule(const PropagationRule& rule, const EconomicEvent& event);

    // ========== Queries ==========

    // Get all variables affected by a trigger
    vector<string> GetAffectedVariables(const string& trigger) const;

    // Get all causes of a variable
    vector<string> GetVariableCauses(const string& variable) const;

    // Get event history for a specific variable
    vector<EconomicEvent> GetEventHistory(const string& variable) const;

    // ========== Visualization & Debugging ==========

    // Print dependency chain for a variable
    void PrintDependencyChain(const string& variable);

    // Print all events from current or previous tick
    void PrintEventLog(int tickDelta = 0) const;

    // Export dependency graph as Graphviz DOT file
    void ExportDependencyGraph(const string& filename);

    // Get statistics on propagation
    int GetEventCount() const { return eventHistory.size(); }
    int GetCurrentTick() const { return currentTick; }

    // ========== Tick Management ==========
    void AdvanceTick() { currentTick++; }
};
```

---

## 2. EconomicPropagation.cpp - Core Rules

```cpp
#include "EconomicPropagation.h"
#include "Simulation.h"
#include "Market.h"
#include "Farmer.h"
#include "Government.h"
#include <iostream>
#include <sstream>

EconomicPropagation::EconomicPropagation(Simulation* sim)
    : simulation(sim)
{
}

void EconomicPropagation::RegisterAllRules()
{
    // ========== TIER 1: IMMEDIATE MARKET EFFECTS ==========

    // Rule 1: Supply Increase → Price Decrease
    RegisterRule({
        .trigger = "farmer.*.output_quantity",
        .targets = "market.supply,market.price,market.equilibriuquantity",
        .rule = [](Simulation* sim, const EconomicEvent& event, const string& target) -> double {
            // Parse which farmer and which market
            // Example trigger: "farmer.Shafin.output_quantity"
            size_t dot = event.variable.find('.', 7);  // Skip "farmer."
            string farmerName = event.variable.substr(7, dot - 7);

            Farmer* farmer = sim->FindFarmer(farmerName);
            if (!farmer) return 0;

            string marketName = farmer->GetCrop();
            Market* market = sim->FindMarket(marketName);
            if (!market) return 0;

            if (target == "market.supply") {
                // Supply increases by the harvest amount
                return market->GetQuantitySupplied() + event.newValue;

            } else if (target == "market.price") {
                // Law of Supply: Qs ↑ → P ↓
                // Simplified: each 10% supply increase → price drops by elasticity%
                double supplyChangeRatio = event.changeRatio;  // e.g., 1.5 = 50% increase
                double priceElasticity = 0.3;  // Price sensitivity
                double newPrice = market->GetCurrentPrice() * 
                                  (1.0 - (supplyChangeRatio - 1.0) * priceElasticity);
                return newPrice;

            } else if (target == "market.equilibriuquantity") {
                // Recalculate equilibrium based on new price
                // (simplified: demand curve slope = -0.5, etc.)
                return market->GetQuantityDemanded();
            }
            return 0;
        },
        .strength = 1.0,
        .delay = 0,
        .isImmediate = true,
        .description = "Supply increase causes market price to fall (law of supply)"
    });

    // Rule 2: Price Change → Quantity Demanded Change
    RegisterRule({
        .trigger = "market.*.price",
        .targets = "market.quantity_demanded,consumer.*.quantity_demanded",
        .rule = [](Simulation* sim, const EconomicEvent& event, const string& target) -> double {
            // Price elasticity of demand: approximately -0.8 for normal goods
            double priceElasticity = -0.8;
            double priceChangeRatio = event.changeRatio;  // e.g., 1.3 = 30% price increase

            if (target == "market.quantity_demanded") {
                // Market-level quantity demanded
                Market* market = sim->FindMarket("Rice");  // TODO: Parse from event.variable
                if (market) {
                    double oldQd = market->GetQuantityDemanded();
                    double newQd = oldQd * (1.0 + (priceChangeRatio - 1.0) * priceElasticity);
                    return newQd;
                }
            }
            return 0;
        },
        .strength = 0.9,
        .delay = 0,
        .isImmediate = true,
        .description = "Higher price causes consumers to demand less (law of demand)"
    });

    // ========== TIER 2: AGENT DECISION MAKING ==========

    // Rule 3: Low Profit → Reduce Next Period's Production
    RegisterRule({
        .trigger = "farmer.*.profit",
        .targets = "farmer.*.labor_hired,farmer.*.fertilizer_units",
        .rule = [](Simulation* sim, const EconomicEvent& event, const string& target) -> double {
            double profitChanges = event.changeRatio;  // e.g., 0.7 = 30% profit drop

            if (profitChanges < 1.0) {  // Profit declined
                double reductionFactor = 1.0 - (1.0 - profitChanges) * 0.5;  // Cut 50% of decline
                return event.oldValue * reductionFactor;
            }
            return event.oldValue;
        },
        .strength = 0.6,
        .delay = 1,  // Takes 1 tick for farmer to adjust
        .isImmediate = false,
        .description = "Farmer reduces inputs when profit falls"
    });

    // Rule 4: Unemployment Increase → Consumer Budget Decrease
    RegisterRule({
        .trigger = "labor_market.unemployment_increase",
        .targets = "consumer.*.budget",
        .rule = [](Simulation* sim, const EconomicEvent& event, const string& target) -> double {
            double unemploymentRateIncrease = event.newValue - event.oldValue;  // e.g., +0.05 = 5%
            double avgWorkerWage = 500;  // Average wage in economy

            // Each percentage point of unemployment = some workers lose income
            double budgetReduction = unemploymentRateIncrease * avgWorkerWage;

            return event.oldValue - budgetReduction;
        },
        .strength = 0.8,
        .delay = 0,
        .isImmediate = false,
        .description = "Unemployment reduces consumer budgets and demand"
    });

    // Rule 5: Wage Increase → Production Cost Increase
    RegisterRule({
        .trigger = "labor_market.wage",
        .targets = "farmer.*.variable_cost,owner.*.variable_cost",
        .rule = [](Simulation* sim, const EconomicEvent& event, const string& target) -> double {
            double wageChangeRatio = event.changeRatio;
            double laborShareOfCost = 0.6;  // Labor is 60% of variable costs

            // If wage goes up 20%, variable costs go up 20% * labor share
            return event.oldValue * (1.0 + (wageChangeRatio - 1.0) * laborShareOfCost);
        },
        .strength = 1.0,
        .delay = 0,
        .isImmediate = true,
        .description = "Higher wages increase production costs"
    });

    // ========== TIER 3: MACROECONOMIC EFFECTS ==========

    // Rule 6: Tax Increase → Worker Wallet Decrease
    RegisterRule({
        .trigger = "government.income_tax_rate",
        .targets = "worker.*.wallet",
        .rule = [](Simulation* sim, const EconomicEvent& event, const string& target) -> double {
            double newTaxRate = event.newValue;
            double oldTaxRate = event.oldValue;
            double workerIncome = 500;  // Average worker income

            double oldTaxPayment = workerIncome * oldTaxRate;
            double newTaxPayment = workerIncome * newTaxRate;
            double additionalTaxBurden = newTaxPayment - oldTaxPayment;

            return event.oldValue - additionalTaxBurden;
        },
        .strength = 1.0,
        .delay = 0,
        .isImmediate = true,
        .description = "Income tax increase reduces worker disposable income"
    });

    // Rule 7: Money Supply Increase → Price Level Increase (Quantity Theory: MV = PQ)
    RegisterRule({
        .trigger = "government.money_supply",
        .targets = "market.*.price",
        .rule = [](Simulation* sim, const EconomicEvent& event, const string& target) -> double {
            double moneySupplyChangeRatio = event.changeRatio;  // e.g., 1.1 = 10% increase

            // If real variables (V, Q) stay constant, price level must increase proportionally
            // Note: In reality, some inflation is slower due to sticky prices
            double inflationResponse = 0.7;  // 70% of money increase → inflation

            return event.oldValue * (1.0 + (moneySupplyChangeRatio - 1.0) * inflationResponse);
        },
        .strength = 0.7,
        .delay = 2,  // Takes 2 ticks for inflation to fully manifest
        .isImmediate = false,
        .description = "Money supply increase causes inflation (Quantity Theory)"
    });

    // Rule 8: Minimum Wage Above Equilibrium → Unemployment
    RegisterRule({
        .trigger = "government.minimuwage",
        .targets = "labor_market.unemployment_rate",
        .rule = [](Simulation* sim, const EconomicEvent& event, const string& target) -> double {
            double minWage = event.newValue;
            double equilibriumWage = 400;  // ASSUMPTION: equilibrium is around 400

            if (minWage <= equilibriumWage) {
                return sim->GetGovernment()->GetUnemploymentRate();  // No binding constraint
            }

            // If minimum wage > equilibrium, create unemployment
            double excessRatio = minWage / equilibriumWage;  // e.g., 1.15 = 15% above equilibrium
            double unemploymentIncrease = (excessRatio - 1.0) * 0.5;  // Each 10% excess → 5% more unemployment

            return event.oldValue + unemploymentIncrease;
        },
        .strength = 0.8,
        .delay = 1,
        .isImmediate = false,
        .description = "Minimum wage above equilibrium causes unemployment"
    });

    // Register all rules in the dependency graph
    for (const auto& rule : rules) {
        dependencyGraph.AddRelation(rule.trigger, rule.targets);
    }
}

void EconomicPropagation::QueueEvent(const EconomicEvent& event)
{
    eventQueue.push(event);
    eventHistory.push_back(event);
}

void EconomicPropagation::ProcessImmediateEffects()
{
    // Process all events in queue, but only immediate rules
    while (!eventQueue.empty()) {
        EconomicEvent event = eventQueue.front();
        eventQueue.pop();

        // Find all rules triggered by this event
        auto it = rulesByTrigger.find(event.variable);
        if (it != rulesByTrigger.end()) {
            for (auto rule : it->second) {
                if (rule->isImmediate && rule->delay == 0) {
                    ApplyRule(*rule, event);
                }
            }
        }
    }
}

void EconomicPropagation::ProcessAgentDecisions()
{
    // Process Tier 2: Agent decision-making rules
    // Similar structure but for rules with delay = 1
}

void EconomicPropagation::ProcessDelayedEffects()
{
    // Process Tier 3: Delayed systemic effects
    // Similar structure but for rules with delay > 1
}

void EconomicPropagation::ApplyRule(const PropagationRule& rule, const EconomicEvent& event)
{
    // Parse targets (can be comma-separated)
    stringstream ss(rule.targets);
    string target;
    while (getline(ss, target, ',')) {
        // Apply rule to each target
        double newValue = rule.rule(simulation, event, target);

        // Create new event for cascading effect
        EconomicEvent cascadeEvent;
        cascadeEvent.source = event.variable;
        cascadeEvent.variable = target;
        cascadeEvent.oldValue = 0;  // TODO: Get current value for this target
        cascadeEvent.newValue = newValue;
        cascadeEvent.propagationLevel = event.propagationLevel + 1;
        cascadeEvent.tick = currentTick;

        // Queue for next processing level if propagationLevel < 2
        if (cascadeEvent.propagationLevel < 3) {
            QueueEvent(cascadeEvent);
        }
    }
}
```

---

## 3. Refactored Simulation.h Changes

Add to the Simulation class:

```cpp
private:
    unique_ptr<EconomicPropagation> propagator;
    int currentTick = 0;

public:
    // NEW: Main simulation step
    void Step();

    // Helper: Get propagator (for debugging/visualization)
    EconomicPropagation* GetPropagator() { return propagator.get(); }
```

---

## 4. Refactored Simulation::Step() Implementation

```cpp
void Simulation::Step()
{
    // ========== TICK PHASE 1: AGENT ACTIONS ==========
    // Autonomous agent behaviors (harvests, consumption budgets)

    // Farmers harvest based on their planned output
    for (auto& farmer : farmers) {
        farmer->Harvest();
        // Queue event for harvest
        propagator->QueueEvent({
            .source = farmer->GetName(),
            .variable = "farmer." + farmer->GetName() + ".output_quantity",
            .oldValue = farmer->GetPreviousOutputQuantity(),
            .newValue = farmer->GetOutputQuantity(),
            .changeRatio = farmer->GetOutputQuantity() / farmer->GetPreviousOutputQuantity(),
            .propagationLevel = 0,
            .tick = currentTick,
            .description = "Farmer harvested"
        });
    }

    // Consumers update their demand at current prices
    for (auto& consumer : workers) {
        for (auto& market : markets) {
            consumer.UpdateQuantityDemanded(market.first, market.second->GetCurrentPrice());
        }
    }

    // ========== TICK PHASE 2: MARKET CLEARING ==========
    // Markets find equilibrium based on aggregate supply/demand

    for (auto& [productName, market] : markets) {
        double totalDemand = 0;
        double totalSupply = 0;

        // Aggregate consumer demand
        for (auto& worker : workers) {
            totalDemand += worker.GetQuantityDemanded(productName);
        }
        // Aggregate farmer supply
        for (auto& farmer : farmers) {
            if (farmer.GetCrop() == productName) {
                totalSupply += farmer.GetOutputQuantity();
            }
        }

        // Update market and recalculate equilibrium
        market.second->SetSupplyAndDemand(totalSupply, totalDemand);
        market.second->RecalculateEquilibrium();

        // Queue price change event if price changed
        if (market.second->GetCurrentPrice() != market.second->GetPreviousPrice()) {
            propagator->QueueEvent({
                .source = "market_clearing",
                .variable = "market." + productName + ".price",
                .oldValue = market.second->GetPreviousPrice(),
                .newValue = market.second->GetCurrentPrice(),
                .changeRatio = market.second->GetCurrentPrice() / market.second->GetPreviousPrice(),
                .propagationLevel = 0,
                .tick = currentTick,
                .description = "Price equilibrium recalculated"
            });
        }
    }

    // ========== TICK PHASE 3: PROPAGATE EFFECTS ==========
    // Economic cascade: price changes affect demand, profit, decisions, etc.

    propagator->ProcessImmediateEffects();    // Tier 1: Same-tick adjustments
    propagator->ProcessAgentDecisions();      // Tier 2: Next-period decisions
    propagator->ProcessDelayedEffects();      // Tier 3: Macroeconomic shifts

    // ========== TICK PHASE 4: GOVERNMENT POLICY ==========
    // Government applies macroeconomic policy

    government->ApplyMacroPolicy();

    // ========== TICK PHASE 5: STATISTICS ==========
    // Update all aggregate statistics

    RefreshStats();

    // Advance time
    propagator->AdvanceTick();
    currentTick++;

    // Optional: Callback for UI updates
    if (statsCallback) {
        statsCallback(stats);
    }
}
```

---

## 5. Example: Running a Simulation

```cpp
int main()
{
    Simulation sim;
    sim.Initialize(5, 2, 1);  // 5 workers, 2 farmers, 1 owner

    // Run for 24 ticks (2 years, assuming monthly ticks)
    for (int t = 0; t < 24; t++) {
        sim.Step();

        // Every 6 ticks, print what happened
        if (t % 6 == 0) {
            cout << "\n=== MONTH " << (t + 1) << " ===\n";
            sim.GetPropagator()->PrintEventLog(0);  // Print last tick's events
            sim.GetPropagator()->PrintDependencyChain("market.Rice.price");
        }
    }

    // Show final dependency graph
    sim.GetPropagator()->ExportDependencyGraph("economy.dot");
    cout << "\nDependency graph exported to economy.dot\n";
    cout << "Visualize with: dot -Tpng economy.dot -o economy.png\n";

    return 0;
}
```

Output would look like:

```
=== MONTH 1 ===
[TICK 0, Level 0] farmer.Shafin.output_quantity:  850.0 → 1275.0 (50% increase)
  └─ [TICK 0, Level 1] market.Rice.supply: 500.0 → 775.0
     └─ [TICK 0, Level 1] market.Rice.price: 5.0 → 3.8 (24% decrease)
        └─ [TICK 0, Level 2] market.Rice.quantity_demanded: 200.0 → 286.0 (43% increase)

=== DEPENDENCY CHAINS ===
To change market.Rice.price, you can affect:
  ← farmer.*.output_quantity (law of supply)
  ← market.Rice.supply (direct)
  ← government.money_supply (inflation)
  ← government.income_tax_rate (demand effect)
```

This shows exactly how the economy responds!

---

## Benefits Summary

✅ **Every relationship is explicit and traceable**  
✅ **Easy to add new rules without modifying Simulation**  
✅ **Can visualize how variables affect each other**  
✅ **Automatic detection of feedback loops**  
✅ **Educational: students see economic cause-and-effect**  
✅ **Realistic: emergent behavior from rule combinations**  
