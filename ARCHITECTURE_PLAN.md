# cppConomy: Fully Connected Economic System Architecture

## Current State Assessment

### Strengths
✅ Good entity structure (Workers, Farmers, Owners, Market, Government)  
✅ VariableTracker infrastructure partially in place  
✅ Some relationships defined (farmer harvest → market supply)  
✅ Market equilibrium calculations exist  

### Weaknesses
❌ Limited feedback loops - changes don't cascade fully  
❌ Agent behavior is static - doesn't respond to economic signals  
❌ Policy effects are isolated - don't propagate through economy  
❌ No decision-making based on prices, wages, profits  
❌ Market results don't influence production decisions  

---

## Vision: Fully Connected System

Create an **Economic Propagation Engine** that ensures:
> **Every economic action creates ripple effects through the entire system**

Examples of desired connections:
- **Farmer harvests** → supply increases → price falls → consumers demand more → farmer sees lower revenue → stops hiring → worker unemployed → consumption falls → overall demand decreases
- **Tax increase** → worker income falls → consumption decreases → market demand falls → business profits decline → owners invest less → unemployment rises
- **Bumper harvest** → surplus goods → prices crash → farmer profit falls despite high output → may reduce next harvest instead
- **Wage increase** → production costs rise → firms raise prices → inflation → government must increase money supply → inflation accelerates

---

## Architecture: Economic Propagation System

### 1. **EconomicPropagation.h/cpp** (NEW - Separate File)

This file handles ALL inter-variable relationships. It's a state-agnostic relationship engine.

```cpp
// EconomicPropagation.h

struct EconomicEvent
{
    string source;           // "farmer.harvest", "government.tax_increase", etc.
    string variable;         // What changed
    double oldValue, newValue;    // The change magnitude
    int propagationLevel = 0;     // How many steps deep (0=original, 1+=cascading)
};

struct PropagationRule
{
    string trigger;          // "farmer.harvest", "market.price_change", etc.
    string target;           // What gets affected
    function<void(...)> rule; // Lambda with the calculation
    double strength = 1.0;        // 0-1, how much influence
    int delay = 0;               // Ticks before effect applies
};

class EconomicPropagation
{
private:
    vector<PropagationRule> rules;
    queue<EconomicEvent> pendingEvents;
    map<string, vector<string>> dependencyGraph;

public:
    // Register a propagation rule
    void RegisterRule(const PropagationRule& rule);
    
    // Queue an event to propagate
    void QueueEvent(const EconomicEvent& event);
    
    // Process one level of propagation (call in simulation step)
    void ProcessOneLevel();
    
    // Get all affected vars from a trigger
    vector<string> GetAffectedVariables(const string& trigger);
    
    // Visualize dependency chain
    void PrintDependencyChain(const string& trigger);
};
```

### 2. **Three-Tier Propagation Architecture**

```
TIER 1: IMMEDIATE EFFECTS (same tick)
├─ Farmer harvests → Market supply updated
├─ Market supply changes → Recalculate equilibrium price
├─ Tax rate changes → Recalculate government revenue
└─ Price changes → Recalculate quantity demanded

TIER 2: AGENT DECISIONS (next tick)
├─ Farmer sees low price → Reduces next harvest plan
├─ Consumer sees high price → Reduces quantity demanded
├─ Worker sees high wages → Accepts more jobs
├─ Owner sees high profit → Invests in expansion
└─ Government sees deficit → Adjusts spending/taxes

TIER 3: SYSTEMIC EFFECTS (2-3 ticks later)
├─ Unemployment ↑ → Consumer demand falls → Prices fall → Production cuts
├─ Investment ↓ → Productive capacity shrinks → Supplies fall → Prices rise
├─ Inflation ↑ → Government increases money supply → Inflation worsens
└─ Real wage ↓ → Literacy investment falls → Productivity growth stops
```

---

## Implementation Strategy

### Phase 1: Create Propagation Engine

