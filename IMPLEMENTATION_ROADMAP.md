# Implementation Roadmap: Making cppConomy Fully Connected

## Quick Summary

Your current system has good entity structures but lacks interconnection. The solution is to create an **EconomicPropagation** engine that automatically cascades effects through the economy.

**Key Insight**: Separate *relationship logic* from *simulation control*.

---

## Phase-by-Phase Roadmap

### ⏱️ Timeline: 2-3 weeks

```
Week 1: Engine Foundation
├─ Create EconomicPropagation.h/cpp
├─ Define rule structure
├─ Register 5-10 core rules
└─ Test with simple examples

Week 2: Integration
├─ Refactor Simulation::Step()
├─ Connect propagator to simulation
├─ Add event queuing
└─ Test market clearing with propagation

Week 3: Polish & Examples
├─ Add remaining rules
├─ Create dependency visualization
├─ Document with examples
└─ Test complex scenarios (bumper harvest, tax changes, etc.)
```

---

## Current vs. Proposed

### BEFORE (Current State)

```cpp
// CLI executes commands
farmer.harvest();           // Changes output_quantity
market.recalculate();       // Updates price (maybe)
// That's it - no cascading effects
// Changes don't propagate to other variables
```

```
Farmer Harvest
    ↓
Market Price
    ↓ (stops here)
[Isolated]
```

### AFTER (Proposed State)

```cpp
// Simulation.Step() orchestrates everything
sim.Step();  // This now does:

// Phase 1: Agents act
farmer.Harvest();          // output_quantity → 1275

// Phase 2: Markets clear
market.ClearMarket();      // price equilibrium

// Phase 3: Propagate effects ← NEW!
propagator.ProcessTier1(); // price → demand
propagator.ProcessTier2(); // profit → labor decisions  
propagator.ProcessTier3(); // unemployment → consumption

// Phase 4: Government policy
government.ApplyPolicy();

// Phase 5: Stats
RefreshStats();
```

```
Farmer Harvest (output ↑)
    ↓ 
Market Supply ↑
    ↓
Market Price ↓
    ↓
Consumer Demand ↑
    ↓
Farmer Revenue ↓ (despite more output!)
    ↓
Farmer Profit ↓ (major!)
    ↓
Labor Hired ↓ (next tick)
    ↓
Unemployment ↑
    ↓
Consumer Budget ↓
    ↓
Market Demand ↓ (completing circle)
    ↓
GDP Growth ↓ (despite harvest!)

[FULLY CONNECTED - Realistic feedback loops]
```

---

## Detailed Implementation Steps

### Step 1: Create EconomicPropagation.h

**File**: `src/EconomicPropagation.h`

**What to include**:
- `EconomicEvent` struct (variable changes)
- `PropagationRule` struct (relationships)
- `DependencyGraph` class (visualization)
- `EconomicPropagation` class (main engine)

**Time**: ~2-3 hours

**Key sections**:
```cpp
struct EconomicEvent {
    string variable;
    double oldValue, newValue;
    int propagationLevel;  // 0=original, 1+=cascading
    int tick;
};

struct PropagationRule {
    string trigger;   // "farmer.*.output_quantity"
    string targets;   // "market.supply,market.price"
    PropagationFunction rule;
    double strength;
    int delay;
};

class EconomicPropagation {
public:
    void RegisterRule(const PropagationRule& rule);
    void QueueEvent(const EconomicEvent& event);
    void ProcessImmediateEffects();
    void ProcessAgentDecisions();
    void ProcessDelayedEffects();
};
```

**Checklist**:
- [ ] Create file with header guards
- [ ] Define EconomicEvent struct
- [ ] Define PropagationRule struct
- [ ] Define DependencyGraph class
- [ ] Define EconomicPropagation class
- [ ] Add Doxygen comments

---

### Step 2: Create EconomicPropagation.cpp

**File**: `src/EconomicPropagation.cpp`

**What to implement**:
- Constructor
- `RegisterRule()` - add rules to rules
- `RegisterAllRules()` - bulk rule registration
- Rule application logic
- Event processing (Tier 1, 2, 3)

**Time**: ~4-5 hours

