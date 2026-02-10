# cppConomy: Complete Features Guide

A comprehensive economic simulation system demonstrating real economic principles through interactive CLI commands.

---

## 📊 System Architecture

The system is built on **three core layers**:

1. **EconomicEquations** - Mathematical formulas for all economic concepts
2. **VariableTracker** - Logs all variable changes and their effects
3. **Market/Agents** - Simulates real economic behavior

Every action automatically tracks how variables change and what downstream effects occur.

---

## 🎯 Feature Categories

### **1. MARKET MECHANISMS & EQUILIBRIUM**

Demonstrate how markets find equilibrium and respond to supply/demand changes.

#### **1.1 Market Equilibrium Analysis**

Shows where supply equals demand and calculates economic welfare.

**Command:**
```
market.equilibrium("Rice")
```

**What happens:**
- Solves: Qd = Qs using linear equations
- Example: Qd = 100 - 0.5*P, Qs = 10 + 1.5*P
- Finds equilibrium: Price = $36, Quantity = 82 units
- Calculates Consumer Surplus: $1,681
- Calculates Producer Surplus: $1,066
- Shows market is efficient (Deadweight Loss = $0)

**Output:**
```
===== MARKET EQUILIBRIUM ANALYSIS FOR Rice =====
Demand Curve: Qd = 100.00 - 0.50*P
Supply Curve: Qs = 10.00 + 1.50*P
-----
EQUILIBRIUM
  Price: $36.00
  Quantity: 82.00 units

CURRENT STATE
  Price: $36.00
  Qd: 82.00,  Qs: 82.00
  Shortage/Surplus: 0.00 units
  Status: [OK] Market Clears at Equilibrium
```

**Economic Principle:**
- Shows Law of Supply and Demand
- Demonstrates that markets self-correct through price mechanism
- Shortage → Price rises → Qd falls, Qs rises → Back to equilibrium

---

#### **1.2 Price Elasticity Analysis**

Shows how responsive consumers are to price changes.

**Command:**
```
market.elasticity("Rice")         # Inelastic (necessity)
market.elasticity("IceCream")     # Elastic (luxury)
market.elasticity("Tobacco")      # Addiction case
```

**What happens:**
- Calculates PED = (% Change Quantity) / (% Change Price)
- **Elastic goods (PED > 1):**
  - 10% price increase → 15% quantity decrease
  - Lower prices increase Total Revenue
  - Examples: Ice cream, clothing, entertainment

- **Inelastic goods (PED < 1):**
  - 10% price increase → 2% quantity decrease
  - Raise prices increase Total Revenue
  - Examples: Rice, salt, gasoline

- **Addiction case (PED ≈ 0):**
  - Price changes barely affect quantity
  - Revenue always increases with price hikes
  - Examples: Tobacco, addictive drugs

**Output:**
```
===== PRICE ELASTICITY ANALYSIS FOR IceCream =====
Price Elasticity of Demand (PED): 1.85

Classification: ELASTIC (|PED| > 1)
  * Consumers very responsive to price changes
  * Luxury goods (ice cream, curd)
  * Recommendation: LOWER PRICES to increase Total Revenue

Current Total Revenue: $4,230.00
```

**Economic Principles:**
- Demonstrates Revenue = Price × Quantity
- Shows optimal pricing strategies depend on elasticity
- Explains why luxury goods need competitive pricing

---

#### **1.3 Supply Shocks**

Demonstrate how external events (weather, disasters) shift supply curves.

**Command:**
```
market.supply_shock("Bumper Harvest", "Rice")
market.supply_shock("Natural Disaster", "Rice")
```

**What happens:**

**Bumper Harvest:**
- Supply curve shifts RIGHT (Qs increases 30%)
- Example: Qs = 10 + 1.5*P → Qs = 13 + 1.95*P
- New equilibrium: Lower price, higher quantity
- Consumers benefit (lower prices), farmers lose (revenue drop)
- Old price: $36 → New price: $24 → Consumer saves money per unit
- Old quantity: 82 → New quantity: 106