**File**: `EconomicPropagation.h/cpp`

**Core Features**:
```cpp
// Core Propagation Rules (examples)

// MARKET EQUILIBRIUM CHAIN
PropagationRule priceDropRule = {
    trigger: "market.supply_increase",
    target: "market.price",
    rule: (supply_change) => {
        // Law of supply: more supply → lower price
        newPrice = oldPrice * (1 - supply_change * priceElasticity)
    }
};

// DEMAND RESPONSE
PropagationRule priceToQuantityRule = {
    trigger: "market.price_change",
    target: "consumer.quantity_demanded",
    rule: (price_change) => {
        // Law of demand: higher price → lower quantity demanded
        newQd = oldQd * (1 + price_change * demandElasticity)
    }
};

// FARMER REVENUE → PRODUCTION DECISION
PropagationRule revenueToLaborRule = {
    trigger: "farmer.revenue_change",
    target: "farmer.labor_hired",
    rule: (revenue) => {
        // If profit fell, hire fewer workers next period
        if (revenue < expectedRevenue) {
            laborHired *= 0.9; // Reduce by 10%
        }
    }
};

// UNEMPLOYMENT → CONSUMPTION
PropagationRule unemploymentToDemandRule = {
    trigger: "labor_market.unemployment_increase",
    target: "consumer.budget",
    rules: (unemployment_increase) => {
        // Each 1% unemployment → some % of consumers lose income
        budgetReduction = totalWages * unemploymentRate * (1 - reemploy_rate)
    }
};
```

### Phase 2: Simulation Loop Refactor

Current loop (implicit/missing):
```
Initialize
└─ Tick Loop:
   ├─ (agents act somehow?)
   └─ RefreshStats
```

**New Loop**:
```cpp
void Simulation::Step()
{
    // PHASE 1: AGENT ACTIONS (independent)
    for (auto& farmer : farmers) {
        farmer.Harvest();  // Creates harvest production
    }
    for (auto& worker : workers) {
        worker.ConsumeBudget();  // Demand goods
    }
    
    // PHASE 2: MARKET REACTIONS (market clears based on demands)
    UpdateConsumerDemand();      // Consumers reveal demand at current prices
    for (auto& market : markets) {
        market.ClearMarket();     // Match supply/demand, find equilibrium
    }
    
    // PHASE 3: PROPAGATION (cascade effects through economy)
    propagator.ProcessTier1();   // Equilibrium calculations, tax effects
    propagator.ProcessTier2();   // Agent decision-making
    propagator.ProcessTier3();   // Delayed systemic effects
    
    // PHASE 4: POLICY APPLICATION
    government.ApplyMacroPolicy();  // Inflation, interest rates, etc.
    
    // PHASE 5: STATISTICS
    RefreshStats();
    timestep++;
}
```

---

## Detailed Connection Map

### SUPPLY-SIDE CONNECTIONS

```
Farmer Actions:
┌─ Harvest Output
│  ├─ → Market Supply
│  │   ├─ → Equilibrium Price (↓)
│  │   └─ → Quantity Demanded (↑)
│  └─ → Farmer Revenue (P × Q)
│     ├─ → Farmer Profit (Revenue - Costs)
│     │   └─ → Production Decision for next period
│     └─ → Worker Wages (if hiring)
│
└─ Technology/Input Changes
   └─ → Production Function Output
      └─ (cycle repeats)
```

### DEMAND-SIDE CONNECTIONS

```
Consumer Actions:
┌─ Wallet (disposable income)
│  ├─ Determined by: Wages - Taxes - Savings
│  ├─ → Quantity Demanded
│  │   └─ → Market Demand Curve
│  │       ├─ → Market Equilibrium
│  │       └─ → Price Discovery
│  └─ → Addictions/Preferences
│     └─ → Demand Shift
│
└─ Willingness to Pay
   └─ → Price Ceiling
      └─ → Markets with binding constraints
```