**Core methods** (in order):
```cpp
// 1. Constructor
EconomicPropagation::EconomicPropagation(Simulation* sim)
    : simulation(sim) { }

// 2. Rule registration
void RegisterAllRules() {
    // Market Rules
    RegisterRule({
        trigger: "farmer.*.output_quantity",
        targets: "market.supply,market.price",
        rule: [](Simulation*, Event, target) { /* ... */ },
        strength: 1.0,
        delay: 0,
        isImmediate: true
    });
    
    // Labor Market Rules
    // Government Rules
    // ... (10-15 total rules)
}

// 3. Event processing
void ProcessImmediateEffects() {
    while (!eventQueue.empty()) {
        EconomicEvent event = eventQueue.front();
        eventQueue.pop();
        
        // Find matching rules
        // Apply only immediate rules
        // Queue new events
    }
}
```

**Important rules to implement**:
1. Supply change → Price change (law of supply)
2. Price change → Quantity demanded (law of demand)
3. Profit change → Labor hired (agent decision)
4. Unemployment change → Consumer budget (income effect)
5. Tax rate change → Worker wallet (fiscal policy)
6. Wage change → Production cost (cost structure)
7. Money supply change → Price level (inflation)
8. Interest rate change → Investment (monetary policy)
9. Minimum wage change → Unemployment (labor market friction)
10. Revenue change → Next period output (expectation)

**Checklist**:
- [ ] Implement constructor
- [ ] Implement RegisterRule()
- [ ] Implement RegisterAllRules() with all 10+ core rules
- [ ] Implement ProcessImmediateEffects()
- [ ] Implement ProcessAgentDecisions()
- [ ] Implement ProcessDelayedEffects()
- [ ] Implement ApplyRule() helper
- [ ] Implement event queueing
- [ ] Test each rule individually

---

### Step 3: Modify Simulation.h

**What to change**:
```cpp
private:
    unique_ptr<EconomicPropagation> propagator;
    int currentTick = 0;

public:
    // NEW method
    void Step();
    
    // NEW accessor
    EconomicPropagation* GetPropagator() { return propagator.get(); }
```

**Also fix**: Constructor should Initialize the propagator:
```cpp
Simulation::Simulation()
    : government(make_unique<Government>()),
      propagator(make_unique<EconomicPropagation>(this)),  // NEW
      ...
{
    propagator->RegisterAllRules();  // NEW
}
```

**Time**: ~30 minutes

**Checklist**:
- [ ] Add propagator member
- [ ] Add currentTick member
- [ ] Declare Step() method
- [ ] Declare GetPropagator() method
- [ ] Update constructor to initialize propagator

---

### Step 4: Implement Simulation::Step()

**File**: `src/Simulation.cpp`

**What to implement**:

```cpp
void Simulation::Step()
{
    // PHASE 1: AGENT ACTIONS
    for (auto& farmer : farmers) {
        farmer->Harvest();
        propagator->QueueEvent({
            .variable = "farmer." + farmer->GetName() + ".output_quantity",
            .oldValue = farmer->GetPreviousOutput(),
            .newValue = farmer->GetOutputQuantity(),
            // ...
        });
    }
    
    for (auto& consumer : workers) {
        for (auto& [productName, market] : markets) {
            consumer->UpdateQuantityDemanded(productName, market->GetCurrentPrice());
        }
    }
    
    // PHASE 2: MARKET CLEARING
    for (auto& [productName, market] : markets) {
        double totalDemand = AggregateConsumerDemand(productName);
        double totalSupply = AggregateProducerSupply(productName);
        
        market->SetSupplyAndDemand(totalSupply, totalDemand);
        market->RecalculateEquilibrium();
        
        if (market->GetCurrentPrice() != market->GetPreviousPrice()) {
            propagator->QueueEvent({
                .variable = "market." + productName + ".price",
                // ...
            });
        }
    }
    
    // PHASE 3: PROPAGATE EFFECTS
    propagator->ProcessImmediateEffects();
    propagator->ProcessAgentDecisions();
    propagator->ProcessDelayedEffects();
    
    // PHASE 4: GOVERNMENT POLICY
    government->ApplyMacroPolicy();
    
    // PHASE 5: STATISTICS
    RefreshStats();
    
    // ADVANCE TIME
    propagator->AdvanceTick();
    currentTick++;
    
    if (statsCallback) {
        statsCallback(stats);
    }
}
```