**Natural Disaster:**
- Supply curve shifts LEFT (Qs decreases 50%)
- Example: Qs = 10 + 1.5*P → Qs = 5 + 0.75*P
- New equilibrium: Higher price, lower quantity
- Farmers benefit (higher prices), consumers pay more
- Shows why droughts cause food inflation

**Output:**
```
SUPPLY SHOCK DETECTED: Bumper Harvest
Supply Curve Shifted: Qs = 13.00 + 1.95*P

OLD EQUILIBRIUM: P=$36.00, Q=82.00, Revenue=$2,952
NEW EQUILIBRIUM: P=$24.00, Q=106.00, Revenue=$2,544

Effect on stakeholders:
  ✓ Consumers: Happy - prices DOWN
  ✗ Farmers: Unhappy - total revenue DOWN
  
Variable Changes Logged:
  Price: $36.00 → $24.00 (↓ -33%)
    Affects: Quantity_Supplied, Total_Revenue, Consumer_Surplus
  Quantity: 82.00 → 106.00 (↑ +29%)
    Affects: GDP, Total_Production
```

**Economic Principles:**
- Demonstrates supply curve shifts (not movement along curve)
- Shows unintended consequences (helping consumers hurts farmers)
- Explains agricultural price volatility

---

#### **1.4 Market Policies**

Show how government taxes and subsidies affect equilibrium.

**Command:**
```
market.tax("Rice", 0.15)          # 15% tax on Rice
market.subsidy("Rice", 0.20)      # 20% subsidy on Rice
market.price_control("ceiling", "Rice", 25)
market.price_control("floor", "Rice", 40)
```

**What happens:**

**Tax (shifts supply LEFT):**
- Reduces supply at every price
- Example: Qs = 10 + 1.5*P → Qs = 8.5 + 1.275*P
- Price rises, quantity falls
- Creates deadweight loss (inefficiency)
- Tax revenue = Amount × Quantity

**Subsidy (shifts supply RIGHT):**
- Increases supply at every price
- Lowers consumer price, helps producers
- Government pays the difference
- Creates deadweight loss and budget cost

**Price Ceiling (legal maximum price):**
- If binding (ceiling < equilibrium price):
  - Creates shortage (Qd > Qs)
  - Black markets develop
  - Deadweight loss = lost transactions

**Price Floor (legal minimum price):**
- If binding (floor > equilibrium price):
  - Creates surplus (Qs > Qd)
  - Unemployment in labor markets
  - Deadweight loss = unsold goods

**Output:**
```
TAX APPLIED: 15% on Rice

EQUILIBRIUM SHIFTED:
  Old: P=$36.00, Q=82.00
  New: P=$40.20, Q=72.00

DEADWEIGHT LOSS: $150.00
  * Efficiency loss from tax distortion
  
TAX REVENUE: $540.00 (Quantity × Tax per unit)

Variable Changes:
  Price: $36.00 → $40.20 (↑ +12%)
  Quantity: 82.00 → 72.00 (↓ -12%)
```

**Economic Principles:**
- Deadweight loss = economic waste from market distortion
- Incidence = who actually pays the tax
- Optimal taxation minimizes deadweight loss

---

#### **1.5 Welfare Analysis**

Calculate consumer and producer surplus (economic wellbeing).

**Command:**
```
market.welfare("Rice")
```

**What happens:**
- **Consumer Surplus:** Max willingness to pay - Actual price
- **Producer Surplus:** Actual price - Min willingness to produce
- **Deadweight Loss:** Loss from market inefficiency (taxes, monopoly, etc.)
- **Total Welfare = Consumer Surplus + Producer Surplus**

**Economic Meaning:**
- Consumer Surplus = benefit consumers get from trade
- Producer Surplus = benefit producers get from trade
- Perfect competition maximizes total welfare
- Monopoly/taxes reduce total welfare

**Output:**
```
===== CONSUMER & PRODUCER WELFARE FOR Rice =====

CONSUMER SURPLUS: $1681.00
  * Area below demand curve, above price paid
  * Benefit consumers get from trade

PRODUCER SURPLUS: $1066.00
  * Area above supply curve, below price received
  * Benefit producers get from trade

TOTAL WELFARE: $2747.00
  * Sum of consumer and producer surplus

DEADWEIGHT LOSS: $0.00
  [OK] Market is efficient (perfect competition)
```