### LABOR MARKET CONNECTIONS

```
Wage Setting:
┌─ Labor Supply (workers wanting jobs)
│  └─ Affected by: Minimum Wage, Reservation Wage
│
├─ Labor Demand (farmers/owners needing workers)
│  └─ Affected by: Profit, Interest Rate (cost of capital)
│
└─ Equilibrium Wage = Supply ∩ Demand
   ├─ → Worker Wallet (monthly income)
   │   └─ → Consumer Demand
   │       └─ → GDP (C component)
   ├─ → Firm Production Costs
   │   └─ → Profit Level
   │       └─ → Investment Decisions
   └─ → Unemployment (if wage > equilibrium)
       └─ → Tax Revenue (fewer employed)
```

### GOVERNMENT POLICY CONNECTIONS

```
Tax Policy:
┌─ Income Tax Rate ↑
│  └─ → Worker Wallet ↓
│     └─ → Consumer Demand ↓
│        └─ → Market Demand ↓
│           └─ → Producer Revenue ↓
│              └─ → Production Capacity ↓
│
├─ Tax Revenue ↑ (more workers, higher rates)
│  └─ → Government Budget Balance
│  
└─ During Deficit:
   └─ Borrow or Print Money?
      ├─ Borrow → Interest Rates ↑ → Investment ↓
      └─ Print Money → Money Supply ↑ → Inflation ↑

Monetary Policy:
├─ Money Supply Change
│  └─ (Quantity Theory: MV = PQ)
│     └─ If V,Q constant → Price Level ↑
│
└─ Interest Rate Change
   ├─ → Loan Affordability
   │  └─ → Investment Decisions (farmers hiring, owners expanding)
   ├─ → Savings Attractiveness
   │  └─ → Consumer Spending vs Saving
   └─ → Inflation Expectations
      └─ → Wage Demands
```

### FEEDBACK LOOPS (The Interesting Parts!)

```
SCENARIO: Bumper Harvest
1. Harvest ↑     (weather factor = 1.5)
2. Supply ↑      (market.supply = 150 units)
3. Price ↓       (law of supply: P drops 30%)
4. Farmer Revenue ↓  (despite Q↑, P↓↓ = lower total)
5. Profit ↓      (costs same, revenue lower)
6. Labor Hired ↓ (next period, "can't afford it")
7. Unemployment ↑ (workers laid off)
8. Consumer Budget ↓ (unemployed workers have no income)
9. Demand ↓      (consumers can't buy as much)
10. Prices ↓ more (downward spiral)
```

```
SCENARIO: Tax Increase
1. Income Tax ↑        (from 20% to 30%)
2. Worker Wallet ↓     (same paycheck, less take-home)
3. Consumption ↓       (can't afford luxuries)
4. Market Demand ↓     (especially non-essentials)
5. Producer Revenue ↓  (sells less)
6. Profit ↓            (higher costs, lower revenue)
7. Investment ↓        (can't afford to expand)
8. Unemployment ↑      (businesses shrink, lay off workers)
9. Tax Revenue ↓       (fewer workers, lower total income)
10. Government Deficit ↑ (paradox: higher tax rate, lower revenue!)
```

```
SCENARIO: Minimum Wage Increase
1. Minimum Wage ↑      (from 5 to 8)
2. Labor Cost ↑        (firms pay more per worker)
3. Can Hire Fewer Workers  (same budget, fewer employees)
4. Unemployment ↑      (workers getting cut)
5. Consumer Budget ↓   (unemployed have no income)
6. But Employed Workers Win! (earn more)
   ├─ If income effect > substitution effect → Consumption ↑
   └─ If substitution effect > income effect → Prices rise → Not much better
```

---

## Implementation Checklist

### Step 1: Create EconomicPropagation Engine
- [ ] Create `EconomicPropagation.h`
- [ ] Define `PropagationRule` struct
- [ ] Create rule queue/processing system
- [ ] **Build dependency graph visualization**

