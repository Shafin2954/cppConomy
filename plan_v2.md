# CppConomy v2 - Simplified Economic Simulator Plan

## Overview

A simplified economic simulator focusing on demonstrating specific economic concepts through interactive visualization of Person, Farmer, Product, and Market entities.

---

## UI Layout

```
+------------------------------------------------------------------+
|                         MENU BAR                                  |
+------------------------------------------------------------------+
|                                                                   |
|                     TOP PANEL (Dashboard)                         |
|        Global Stats: GDP, Population, Total Production, etc.      |
|                                                                   |
+------------------------------------------------------------------+
|                                                                   |
|                    MIDDLE PANEL (4 Columns)                       |
|  +------------+------------+------------+------------+            |
|  |   PERSON   |   FARMER   |  PRODUCT   |   MARKET   |            |
|  +------------+------------+------------+------------+            |
|  | Name: ...  | Name: ...  | Name: ...  | Name: ...  |            |
|  | Income: $  | Land: acres| Price: $   | Type: cloth|            |
|  | Spending:$ | Crop: rice | Quantity:  | Demand:    |            |
|  | Savings: $ | Yield:     | Cost: $    | Supply:    |            |
|  | Utility:   | Labor:     | Margin:    | Eqb Price: |            |
|  | ...        | ...        | ...        | ...        |            |
|  +------------+------------+------------+------------+            |
|                                                                   |
+------------------------------------------------------------------+
|  COMMAND: select(person="John", market="cloth")           [Enter] |
+------------------------------------------------------------------+
|  COMMANDS  |                                                      |
|  --------- |                 EVENT LOG                            |
|  add_person|   [INFO] Person John selected                        |
|  add_farmer|   [OK] Market cloth: Price = $50                     |
|  buy       |   [WARN] Low supply in rice market                   |
|  harvest   |   ...                                                |
|  hire      |                                                      |
|  produce   |                                                      |
|  select    |                                                      |
|  sell      |                                                      |
|  set_price |                                                      |
|  set_tax   |                                                      |
|  set_wage  |                                                      |
+------------------------------------------------------------------+
```

### Panel Proportions
- **Right sidebar (Commands)**: 15% width (was 20%+, now smaller)
- **Top panel**: 15% height
- **Middle panel**: 50% height  
- **Bottom (Command + Log)**: 35% height

---

## Core Classes

### 1. Person
Represents a consumer/worker in the economy.

```cpp
class Person {
    std::string name;
    double income;          // Monthly income
    double savings;         // Accumulated savings
    double spending;        // Monthly spending
    double utility;         // Satisfaction level
    std::string occupation; // "worker", "unemployed", "farmer"
    
    // For demonstrating:
    // - Marginal utility, income effect
    // - Consumer surplus
    // - Budget constraints
    // - Demand behavior
};
```

**Variables shown in panel:**
- Name, Income, Savings, Spending
- Utility, Occupation
- Budget remaining
- Goods owned (list)

### 2. Farmer
Represents an agricultural producer.

```cpp
class Farmer {
    std::string name;
    double land;            // Acres of land
    std::string crop;       // "rice", "wheat", etc.
    double yield;           // Units per acre
    int laborHired;         // Number of workers
    double productionCost;  // Total cost
    double revenue;         // From sales
    double profit;          // Revenue - Cost
    
    // For demonstrating:
    // - Production function
    // - Diminishing returns
    // - PPF concepts
    // - Bumper harvest effects
};
```

**Variables shown in panel:**
- Name, Land (acres), Crop type
- Yield, Labor hired
- Production cost, Revenue, Profit
- Marginal product of labor

### 3. Product
Represents a good/commodity.

```cpp
class Product {
    std::string name;       // "rice", "cloth", "ice cream"
    double price;           // Current market price
    double quantity;        // Available quantity
    double cost;            // Production cost per unit
    double margin;          // Price - Cost
    std::string category;   // "necessity", "luxury", "addictive"
    double elasticity;      // Price elasticity of demand
    
    // For demonstrating:
    // - Elasticity effects
    // - Substitution (linked products)
    // - Price changes and revenue
};
```

**Variables shown in panel:**
- Name, Category, Price
- Quantity, Cost, Margin
- Elasticity value
- Substitute product (if any)

### 4. Market
Represents a market for a specific good.

```cpp
class Market {
    std::string productName;  // What's being traded
    double demand;            // Quantity demanded
    double supply;            // Quantity supplied
    double equilibriumPrice;  // Where D = S
    double currentPrice;      // Actual price
    double taxRate;           // Tax on this market
    double minWage;           // If labor market
    
    // For demonstrating:
    // - Supply and demand
    // - Equilibrium
    // - Tax effects
    // - Minimum wage effects
};
```

**Variables shown in panel:**
- Product name, Current price
- Demand, Supply
- Equilibrium price
- Tax rate, Surplus/Shortage

---

## Economic Concepts Mapping