---

### **2. CONSUMER BEHAVIOR**

Demonstrate how consumers make purchasing decisions.

#### **2.1 Optimal Bundle (Equi-Marginal Principle)**

Shows how consumers maximize satisfaction given a budget.

**Command:**
```
consumer.optimize_bundle(budget=100)
```

**What happens:**
- Presents two goods: Rice and Cloth
- Each has different Marginal Utility (MU) and Price
- Consumer optimizes when: MU_Rice/Price_Rice = MU_Cloth/Price_Cloth
- Buys combinations until ratios are equal

**Output:**
```
OPTIMAL CONSUMPTION BUNDLE (Budget: $100)

RICE:
  Marginal Utility: 50
  Price: $5
  MU/Price: 10.0

CLOTH:
  Marginal Utility: 20
  Price: $4
  MU/Price: 5.0

ANALYSIS:
  Rice has higher MU/Price ratio!
  Decision: Buy more Rice until ratios equalize
  
OPTIMAL CHOICE:
  Buy 12 Rice ($60) and 10 Cloth ($40)
  MU/Price ratios now equal → Maximum satisfaction achieved
  
Economic Principle: Equi-Marginal Principle
  * Consumer maximizes utility at the margin
  * Last dollar spent on each good gives equal satisfaction
```

**Economic Principles:**
- Marginal Utility = satisfaction from one more unit
- Diminishing Marginal Utility = each extra unit gives less satisfaction
- Optimal consumption = where ratios are equal

---

#### **2.2 Substitutes & Income Effects**

Show how consumers switch between substitute goods.

**Command:**
```
consumer.substitute("Curd", "IceCream", 0.25)
```

**What happens:**
- Curd price increases 25%
- Consumers switch to substitute: Ice cream
- Demand for Curd falls (law of demand)
- Demand for Ice cream rises (substitute effect)

**Output:**
```
SUBSTITUTE GOODS ANALYSIS

EVENT: Price of Curd increased by 25%

CROSS-PRICE ELASTICITY: +0.6 (Substitutes)
  * Positive means they're substitutes
  * When one gets expensive, demand shifts to other
  
DEMAND CHANGES:
  Curd: 50 units → 35 units (↓ -30%)
    * Law of Demand: Higher price → Lower quantity
  Ice Cream: 40 units → 58 units (↑ +45%)
    * Substitute effect: Demand switches to closer alternative
    
VARIABLE TRACKER LOG:
  Price of Curd: $4.00 → $5.00
    Affects: Demand_for_Curd, Demand_for_IceCream
  Demand_for_IceCream: 40 → 58 (↑ +45%)
    Reason: Substitute good became more expensive
```

**Economic Principles:**
- Substitutes: Curd and Ice cream compete
- Complements: Hamburger and Ketchup go together
- Cross-price elasticity > 0 for substitutes, < 0 for complements

---

### **3. PRODUCTION & FIRM COSTS**

Demonstrate how firms make production decisions.

#### **3.1 Cost Curves & Optimal Production**

Show different types of costs at each production level.

**Command:**
```
firm.cost_analysis("Rice Farm", quantity=100)
```

**What happens:**
- **Fixed Cost (TFC):** Doesn't change with quantity (land, equipment)
- **Variable Cost (TVC):** Changes with output (wages, fertilizer)
- **Total Cost (TC):** TFC + TVC
- **Average Cost (AC):** TC / Quantity (per unit cost)
- **Marginal Cost (MC):** Cost of one more unit

**Economics:**
- At low quantities: AC high (fixed costs spread over few units)
- At high quantities: MC rises (diminishing returns)
- Optimal production: Where MC = Price = AC

**Output:**
```
================================================
         Cost Analysis (Q = 100)
================================================
Total Fixed Cost (TFC)            : $500.00
Total Variable Cost (TVC)         : $800.00
Total Cost (TC)                   : $1300.00
-------------------------------------------------
AFC (Fixed Cost / Q)              : $5.00
AVC (Variable Cost / Q)           : $8.00
ATC (Total Cost / Q)              : $13.00
-------------------------------------------------
Price                             : $15.00
Revenue (P × Q)                   : $1500.00
Profit                            : $200.00 ✓ (Profitable)
Profit Margin                     : 13.33%
-------------------------------------------------
✓ PROFITABLE
Price ($15.00) > ATC ($13.00)
Firm is making economic profit.
================================================
```

