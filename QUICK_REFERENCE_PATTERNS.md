# Quick Reference: Economic Propagation Patterns

## Common Relationship Patterns

### Pattern 1: Direct Proportional Relationship

**When**: Variable A change causes Variable B to change in same direction/magnitude

```cpp
RegisterRule({
    .trigger = "farmer.*.labor_hired",
    .targets = "production.output",
    .rule = [](Simulation* sim, const Event& e, const string& target) {
        // More labor → more output (all else equal)
        double laborChangeRatio = e.newValue / e.oldValue;
        return event.oldValue * laborChangeRatio * 1.2;  // 1.2 = base productivity
    },
    .strength = 1.0,
    .delay = 0,
    .isImmediate = true
});
```

**Economics**: Marginal product of labor (constant or diminishing)

---

### Pattern 2: Inverse Relationship

**When**: Variable A increases → Variable B decreases

```cpp
RegisterRule({
    .trigger = "market.*.price",
    .targets = "market.quantity_demanded",
    .rule = [](Simulation* sim, const Event& e, const string& target) {
        // Higher price → lower quantity demanded (elasticity = -0.8)
        double priceChangeRatio = e.newValue / e.oldValue;
        double elasticity = -0.8;
        double demandChangeRatio = priceChangeRatio + (priceChangeRatio - 1.0) * elasticity;
        return e.oldValue * demandChangeRatio;
    },
    .strength = 0.9,
    .delay = 0,
    .isImmediate = true
});
```

**Economics**: Law of demand (∂Qd/∂P < 0)

---

### Pattern 3: Conditional Relationship

**When**: Effect only triggers if condition is met

```cpp
RegisterRule({
    .trigger = "government.minimuwage",
    .targets = "labor_market.unemployment",
    .rule = [](Simulation* sim, const Event& e, const string& target) {
        double minWage = e.newValue;
        double equilibriumWage = 400.0;
        
        // ONLY create unemployment if minimum wage BINDS (above equilibrium)
        if (minWage <= equilibriumWage) {
            return e.oldValue;  // No effect
        }
        
        // Excess supply = unemployment
        double excess = minWage - equilibriumWage;
        double unemployment = e.oldValue + (excess / equilibriumWage) * 0.3;
        return min(unemployment, 0.99);  // Cap at 99%
    },
    .strength = 0.7,
    .delay = 1,
    .isImmediate = false
});
```

**Economics**: Binding vs non-binding constraints

---

### Pattern 4: Distributed/Aggregate Relationship

**When**: One variable affects multiple consumers/firms

```cpp
RegisterRule({
    .trigger = "labor_market.unemployment_increase",
    .targets = "consumer.*.budget",
    .rule = [](Simulation* sim, const Event& e, const string& target) {
        // Parse target: consumer.John.budget  
        double unemploymentIncrease = e.newValue - e.oldValue;
        
        // Each 1% unemployment = that many workers lose income
        double avgWage = 500.0;
        double avgHoursPerMonth = 160.0;
        
        double budgetLossPerWorker = avgWage * (unemploymentIncrease / 100.0);
        
        return e.oldValue - budgetLossPerWorker;  // Consumer loses money
    },
    .strength = 1.0,
    .delay = 0,
    .isImmediate = false
});
```

**Economics**: Aggregate demand, multiplier effects

---

### Pattern 5: Time-Delayed Response

**When**: Effect takes multiple periods to manifest

```cpp
RegisterRule({
    .trigger = "government.money_supply",
    .targets = "market.*.price",
    .rule = [](Simulation* sim, const Event& e, const string& target) {
        // Quantity Theory: MV = PQ
        // If M ↑ 10%, P tends to ↑ 10% (if V, Q constant)
        // But this effect is SLOW due to sticky prices
        
        double moneySupplyChangePercent = (e.newValue / e.oldValue) - 1.0;
        double priceAdjustmentFraction = 0.3;  // Only 30% adjustment this period
        
        return e.oldValue * (1.0 + moneySupplyChangePercent * priceAdjustmentFraction);
    },
    .strength = 0.7,
    .delay = 2,  // Takes 2 ticks for inflation to show
    .isImmediate = false
});
```

