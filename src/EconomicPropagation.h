#pragma once

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <functional>
#include <sstream>

// ============================================================================
// EconomicPropagation.h - The Economic Relationship Engine
//
// This is the CORE of the fully-connected economic system.
// It manages HOW changes in one variable cascade to affect others.
//
// Architecture:
//   1. Events are queued when any economic variable changes
//   2. Rules define relationships between variables
//   3. The engine processes events through 3 tiers:
//      - Tier 1 (Immediate): Market clearing, price adjustments
//      - Tier 2 (Decisions):  Agent responses to new information
//      - Tier 3 (Delayed):    Slow macro effects (inflation, unemployment)
//   4. Each rule application can create NEW events, forming cascades
//
// Example cascade:
//   farmer.harvest ↑ → market.supply ↑ → market.price ↓
//   → consumer.demand ↑ → farmer.revenue ↓ → farmer.profit ↓
//   → farmer.labor_hired ↓ → unemployment ↑ → consumer.budget ↓
//
// All relationship logic lives HERE, not scattered across entity classes.
// ============================================================================

// Forward declaration - we reference Simulation but don't include it
// to avoid circular dependency (Simulation.h includes this file)
class Simulation;

// ============================================================================
// EconomicEvent - Records that a variable changed, triggering propagation
//
// Every time a variable changes (farmer harvests, government sets tax, etc.),
// an EconomicEvent is created and queued for processing.
// ============================================================================
struct EconomicEvent
{
    std::string source;      // Who caused this? "farmer.Shafin", "government", "market_clearing"
    std::string variable;    // Full path: "farmer.Shafin.output_quantity", "market.Rice.price"
    double oldValue;         // Value before the change
    double newValue;         // Value after the change
    double changeRatio;      // newValue / oldValue (1.5 = 50% increase, 0.8 = 20% decrease)
    int propagationLevel;    // 0 = original event, 1+ = cascaded from another event
    int tick;                // Which simulation tick this happened on
    std::string description; // Human-readable: "Bumper harvest increased output by 50%"

    // Format this event as a readable std::string for logging/display
    std::string ToString() const
    {
        std::ostringstream ss;
        ss << "[Tick " << tick << ", Level " << propagationLevel << "] "
           << variable << ": " << oldValue << " -> " << newValue;
        if (!description.empty())
            ss << " (" << description << ")";
        return ss.str();
    }
};

// ============================================================================
// PropagationRule - Defines HOW Variable A affects Variable B
//
// Each rule says:
//   "When <trigger> changes, apply <rule function> to calculate new <target>"
//
// The rule function receives: (Simulation*, the triggering event, target name)
// and returns the new value for the target variable.
//
// Rules have modifiers:
//   - strength: 0-1, scales the effect (0.5 = half strength)
//   - delay: how many ticks before the effect manifests
//   - tier: which processing phase (1=immediate, 2=decisions, 3=delayed)
// ============================================================================
struct PropagationRule
{
    // --- What triggers this rule ---
    // Pattern matching: "farmer.*.output_quantity" matches any farmer's output
    // Exact match: "government.income_tax_rate" matches only that variable
    std::string trigger;

    // --- What gets affected ---
    // Comma-separated list: "market.supply,market.price" affects both
    std::string targets;

    // --- How to calculate the new value ---
    // Lambda: (Simulation*, triggering event, target variable name) -> new value
    using PropagationFunction = std::function<double(Simulation *, const EconomicEvent &, const std::string &)>;
    PropagationFunction apply;

    // --- Effect modifiers ---
    double strength; // 0.0-1.0: scales the magnitude of the effect
    int tier;        // 1 = immediate (same tick), 2 = agent decision (next tick), 3 = delayed (2+ ticks)
    int delay;       // Number of ticks before the effect activates (0 = same tick)

    // --- Documentation ---
    std::string description; // Human-readable explanation of this economic relationship
};

// ============================================================================
// DependencyGraph - Maps which variables affect which other variables
//
// Used for:
//   - Visualizing the economic system as a directed graph
//   - Tracing WHY a variable changed (follow causes backwards)
//   - Predicting WHAT will be affected if a variable changes
//   - Exporting to Graphviz DOT format for visual diagrams
// ============================================================================
class DependencyGraph
{
private:
    // trigger_pattern -> list of affected variables
    // e.g., "farmer.*.output_quantity" -> ["market.supply", "market.price"]
    std::map<std::string, std::vector<std::string>> outgoing;

    // affected_variable -> list of triggers that cause it
    // e.g., "market.price" -> ["farmer.*.output_quantity", "government.money_supply"]
    std::map<std::string, std::vector<std::string>> incoming;

public:
    // Register that trigger affects target
    void AddRelation(const std::string &trigger, const std::string &target);

    // Get all variables affected by this trigger
    std::vector<std::string> GetAffected(const std::string &trigger) const;

    // Get all triggers that cause this variable to change
    std::vector<std::string> GetCauses(const std::string &variable) const;

    // Print dependency chain: what affects what, recursively up to `depth` levels
    // Output example:
    //   market.Rice.price is affected by:
    //     ← farmer.*.output_quantity (supply change)
    //     ← government.money_supply (inflation)
    void PrintChain(const std::string &variable, int depth = 5) const;