**Decision Rules:**
- If Price > ATC: Make economic profit, expand
- If Price = ATC: Break even, stable
- If AVC < Price < ATC: Produce in short run, exit long run
- If Price < AVC: Shut down immediately

---

#### **3.2 Diminishing Returns**

Show how adding more workers eventually produces less output per worker.

**Command:**
```
firm.add_worker()             # Add one worker, see output change
```

**What happens:**
- Worker 1 added: +10 units output (increasing returns)
- Worker 2 added: +12 units output (still increasing)
- Worker 3 added: +8 units output (diminishing returns started!)
- Worker 4 added: +3 units output (severe diminishing returns)

**Economics:**
- Early: Workers have abundant capital, high productivity
- Later: Workers crowd into fixed space, productivity falls
- Marginal Product = output gain from one more worker

**Output:**
```
DIMINISHING RETURNS IN ACTION

Worker 1 added:
  Output: 0 → 10 units
  Marginal Product: 10 units
  Status: INCREASING RETURNS
  
Worker 2 added:
  Output: 10 → 22 units
  Marginal Product: 12 units
  Status: INCREASING RETURNS (Peak)
  
Worker 3 added:
  Output: 22 → 30 units
  Marginal Product: 8 units
  Status: DIMINISHING RETURNS STARTED
  
Worker 4 added:
  Output: 30 → 33 units
  Marginal Product: 3 units
  Status: SEVERE DIMINISHING RETURNS

CONCLUSION:
  Optimal workers to hire: 2
  At 3+ workers, would be better to hire more capital equipment
```

---

#### **3.3 Shutdown Rule**

Determine when a firm should stop operating.

**Command:**
```
firm.check_shutdown()
```

**What happens:**
- Compares current price to Average Variable Cost (AVC)
- If Price < AVC: Firm loses money on every unit
- Firm should shut down rather than operate at a loss

**Economic Logic:**
- In short run: Can shut down without paying fixed costs
- In long run: Can sell assets and fully exit

**Output:**
```
SHUTDOWN ANALYSIS

Current Market Price: $4.00
Average Variable Cost: $5.00

RESULT: SHUT DOWN IMMEDIATELY ✗

Reasoning:
  * Price ($4.00) < AVC ($5.00)
  * You lose $1.00 per unit produced
  * Better to close and pay only fixed costs
  * Than operate and lose money on every sale
  
If you operated:
  Revenue (100 units): $400.00
  Variable Costs: $500.00
  Loss per period: $100.00
  
By shutting down:
  Fixed costs still paid: $200.00
  Variable costs saved: $500.00
  Net loss: Only $200.00 (better!)
```

---

### **4. MACROECONOMICS & GOVERNMENT POLICY**

Demonstrate economy-wide effects and government intervention.

#### **4.1 GDP Calculation**

Show how GDP is computed from components.

**Command:**
```
gov.calculate_gdp("Expenditure")
```

**What happens:**
- **GDP = C + I + G + (X - M)**
- **C:** Consumer spending ($500)
- **I:** Investment spending ($200)
- **G:** Government spending ($300)
- **X - M:** Net exports (Exports - Imports) ($-50)

**Economic Interpretation:**
- Large C: Consumer-driven economy (most economic activity)
- Large I: Growth-oriented (building for future)
- Large G: Government-driven (public construction, services)
- Negative trade balance: Importing more than exporting

**Output:**
```
================================
      GDP CALCULATION
      (Expenditure Approach)
================================

Consumption (C)           : $500.00
  * Household spending on goods/services
  
Investment (I)            : $200.00
  * Business investment, home construction
  
Government Spending (G)   : $300.00
  * Government purchases, services
  
Exports (X)               : $100.00
  * Goods sold to foreign countries
  
Imports (M)               : $150.00
  * Goods bought from foreign countries
  
Net Exports (X - M)       : -$50.00
  
================================
GDP = C + I + G + (X-M)
GDP = $500 + $200 + $300 + (-$50)
GDP = $950.00
================================
```