**Economics**: Sticky prices, gradual inflation adjustment

---

### Pattern 6: Feedback Loop

**When**: Variable affects another which affects the first (circular)

```
Example: Price → Output → Supply → Price

RegisterRule({
    .trigger = "market.*.price",
    .targets = "producer.output_decision",
    
    // Next tick triggers:
    RegisterRule({
        .trigger = "producer.output_change",
        .targets = "market.supply",
        
        // Which triggers:
        RegisterRule({
            .trigger = "market.supply",
            .targets = "market.price",  // Back to original!
```

**Economics**: Supply and demand equilibration, cobweb theory

---

### Pattern 7: Magnitude-Sensitive

**When**: Effect depends on size of change, not just direction

```cpp
RegisterRule({
    .trigger = "farmer.*.profit",
    .targets = "farmer.*.labor_hired",
    .rule = [](Simulation* sim, const Event& e, const string& target) {
        double profitChange = e.newValue / e.oldValue;
        
        // Small loss (-10%): reduce labor by 5%
        // Medium loss (-50%): reduce labor by 30%
        // Large loss (-80%): reduce labor by 70%
        
        if (profitChange >= 1.0) {
            // Profit increased - maybe hire more
            return e.oldValue * (1.0 + (profitChange - 1.0) * 0.5);
        } else {
            // Profit decreased - proportional labor cut
            double lossMagnitude = 1.0 - profitChange;  // e.g., 0.80
            return e.oldValue * (1.0 - lossMagnitude * 0.4);  // Cut 40% of the loss
        }
    },
    .strength = 0.6,
    .delay = 1,
    .isImmediate = false
});
```

**Economics**: Firm behavior under uncertainty

---

## Common Economic Relationships

### Labor Market

```cpp
// Wage → Employment (inverse)
wage ↑ → quantity_supplied ↑ (workers want more jobs)
wage ↑ → quantity_demanded ↓ (firms hire less)

// Wage-Price Spiral
wage ↑ → production_cost ↑ → price ↑ → inflation ↑ 
    → wage_expectations ↑ → wage_demands ↑ (back to start!)

// Unemployment Effects   
unemployment ↑ → consumer_budget ↓ → demand ↓ → output ↓ → unemployment ↑↑
```

### Product Market

```cpp
// Supply and Demand
supply ↑ → price ↓ → quantity_demanded ↑ → revenue ↓ (if elasticity > 1)
demand ↑ → price ↑ → quantity_supplied ↑

// Production Decision
profit ↓ → future_output ↓ → supply ↓ → price ↑ (eventual recovery)
profit ↓ → investment ↓ → capital_stock ↓ → productivity ↓ (long-run stagnation)
```

### Government/Macro

```cpp
// Fiscal Policy
tax ↑ → disposable_income ↓ → consumption ↓ → demand ↓
tax_collected ↑ → budget ↑  (when elasticity effects small)
tax_collected ↓ → budget ↓  (tax incidence on quantity)

// Monetary Policy
money_supply ↑ → inflation ↑ → wage_expectations ↑ → actual_wages ↑
interest_rate ↑ → investment ↓ → capital_accumulation ↓ → long_run_growth ↓
```

---

## Testing Your Rules

### Template for Testing

```cpp
void test_rule_name()
{
    Simulation sim;
    sim.Initialize(5, 2, 1);
    
    // Get initial state
    Market* market = sim.FindMarket("Rice");
    double priceBefore = market->GetCurrentPrice();
    double supplyBefore = market->GetQuantitySupplied();
    
    // Trigger the rule
    Farmer* farmer = sim.GetSelectedFarmer();
    farmer->SetOutputQuantity(supplyBefore + 50);  // +50 units supply
    
    // Run propagation
    sim.Step();
    
    // Check results
    double priceAfter = market->GetCurrentPrice();
    double supplyAfter = market->GetQuantitySupplied();
    
    // Assert expected behavior
    ASSERT_LT(priceAfter, priceBefore);  // Supply ↑ → Price ↓
    ASSERT_EQ(supplyAfter, supplyBefore + 50);
    
    // Check propagation chain
    sim.GetPropagator()->PrintEventHistory();
    sim.GetPropagator()->PrintDependencyChain("market.Rice.price");
    
    cout << "TEST PASSED: Supply increase causes price decrease\n";
}
```