    // Export the entire graph as a Graphviz DOT file
    // Can be visualized with: dot -Tpng output.dot -o output.png
    void ExportDot(const std::string &filename) const;
};

// ============================================================================
// EconomicPropagation - The Main Engine
//
// This is the heart of the system. It:
//   1. Stores all registered propagation rules
//   2. Maintains an event queue of pending changes
//   3. Processes events through three tiers
//   4. Tracks event history for auditing/debugging
//   5. Builds a dependency graph for visualization
//
// Usage in Simulation::Step():
//   propagator->ProcessTier1();  // Immediate market effects
//   propagator->ProcessTier2();  // Agent decision responses
//   propagator->ProcessTier3();  // Delayed macro effects
// ============================================================================
class EconomicPropagation
{
private:
    // Back-reference to the simulation (needed so rules can read/write agent state)
    Simulation *simulation;

    // ========== Rule Storage ==========
    // All registered propagation rules
    std::vector<PropagationRule> rules;

    // Index: trigger pattern -> list of rule indices for fast lookup
    // When an event comes in, we look up which rules match its variable name
    std::map<std::string, std::vector<int>> ruleIndex;

    // ========== Event Queue ==========
    // Pending events waiting to be processed
    std::queue<EconomicEvent> eventQueue;

    // Complete history of all events (for audit trail and debugging)
    std::vector<EconomicEvent> eventHistory;

    // Delayed events: events scheduled for future ticks
    // Maps tick_number -> list of events to process on that tick
    std::map<int, std::vector<EconomicEvent>> delayedEvents;

    // ========== Dependency Graph ==========
    DependencyGraph dependencyGraph;

    // ========== Tick Counter ==========
    int currentTick = 0;

    // ========== Safety ==========
    // Maximum propagation depth to prevent infinite cascading loops
    static constexpr int MAX_PROPAGATION_DEPTH = 5;

public:
    // ========== Constructor ==========
    EconomicPropagation(Simulation *sim);

    // ========== Rule Registration ==========

    // Register a single propagation rule
    // The rule is indexed by its trigger pattern for fast lookup
    void RegisterRule(const PropagationRule &rule);

    // Register ALL core economic rules at once
    // Called once during Simulation construction
    // This is where ALL economic relationships are defined
    void RegisterAllRules();

    // ========== Event Queueing ==========

    // Queue an event for processing
    // Events are processed in the next ProcessTierX() call
    void QueueEvent(const EconomicEvent &event);

    // Convenience: create and queue an event from a value change
    void EmitChange(const std::string &source,
                    const std::string &variable,
                    double oldValue, double newValue,
                    const std::string &description = "");

    // ========== Tier Processing ==========

    // Tier 1: IMMEDIATE effects (same tick)
    // Market clearing, price adjustments, cost recalculations
    // These happen instantly when supply/demand changes
    void ProcessTier1();

    // Tier 2: AGENT DECISION effects (next tick)
    // Farmers decide output, workers decide job search, firms decide investment
    // Based on information from the current tick
    void ProcessTier2();

    // Tier 3: DELAYED macro effects (2+ ticks)
    // Inflation manifestation, unemployment cascading, capital accumulation
    // Slow-moving systemic changes
    void ProcessTier3();

    // Process any delayed events that are now due
    void ProcessDelayedEvents();

    // ========== Queries ==========

    // Get all variables affected if this trigger fires
    std::vector<std::string> GetAffectedVariables(const std::string &trigger) const;

    // Get what causes a variable to change
    std::vector<std::string> GetVariableCauses(const std::string &variable) const;

    // Get event history (optionally filtered by variable)
    const std::vector<EconomicEvent> &GetFullHistory() const { return eventHistory; }
    std::vector<EconomicEvent> GetHistoryForVariable(const std::string &variable) const;

    // ========== Visualization & Debugging ==========

    // Print the dependency chain for a variable
    // Shows: what caused it, and what it causes, recursively
    void PrintDependencyChain(const std::string &variable) const;

    // Print all events from the current tick
    std::string GetEventLog(int forTick = -1) const;

    // Export the dependency graph as a Graphviz DOT file
    void ExportDependencyGraph(const std::string &filename) const;

    // Get summary stats
    int GetEventCount() const { return static_cast<int>(eventHistory.size()); }
    int GetCurrentTick() const { return currentTick; }
    int GetRuleCount() const { return static_cast<int>(rules.size()); }

    // ========== Tick Management ==========
    void AdvanceTick() { currentTick++; }

private:
    // ========== Internal Helpers ==========

    // Process events in the queue, filtering by tier
    void ProcessEventsForTier(int tier);

    // Apply a single rule to an event, potentially creating cascading events
    void ApplyRule(const PropagationRule &rule, const EconomicEvent &event);

    // Check if an event's variable matches a rule's trigger pattern
    // Supports wildcards: "farmer.*.output_quantity" matches "farmer.Shafin.output_quantity"
    bool MatchesTrigger(const std::string &eventVariable, const std::string &triggerPattern) const;

    // Parse a dot-separated variable name to extract entity info
    // "farmer.Shafin.output_quantity" -> ("farmer", "Shafin", "output_quantity")
    static std::vector<std::string> SplitVariable(const std::string &variable);
};