---

#### **4.2 Inflation & CPI**

Track how prices change over time.

**Command:**
```
stats.cpi()
stats.inflation()
```

**What happens:**
- Picks a basket of goods: Rice, Bread, Milk, Eggs
- Tracks total cost over time
- Inflation = (Current price - Last period price) / Last period

**Economic Effects:**
- High inflation: Purchasing power falls, savers lose, borrowers win
- Low/Negative inflation: Prices stable, economy predictable
- Unexpected inflation: Contracts become unfair

**Output:**
```
================================
   INFLATION & CPI ANALYSIS
================================

Basket of Goods (Base Year):
  Rice (5 lbs)    : $10.00
  Bread (2 loaves): $4.00
  Milk (1 gallon) : $3.50
  Eggs (1 dozen)  : $2.50
  Total: $20.00

Current Year:
  Rice (5 lbs)    : $11.00 (+10%)
  Bread (2 loaves): $4.20 (+5%)
  Milk (1 gallon) : $3.85 (+10%)
  Eggs (1 dozen)  : $2.40 (-4%)
  Total: $21.45

CPI = (Current / Base) × 100
CPI = ($21.45 / $20.00) × 100 = 107.25

Inflation Rate = (Current CPI - Last CPI) / Last CPI × 100%
Inflation Rate = (107.25 - 100) / 100 × 100% = 7.25%

Purchasing Power Impact:
  $1,000 last year → Buys only $931.84 today
  * Savers lost 6.76% of value
  * Borrowers gained (repay with cheaper dollars)
```

---

#### **4.3 Minimum Wage**

Show unemployment effects of wage floors.

**Command:**
```
gov.set_policy("MinimumWage", 15)
```

**What happens:**
- Compares to market equilibrium wage
- If minimum > equilibrium: Creates unemployment
- Workers unable to find jobs at minimum wage

**Economic Trade-off:**
- **Good:** Workers who find jobs earn more
- **Bad:** Some lose jobs entirely (unemployment)
- Net effect depends on elasticity of labor demand

**Output:**
```
MINIMUM WAGE POLICY ANALYSIS

Market Equilibrium:
  Equilibrium Wage: $10.00
  Quantity Demanded: 100 workers
  Quantity Supplied: 100 workers
  Unemployment: 0%

New Minimum Wage Policy: $15.00

At $15.00 wage:
  Quantity Demanded: 60 workers (employers want fewer)
  Quantity Supplied: 150 workers (more want to work)
  
UNEMPLOYMENT CREATED:
  Shortage of 90 workers
  Unemployment Rate: 37.5%
  
EFFECTS:
  ✓ Workers employed: Now earn $15 (was $10)
  ✗ Workers unemployed: 90 people without jobs
  
Winners: Employed workers (+50% wage)
Losers: Unemployed workers (now jobless)
Ambiguous: Consumers (depend on business response)

Economic Principle: Labor Market Floor
  * When minimum > equilibrium: Unemployment results
  * Trade-off between higher wages and fewer jobs
```

---

#### **4.4 Monetary Policy**

Show how central bank affects economy.

**Command:**
```
centralBank.monetary_policy("Expansionary")
centralBank.monetary_policy("Contractionary")
```

**What happens:**

**Expansionary (Lower interest rates):**
- Central bank increases money supply
- Lower interest rates → Cheaper borrowing
- Businesses invest more, consumers spend more
- Aggregate Demand rises, inflation might increase

**Contractionary (Raise interest rates):**
- Central bank decreases money supply
- Higher interest rates → Expensive borrowing
- Businesses invest less, consumers spend less
- Aggregate Demand falls, inflation comes down