**Time**: ~2-3 hours

**Checklist**:
- [ ] Add Phase 1 (agent actions with event queueing)
- [ ] Add Phase 2 (market clearing with event queueing)
- [ ] Add Phase 3 (call all three propagation levels)
- [ ] Add Phase 4 (government policy)
- [ ] Add Phase 5 (statistics refresh)
- [ ] Add time advancement
- [ ] Test each phase independently

---

### Step 5: Add Agent Decision-Making

**Where**: Agent classes (Farmer.h/cpp, Worker.h/cpp, Owner.h/cpp)

**What to add**:

These methods allow agents to respond to economic signals:

```cpp
// In Farmer.h/cpp
public:
    void DecideNextOutputLevel();
    void DecideLabor Demand();
    void DecideInvestment();

// In Worker.h/cpp
public:
    void DecideJobSearch();
    void UpdateReservationWage();

// In Owner.h/cpp
public:
    void DecideProduction();
    void DecideInvestment();
    void DecideMarketExit();
```

**Implementation example**:
```cpp
void Farmer::DecideNextOutputLevel()
{
    // Last period's profit determines this period's production goal
    double profitMargin = lastPeriodProfit / lastPeriodRevenue;
    
    if (profitMargin < 0.15) {  // Less than 15% profit
        targetOutput *= 0.9;   // Reduce next period by 10%
    } else if (profitMargin > 0.30) {
        targetOutput *= 1.1;   // Increase next period by 10%
    }
    
    // Clamp to land capacity
    targetOutput = min(targetOutput, land_size * technology_level);
}
```

**Time**: ~2-3 hours

**Checklist**:
- [ ] Add DecideNextOutputLevel() to Farmer
- [ ] Add DecideLabor Demand() to Farmer
- [ ] Add DecideJobSearch() to Worker
- [ ] Call these in appropriate propagation phases
- [ ] Test that decisions respond to prices/profits

---

### Step 6: Create Visualization & Testing

**What to add**:
- Export dependency graph to Graphviz format
- Print event chains for debugging
- Create test scenarios

**Files**:
- Methods in `EconomicPropagation.cpp`:
  - `void ExportDependencyGraph(const string& filename)`
  - `void PrintEventHistory()`
  - `void PrintDependencyChain(const string& variable)`

**Example test**:
```cpp
void TestBumperHarvest()
{
    Simulation sim;
    sim.Initialize(5, 1, 1);
    
    // Set up bumper harvest
    Farmer* farmer = sim.FindFarmer("Farmer1");
    farmer->SetWeatherFactor(1.5);  // 50% better
    
    // Run one step
    sim.Step();
    
    // Check propagation
    cout << "Events this tick:\n";
    sim.GetPropagator()->PrintEventHistory();
    
    // Export dependency
    sim.GetPropagator()->ExportDependencyGraph("bumper_harvest.dot");
}
```

**Time**: ~1-2 hours

**Checklist**:
- [ ] Implement ExportDependencyGraph()
- [ ] Implement PrintEventHistory()
- [ ] Create test scenario: Bumper Harvest
- [ ] Create test scenario: Tax Increase
- [ ] Create test scenario: Wage Increase
- [ ] Visualize each scenario

---

## Priority Rules (Implement First)

**Essential (Do First)**:
1. ✅ Supply → Price (law of supply)
2. ✅ Price → Quantity Demanded (law of demand)
3. ✅ Unemployment → Consumer Budget
4. ✅ Wage → Production Cost
5. ✅ Profit → Labor Decision

**Important (Do Next)**:
6. Tax → Worker Wallet
7. Money Supply → Inflation
8. Interest Rate → Investment
9. Minimum Wage → Unemployment
10. Revenue → Next Period Output

**Nice-to-Have**:
11. Literacy → Productivity
12. Child Labor → Income Inequality
13. Inflation → Wage Expectations
14. Debt → Future Government Spending

---

## Testing Strategy

After each step, test:

```cpp
// Test 1: Single rule application
void test_supply_to_price() {
    Market m("Rice");
    m.SetSupply(100);
    ASSERT_EQ(m.GetPrice(), 10.0);
    
    m.SetSupply(150);  // +50%
    ASSERT_LT(m.GetPrice(), 10.0);  // Price should fall
}

// Test 2: Multi-rule propagation
void test_bumper_harvest() {
    Simulation sim;
    sim.Initialize(5, 1, 1);
    
    double priceBeforestart.GetSelectedMarket()->GetCurrentPrice();
    
    sim.GetSelectedFarmer()->SetWeatherFactor(1.5);
    sim.Step();
    
    double priceAfter = sim.GetSelectedMarket()->GetCurrentPrice();
    ASSERT_LT(priceAfter, priceBefore);  // Price fell due to harvest
}

// Test 3: Feedback loops
void test_bumper_harvest_catastrophe() {
    // Bumper harvest → price falls → profit falls → layoffs → unemployment
    Simulation sim;
    sim.Initialize(10, 2, 1);
    
    int employedBefore = sim.GetStats().employed;
    
    for (int t = 0; t < 5; t++) {  // Run for 5 ticks
        sim.GetSelectedFarmer()->SetWeatherFactor(1.5);
        sim.Step();
    }
    
    int employedAfter = sim.GetStats().employed;
    ASSERT_LT(employedAfter, employedBefore);  // Should have more unemployment
}
```

---

## Compilation & Build

```bash
# Windows (with compiler of your choice)
g++ -std=c++17 -o cppConomy src/*.cpp

# If using CMake (recommended for bigger projects)
cmake -B build
cmake --build build
```

Your existing build should have no issues since you're adding new files.

---

## Next Steps After Implementation

1. **Add More Entities**: Multiple markets, firms, regions
2. **Add Policy Tools**: Tax brackets, interest rate changes, subsidies
3. **Add Macroeconomic Models**:
   - Phillips curve (inflation-unemployment tradeoff)
   - IS-LM model (output-inflation dynamics)
   - Growth models (capital accumulation)
4. **Add Realistic Behaviors**:
   - Forward-looking expectations
   - Bounded rationality
   - Learning and adaptation
5. **Educational Features**:
   - Scenario templates (Great Depression, Oil Shock, etc.)
   - Lesson plans with specific learning objectives
   - Visualization of "what-if" scenarios

---

## Success Metrics

✅ When complete, you should be able to:

1. **Run a simulation** that shows realistic economic dynamics
2. **Trace any variable change** back to its root causes
3. **Observe feedback loops** (bumper harvest → unemployment → recession)
4. **Modify rules** without touching core simulation code
5. **Export dependency graphs** showing how variables relate
6. **Predict second-order effects** (tax increase → demand drop → recession)
7. **See inflation**, unemployment, GDP growth emerge from interactions
8. **Understand trade-offs** (minimum wage helps workers but causes unemployment)

---

## Questions to Answer While Implementing

1. **How should delays work?** (Τakes how many ticks for effect to appear?)
2. **How should strength decay?** (Does a second-order effect matter less?)
3. **What about equilibrium delays?** (Do markets adjust instantly or gradually?)
4. **How to handle conflicting rules?** (E.g., wage ↑ and unemployment ↑ happening simultaneously)
5. **What's the propagation depth limit?** (Stop at level 5? 10? Infinite?)

---

## Files to Create/Modify

```
NEW FILES:
src/EconomicPropagation.h       (+500 lines)
src/EconomicPropagation.cpp     (+800 lines)

MODIFIED FILES:
src/Simulation.h                (+5 lines)
src/Simulation.cpp              (+100 lines - new Step() method)
src/Farmer.h/cpp                (+50 lines - decision methods)
src/Worker.h/cpp                (+30 lines - reservation wage)
src/Owner.h/cpp                 (+30 lines - investment decision)

UNCHANGED:
All other files work as-is
```

---

## Estimated Effort

- **Total Time**: 20-30 hours
- **Week 1**: 8 hours
- **Week 2**: 12 hours
- **Week 3**: 5-10 hours (testing, polish, visualization)

---

This is everything you need to transform cppConomy from a collection of isolated agents into a fully connected, dynamic economic system! Good luck! 🎉