### Step 2: Refactor Simulation Loop
- [ ] Extract phase 1 (agent actions) - `DoAgentDecisions()`
- [ ] Extract phase 2 (market clearing) - `ClearMarkets()`
- [ ] Add phase 3 (propagation) - `PropagateEffects()`
- [ ] Ensure government is applied before stats

### Step 3: Register Core Propagation Rules
Market Rules:
- [ ] `Supply change → Price change`
- [ ] `Price change → Quantity demanded change`
- [ ] `Revenue change → Profit change`

Labor Market Rules:
- [ ] `Unemployment ↑ → Consumer budget ↓`
- [ ] `Wage ↑ → Production cost ↑`
- [ ] `Profit ↑ → Labor demand ↑`

Government Rules:
- [ ] `Tax ↑ → Workers' wallet ↓`
- [ ] `Money supply ↑ → Price level ↑`
- [ ] `Interest rate ↑ → Investment ↓`

### Step 4: Add Agent Decision-Making
- [ ] Farmer: `DecideNextHarvest()` based on last-period profit
- [ ] Owner: `DecideInvestment()` based on profit and interest rate
- [ ] Worker: `DecideJobSearch()` based on wage and alternatives
- [ ] Consumer: `UpdateConsumption()` based on budget and preferences

### Step 5: Add Feedback Loop Examples
- [ ] Implement "Bumper Harvest Paradox"
- [ ] Implement "Tax Policy Adjustment"
- [ ] Implement "Wage Setting with Unemployment Effects"

---

## Key Design Principles

### 1. **Separation of Concerns**
- `EconomicPropagation.h` = **relationship logic only**
- `Simulation.h` = **orchestration of simulation steps**
- Entity classes = **state and basic actions**

### 2. **Rule-Based System**
- No hardcoding of relationships in Simulation class
- All connections defined in `EconomicPropagation`
- Easy to modify/add relationships without touching Simulation

### 3. **Tiered Execution**
- Immediate effects (same tick): prices adjust
- Agent decisions (next tick): agents respond to prices
- Delayed effects (2-3 ticks): macroeconomic shifts

### 4. **Traceability**
- Every effect has a documented rule
- Can output "dependency chain": Why did unemployment go up?
- Answer: Tax ↑ → Wage ↓ → Production ↓ → Business shrinks → Layoffs

---

## Benefits of This Architecture

✅ **Interconnected**: Every variable can affect many others  
✅ **Extensible**: Add new rules without refactoring  
✅ **Transparent**: See exactly how economy evolves  
✅ **Realistic**: Feedback loops create emergent behavior  
✅ **Educational**: Students see chain reactions clearly  
✅ **Debuggable**: Trace effects through propagation engine  

---

## Example: Adding a New Relationship

Suppose we want: **High inflation → Workers demand higher wages**

```cpp
// In EconomicPropagation::RegisterAllRules()

PropagationRule wageInflationRule = {
    trigger: "government.inflation_increase",
    target: "worker.wage_expectation",
    rule: [](double inflationRate) -> double {
        if (inflationRate > 5%) {
            return wageExpectation *= (1 + inflationRate * 0.5);
        }
        return wageExpectation;
    },
    strength: 0.7,  // 70% of inflation increase → wage increase
    delay: 1        // Takes 1 tick for workers to negotiate
};

propagator.RegisterRule(wageInflationRule);
```

That's it! No changes to Simulation, no changes to Worker class.

---

## Migration Strategy

Don't rewrite everything at once. Do it incrementally:

1. **Week 1**: Create `EconomicPropagation.h`, add 5 core rules
2. **Week 2**: Refactor `Simulation::Step()` to use propagation engine
3. **Week 3**: Add agent decision-making (Farmer harvest decisions, etc.)
4. **Week 4**: Add remaining rules, create dependency visualization
5. **Week 5**: Polish and test emergent behaviors

This way, you can keep the program working while building the new system!