**Output:**
```
EXPANSIONARY MONETARY POLICY

Money Supply Increase: +20%
New Money Supply: $1,200

Interest Rate Change: 5% → 2%

CHAIN REACTION:
  1. Interest Rates Down (2%)
     ↓ Borrowing cheaper
  
  2. Investment Up
     * Business: "Now we'll expand"
     * Households: "Let's buy homes"
     ↓ I increases from $200 → $280
  
  3. Consumption Up
     * Wealth effect: Asset prices rise
     * "I feel richer, let's spend"
     ↓ C increases from $500 → $570
  
  4. Aggregate Demand Up
     * Total spending increases
     * Firms respond by hiring
     ↓ Employment increases
  
  5. OUTPUT EXPANSION (Short term)
     * GDP: $950 → $1050 (+10%)
     * Employment: 95% → 98% (unemployment falls)
     ↓ People have jobs and income
  
  6. INFLATION PRESSURE (Over time)
     * More money chasing same goods
     * Prices start rising
     * Expected Inflation: 3% → 5%

SUMMARY:
  Benefit: More jobs, higher output (Short term)
  Cost: Rising inflation (Long term)
```

---

#### **4.5 Fiscal Policy**

Show how taxes and government spending affect economy.

**Command:**
```
gov.set_policy("GovernmentSpending", 500)
gov.set_policy("IncomeTaxRate", 0.25)
```

**What happens:**

**Increase Government Spending:**
- Direct: Government purchases goods/services
- Indirect: Multiplier effect - workers earn income, spend it, creates more jobs
- Initial G ↑ → GDP ↑ × 2-3x multiplier

**Increase Income Tax:**
- Workers have less disposable income
- Consumption falls → Aggregate Demand falls
- GDP falls → Unemployment rises

**Output:**
```
FISCAL POLICY: EXPANSIONARY SPENDING

Government Spending Increase: $300 → $500 (+67%)

MULTIPLIER EFFECT:
  Spending Rounds:
  Round 1: Gov spends $200
    → Workers earn $200
    → Workers save 20%, spend 80%
    → Next Round Spending: $160
    
  Round 2: Spending $160
    → Workers earn $160
    → Workers spend $128
    
  Round 3: Spending $128
    → Workers earn $128
    → Workers spend $102
    
  (Continues with decreasing amounts...)
  
  Total Multiplied Effect: $200 initial → $500 total impact
  Multiplier = 2.5x

AGGREGATE DEMAND EFFECT:
  Initial G increase: $200
  Total AD increase: $500
  * Each dollar of government spending creates
    $2.50 of economic activity

GDP EXPANSION:
  GDP: $950 → $1,100
  Employment: 95% → 98%
  
COST:
  Government Deficit increases (spending > tax revenue)
  May require future tax increases
```

---

### **5. PRODUCTION POSSIBILITY FRONTIER**

Demonstrate economy-wide production trade-offs.

**Command:**
```
economy.ppf("Rice", "Cloth")
```

**What happens:**
- Shows trade-off: More rice = less cloth production
- PPF = Maximum production with full resource utilization
- Inside PPF = Unemployment/inefficiency
- Outside PPF = Impossible (insufficient resources)

**Economic Insights:**
- Productive Efficiency = Operating ON the frontier
- Allocative Efficiency = Best mix of goods people want
- Labor specialization shifts PPF outward (technology upgrade)

**Output:**
```
===============================
  PRODUCTION POSSIBILITY FRONTIER
===============================

Resources Available:
  * 100 workers
  * 1,000 acres of land
  * Technology Level: 1.0

PRODUCTION POSSIBILITIES:

100% Rice, 0% Cloth:
  Rice: 500 units
  Cloth: 0 units
  
75% Rice, 25% Cloth:
  Rice: 375 units
  Cloth: 100 units
  
50% Rice, 50% Cloth:
  Rice: 250 units
  Cloth: 200 units
  
25% Rice, 75% Cloth:
  Rice: 125 units
  Cloth: 300 units
  
0% Rice, 100% Cloth:
  Rice: 0 units
  Cloth: 400 units

OPPORTUNITY COST:
  To produce 1 more unit of Cloth
  Must give up: 1.25 units of Rice
  
CURRENT PRODUCTION:
  Rice: 300 units
  Cloth: 150 units
  Status: INSIDE frontier (Inefficient!)
  * Could produce more with same resources
  * 50 workers are unemployed
  
RECOMMENDATION:
  Move to frontier to be productively efficient
  New production: Rice 350, Cloth 190
  This uses all 100 workers
```