| Concept | Classes Involved | How Demonstrated |
|---------|------------------|------------------|
| **PPF** | Farmer | Show tradeoff between two crops |
| **Demand/Supply/Equilibrium** | Market | Visual of D, S, price adjustment |
| **Substitution** | Product, Market | Link two products, show cross-effects |
| **Elasticity** | Product, Market | Change price, show quantity/revenue change |
| **Marginal Utility** | Person | Buying more of same good decreases utility gain |
| **Income Effect** | Person | Income change affects buying behavior |
| **Consumer Surplus** | Person, Market | Difference between willingness to pay and price |
| **Production Function** | Farmer | Inputs → Outputs relationship |
| **Diminishing Returns** | Farmer | Adding labor shows decreasing marginal product |
| **Tax Effects** | Market | Add tax, see price and quantity change |
| **Minimum Wage** | Market (labor) | Set min wage, see unemployment effect |
| **Bumper Harvest** | Farmer, Market | Excess production → price drop |
| **Inflation (CPI)** | Global | Track price changes across products |
| **GDP** | Global | Sum of all production values |

---

## Commands

### Selection Commands
```
select(person="John")           # Select a person to view
select(farmer="Raju")           # Select a farmer to view
select(product="rice")          # Select a product to view
select(market="cloth")          # Select a market to view
clear_selection()               # Clear all selections
```

### Person Commands
```
add_person(name="John", income=5000)
set_income(person="John", amount=6000)
buy(person="John", product="rice", quantity=10)
work(person="John", employer="Raju")
```

### Farmer Commands
```
add_farmer(name="Raju", land=10, crop="rice")
hire(farmer="Raju", workers=5)
fire(farmer="Raju", workers=2)
harvest()                       # All farmers harvest
plant(farmer="Raju", crop="wheat")
```

### Product Commands
```
add_product(name="cloth", price=50, cost=30)
set_price(product="cloth", price=60)
set_elasticity(product="cloth", value=1.5)
link_substitute(product="curd", substitute="ice cream")
```

### Market Commands
```
add_market(product="rice")
set_tax(market="rice", rate=0.10)
set_min_wage(rate=15)           # For labor market
update_market(market="rice")    # Recalculate equilibrium
```

### Simulation Commands
```
tick()                          # Advance one time period
tick(count=10)                  # Advance multiple periods
reset()                         # Reset simulation
status()                        # Show current state
```

### Policy Commands (for later phases)
```
fiscal_policy(type="tax_cut", amount=0.05)
monetary_policy(type="interest", rate=0.03)
disaster(type="flood", severity=0.5)
```

---

## Implementation Phases

### Phase 1: Core Structure
- [ ] Simplify existing code, remove complex economic models
- [ ] Create new classes: Person, Farmer, Product, Market
- [ ] Basic CRUD operations for each class
- [ ] Storage/management in a central Simulation class

### Phase 2: UI Overhaul
- [ ] Redesign MainFrame with new layout
- [ ] Create 4-column middle panel (SectorPanel)
- [ ] Update right sidebar to show commands list (clickable)
- [ ] Fix panel proportions
- [ ] Update command panel for flexible input

### Phase 3: Selection System
- [ ] Implement select() command
- [ ] Connect selection to panel display
- [ ] Highlight selected entities
- [ ] Show detailed info in respective column

### Phase 4: Basic Economics
- [ ] Implement demand/supply curves
- [ ] Market equilibrium calculation
- [ ] Price changes and effects
- [ ] Basic person buying behavior

### Phase 5: Production
- [ ] Farmer production function
- [ ] Labor and diminishing returns
- [ ] Harvest and market supply
- [ ] Cost calculations

### Phase 6: Advanced Concepts
- [ ] Elasticity effects
- [ ] Substitution between goods
- [ ] Utility calculations
- [ ] Consumer surplus visualization

### Phase 7: Policy Effects
- [ ] Tax implementation
- [ ] Minimum wage effects
- [ ] Inflation tracking (CPI)
- [ ] GDP calculation

---

## File Structure (New)

```
src/
├── main.cpp
├── core/
│   ├── Person.h/cpp
│   ├── Farmer.h/cpp
│   ├── Product.h/cpp
│   ├── Market.h/cpp
│   └── Simulation.h/cpp      # Central controller (replaces Economy)
├── commands/
│   ├── CommandParser.h/cpp   # Keep, modify for new commands
│   └── CommandExecutor.h/cpp # Rewrite for new commands
├── gui/
│   ├── MainFrame.h/cpp       # Redesign layout
│   ├── DashboardPanel.h/cpp  # Keep, simplify
│   ├── SectorPanel.h/cpp     # NEW: 4-column display
│   ├── CommandListPanel.h/cpp # NEW: Clickable command list
│   ├── EventLogPanel.h/cpp   # Keep
│   └── CommandPanel.h/cpp    # Keep
└── utils/
    ├── Config.h              # Simplify
    └── Logger.h              # Keep
```

---

## Next Steps

1. **You explain** each concept from list.md one by one
2. **I implement** the relevant class methods and commands
3. **We test** each feature before moving to next

### Questions for you:
1. Should we start with Phase 1 (Core Structure) now?
2. Which concept from list.md do you want to implement first?
3. What specific variables/behaviors should each entity have initially?

---

*CppConomy v2 - Educational Economic Simulator*