---

## Strength Parameter Guidelines

Control how strong an effect is relative to the theoretical maximum:

```
strength = 1.0   : Full economic effect
           0.8   : Effect is 80% of theoretical (realistic)
           0.5   : Effect is 50% of theoretical (delays adjustment)
           0.2   : Weak effect (small impact)
           
Example: Wage elasticity of labor supply = 0.15
  (A 10% wage increase causes only 1.5% more labor supplied)

RegisterRule({
    .trigger = "labor_market.wage",
    .targets = "labor_market.quantity_supplied",
    .rule = [...],
    .strength = 0.15,  // This IS the wage elasticity!
});
```

---

## Delay Parameter Guidelines

How many ticks before effect manifests:

```
delay = 0   : Immediate (price adjustments, market clearing)
        1   : Next tick (agent decisions based on new info)
        2   : Two ticks later (behavioral adjustments)
        3+  : Long-run effects (capital accumulation, skill development)

Pattern:
├─ delay=0   : Market supply matches demand, prices adjust
├─ delay=1   : Farmers see new prices, decide output for next season
├─ delay=2   : Labor market adjusts (hiring/firing takes time)
└─ delay=3+  : Long-run productivity changes, capital investment payoff
```

---

## Common Anti-Patterns to Avoid

### ❌ Don't: Hard-code values

```cpp
// BAD - values hidden
.rule = [](Simulation*, const Event& e, const string&) {
    return e.oldValue * 0.7;  // Where does 0.7 come from?
}
```

### ✅ Do: Extract to variables with economic meaning

```cpp
// GOOD - values have meaning
const double demandElasticity = -0.8;
const double priceAdjustmentSpeed = 0.7;
.rule = [elasticity=demandElasticity, speed=priceAdjustmentSpeed]
         (Simulation*, const Event& e, const string&) {
    return e.oldValue * (1.0 + elasticity * (e.newValue - e.oldValue) * speed);
}
```

---

### ❌ Don't: Create rules in Simulation class

```cpp
// BAD - logic scattered everywhere
void Simulation::Step() {
    // Apply rule here
    farmer->output *= 1.1;  // Mixed with control flow
}
```

### ✅ Do: Centralize rules in EconomicPropagation

```cpp
// GOOD - all relationships in one place
void EconomicPropagation::RegisterAllRules() {
    RegisterRule({...});  // All rules here
}
```

---

### ❌ Don't: Create infinite loops

```cpp
// BAD - A→B→A with delay=0 causes infinite loop?
RegisterRule({ trigger: "price", targets: "supply" });
RegisterRule({ trigger: "supply", targets: "price", delay: 0 });  // PROBLEM!
```

### ✅ Do: Use appropriate delays to break loops

```cpp
// GOOD - loop is broken by delays
RegisterRule({ 
    trigger: "price", 
    targets: "producer.output_decision", 
    delay: 0,  // Producer responds immediately
    isImmediate: false  // But in Tier 2 (agent decisions)
});

RegisterRule({ 
    trigger: "producer.output_change", 
    targets: "supply", 
    delay: 1,  // Supply change happens next tick
    isImmediate: false
});

RegisterRule({ 
    trigger: "supply", 
    targets: "price", 
    delay: 0,  // Price adjusts immediately
    isImmediate: true  // In Tier 1 (market clearing)
});
```

The delays ensure the loop converges rather than oscillating infinitely.

---

## Debugging Tools

### Print event history
```cpp
sim.GetPropagator()->PrintEventHistory();
// Output:
// [TICK 120] farmer.Shafin.output: 850 → 1275 (Level 0)
// [TICK 120] market.Rice.supply: 500 → 650 (Level 1)
// [TICK 120] market.Rice.price: 5.0 → 3.8 (Level 2)
// [TICK 121] farmer.Shafin.labor: 10 → 7 (Level 0) ← next tick decision
```