---

#### **5.1 Technological Growth**

Show how innovation shifts PPF outward.

**Command:**
```
tech.upgrade("Agriculture")
```

**What happens:**
- Improved farming technology (better seeds, equipment)
- Productivity increases (same workers produce more)
- PPF shifts outward (can produce more of everything)

**Output:**
```
TECHNOLOGICAL UPGRADE: AGRICULTURE

Technology Level: 1.0 → 1.5 (+50%)

PPF SHIFT:

Old Maximum Production:
  Max Rice: 500 units
  Max Cloth: 400 units

New Maximum Production:
  Max Rice: 750 units (+50%)
  Max Cloth: 400 units (unchanged - tech specific)

EFFECT:
  * Farmers now produce 50% more rice with same effort
  * Economy richer - can have more rice AND same cloth
  * Living standards up across board
  
OUTPUT GROWTH:
  Old: Rice 300, Cloth 150 (Total Value: $4,050)
  New: Rice 450, Cloth 150 (Total Value: $5,850)
  Growth: +44%
  
EMPLOYMENT MULTIPLIER:
  Some farmers laid off (fewer needed)
  BUT: Higher income allows more consumption
  New jobs in service industries
  Net effect: Usually positive employment
```

---

### **6. VARIABLE TRACKING & ANALYSIS**

Track how all variables interact throughout the economy.

**Command:**
```
system.show_changes()
system.dependency_chain("Price of Rice")
```

**What happens:**
- Shows complete audit trail of all variable changes
- Displays cause-and-effect chains
- Shows which changes cascade to other variables

**Output:**
```
==================================================
               RECENT VARIABLE CHANGES
==================================================

🟢 Price of Rice [MARKET_PRICE]
   $36.00 ↑ $40.20 (↑ +12%)
   Reason: Tax applied to rice
   Caused by: Tax Policy Applied
   Affects: Quantity_Demanded, Quantity_Supplied, 
            Consumer_Surplus, Total_Revenue
   Time: 2026-02-09 14:32:15

🔴 Quantity Demanded [MARKET_DEMAND]
   82.00 ↓ 72.00 (↓ -12%)
   Reason: Consumers respond to price increase
   Caused by: Price of Rice increased
   Affects: Total_Sales, Market_Revenue, GDP
   Time: 2026-02-09 14:32:16

🟢 Government Tax Revenue [MACRO_POLICY]
   $0.00 ↑ $540.00 (↑ +infinity%)
   Reason: Tax rate applied to quantity sold
   Caused by: Tax Policy Applied
   Affects: Government_Budget, Deficit, Debt
   Time: 2026-02-09 14:32:17

🔴 Consumer Surplus [MARKET_DEMAND]
   $1681.00 ↓ $1521.00 (↓ -9%)
   Reason: Price increase reduces consumer benefit
   Caused by: Price of Rice increased
   Affects: Total_Welfare, Living_Standards
   Time: 2026-02-09 14:32:17

==================================================
```

**Dependency Chain:**
```
Price of Rice Changed
  ↓
Quantity Demanded falls (Law of Demand)
  ↓
Total Revenue might change (depends on elasticity)
  ↓ (if demand is inelastic)
Revenue up - helps farmers
  ↓
Farm profits increase
  ↓
Farmers hire more workers
  ↓
Employment rises, unemployment falls
  ↓
New workers earn income
  ↓
They spend on other goods
  ↓
Those industries expand
  ↓
Overall economic activity increases (GDP grows)
```

---

## 🎮 Quick Start Guide

### **1. Start the simulation:**
```
./cppConomy
```

### **2. Create markets and agents:**
```
add_market("Rice")
add_market("IceCream")
add_worker(name="Alice", income=1000, skill=0.7)
add_farmer(name="Bob", land=10, crop="Rice")
```

### **3. Run economic scenarios:**

**Scenario 1: Market Equilibrium**
```
market.equilibrium("Rice")
market.elasticity("Rice")
market.welfare("Rice")
```

**Scenario 2: Supply Shock**
```
market.supply_shock("Bumper Harvest", "Rice")
market.supply_shock("Natural Disaster", "Rice")
```