### Print dependency chain
```cpp
sim.GetPropagator()->PrintDependencyChain("market.Rice.price");
// Output:
// "market.Rice.price" is affected by:
// ├─ farmer.*.output_quantity (supply)
// ├─ government.money_supply (inflation)
// └─ consumer.*.wallet (demand via income effect)
//
// "market.Rice.price" affects:
// ├─ consumer.rice.quantity_demanded
// ├─ producer.profit
// └─ future.producer.labor_hired
```

### Export dependency graph
```cpp
sim.GetPropagator()->ExportDependencyGraph("economy.dot");
// Then visualize:
// dot -Tpng economy.dot -o economy.png
```

---

## When to Use Each Tier

### Tier 1: Immediate Effects (same tick)

Use for **instantaneous** economic adjustments:
- Market clearing (supply meets demand)
- Price discovery
- Revenue calculations (P × Q)
- Cost calculations
- Any direct mathematical relationship

```cpp
.isImmediate = true,
.delay = 0
```

### Tier 2: Agent Decisions (next tick)

Use for **behavioral** responses that take time:
- Production output decisions (farms, factories)
- Hiring/firing decisions
- Investment decisions
- Saving vs consumption
- Job search

```cpp
.isImmediate = false,
.delay = 1
```

### Tier 3: Delayed/Systemic Effects (2+ ticks)

Use for **slow-moving** macroeconomic changes:
- Unemployment effects (takes time to find jobs)
- Inflation manifestation (sticky prices)
- Capital accumulation (investment payoff)
- Skill development (education returns)
- Long-run growth effects

```cpp
.isImmediate = false,
.delay = 2 or 3
```

---

## Example: Complete Economic Scenario

### Scenario: "Oil Price Shock"

```cpp
void simulate_oil_shock() {
    Simulation sim;
    sim.Initialize(10, 5, 3);
    
    // Baseline: run 20 ticks normally
    for (int t = 0; t < 20; t++) sim.Step();
    
    double gdpBefore = sim.GetStats().gdp;
    double employmentBefore = sim.GetStats().employed;
    
    // SHOCK: Oil prices triple (cost input becomes expensive)
    Government* gov = sim.GetGovernment();
    gov->SetInputCost("oil", 300.0);  // Was 100
    
    // Run simulation forward 30 more ticks
    for (int t = 20; t < 50; t++) {
        sim.Step();
        
        if (t % 5 == 0) {
            cout << "TICK " << t << ": GDP=" << sim.GetStats().gdp 
                      << " Employment=" << sim.GetStats().employed << "\n";
            sim.GetPropagator()->PrintEventHistory();
        }
    }
    
    // Check results
    double gdpAfter = sim.GetStats().gdp;
    double employmentAfter = sim.GetStats().employed;
    
    cout << "\nOIL SHOCK EFFECTS:\n";
    cout << "GDP fell: " << (gdpBefore - gdpAfter) << "\n";
    cout << "Employment fell: " << (employmentBefore - employmentAfter) << "\n";
    cout << "Inflation rate: " << sim.GetStats().inflation << "\n";
    
    // Visualize how shock propagated
    sim.GetPropagator()->ExportDependencyGraph("oil_shock.dot");
}
```

This shows:
1. Initial equilibrium (ticks 0-20)
2. Shock applied (tick 20)
3. Immediate effects (tier 1: costs ↑, prices ↑)
4. Agent responses (tier 2: output ↓, hiring ↓)
5. Systemic effects (tier 3: unemployment ↑, demand ↓, recession)

---

## Performance Considerations

### Rule Processing

```
Event Queue Size: O(n) where n = rules triggered per tick
Typical: 10-50 events per tick
Max Propagation Depth: 5-10 levels

Processing Time: ~1-5ms per tick on modern CPU
Memory: ~1MB for entire economy history
```

### Optimization tips

```cpp
// Use pattern matching sparingly
.trigger = "farmer.*.output_quantity";  // OK
.trigger = "*.*.*.output_quantity";     // Avoid - too general

// Use delay > 0 to reduce same-tick processing
.delay = 0;   // Processes immediately
.delay = 1;   // Processes next tick (smoother distribution)

// Set strength < 1.0 to reduce cascading depth
.strength = 1.0;  // Full effect
.strength = 0.5;  // Effect halves at each level (converges faster)
```

---

This guide covers the patterns you'll use 90% of the time when building economic relationships!