**Scenario 3: Government Intervention**
```
market.tax("Rice", 0.15)
market.subsidy("IceCream", 0.10)
gov.set_policy("MinimumWage", 15)
```

**Scenario 4: Consumer Behavior**
```
consumer.optimize_bundle(budget=100)
consumer.substitute("Curd", "IceCream", 0.25)
```

**Scenario 5: Production & Costs**
```
firm.cost_analysis("RiceFarm", quantity=100)
firm.add_worker()
firm.check_shutdown()
```

**Scenario 6: Macroeconomics**
```
gov.calculate_gdp("Expenditure")
stats.cpi()
stats.inflation()
centralBank.monetary_policy("Expansionary")
```

### **4. Monitor changes:**
```
system.show_changes()
system.dependency_chain("Price of Rice")
```

---

## 📚 Economic Principles Demonstrated

| Principle | Command | Shows |
|-----------|---------|-------|
| Law of Demand | `market.equilibrium()` | Higher price → Lower quantity |
| Law of Supply | `market.equilibrium()` | Higher price → Higher quantity |
| Market Equilibrium | `market.equilibrium()` | Qd = Qs at market-clearing price |
| Price Elasticity | `market.elasticity()` | % Change Qty / % Change Price |
| Price Controls | `market.price_control()` | Deadweight loss from binding floors/ceilings |
| Supply Shocks | `market.supply_shock()` | External events shift curves |
| Substitution | `consumer.substitute()` | Switching between similar goods |
| Marginal Utility | `consumer.optimize_bundle()` | Consumer maximizes MU/Price |
| Diminishing Returns | `firm.add_worker()` | Each worker adds less output |
| Cost Curves | `firm.cost_analysis()` | AC, MC, AVC relationships |
| Shutdown Rule | `firm.check_shutdown()` | Exit if P < AVC |
| GDP Components | `gov.calculate_gdp()` | C + I + G + (X-M) |
| Inflation | `stats.inflation()` | Price level changes over time |
| Monetary Policy | `centralBank.monetary_policy()` | Money supply affects spending |
| Fiscal Policy | `gov.set_policy()` | Taxes and spending affect GDP |
| PPF | `economy.ppf()` | Production trade-offs |
| Technology | `tech.upgrade()` | Innovation shifts PPF outward |
| Labor Market Floor | `gov.set_policy("MinW")` | Unemployment from minimum wage |
| Deadweight Loss | `market.tax()` | Efficiency loss from intervention |
| Variable Cascades | `system.dependency_chain()` | How changes propagate through economy |

---

## 🔧 Technical Details

### Data Flow

```
User Command
    ↓
CommandParser (Parse input)
    ↓
CommandExecutor (Route to handler)
    ↓
Simulation/Market/Agent (Perform action)
    ↓
EconomicEquations (Calculate effects)
    ↓
VariableTracker (Log changes & cascades)
    ↓
Output to CLI (Formatted results)
```

### File Structure

```
src/
  ├─ EconomicEquations.h/cpp    (Math formulas)
  ├─ VariableTracker.h/cpp      (Change logging)
  ├─ Market.h/cpp               (Markets & equilibrium)
  ├─ Farmer.h/cpp               (Production & costs)
  ├─ Worker.h/cpp               (Labor & consumption)
  ├─ Government.h/cpp           (Policies)
  ├─ Simulation.h/cpp           (Coordination)
  ├─ CommandParser.h/cpp        (Parse commands)
  ├─ CommandExecutor.h/cpp      (Execute commands)
  └─ CLI.h/cpp                  (User interface)
```

---

## ⚙️ Compilation

```bash
g++ -std=c++17 -o cppConomy src/*.cpp
./cppConomy
```

---

## 📋 Summary

This system demonstrates **18+ economic principles** through interactive simulations:
- Markets (equilibrium, elasticity, welfare)
- Consumer behavior (utility, substitution)
- Production (costs, diminishing returns)  
- Macroeconomics (GDP, inflation, policies)
- Government intervention (taxes, minimum wage, monetary policy)
- Growth (technology, PPF shifts)

All interconnected through comprehensive variable tracking showing how the economy responds to changes.

