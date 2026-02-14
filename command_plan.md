# Command System Redesign Plan

Based on the available entities and their implemented functionality in the `.h` files, this document outlines commands that can actually be implemented without external dependencies or "propagators."

---

## Available Functionality Summary

### consumer.h
| Method | Description |
|--------|-------------|
| `getMUperDollar()` | Returns `1/wealth` where wealth = savings + 30*incomePerDay |
| `getMarginalUtility(product)` | Returns `WTP * muPerDollar` where WTP = `c - m*consumed` |
| `consumerSurplus(product, price)` | Returns `0.5 * c - price * consumed` |
| `updateSubRatio(product)` | Returns `MU(product) / MU(rice)` |
| `pass_day()` | Increments age, decays consumed amounts |

### laborer.h (extends consumer)
| Property | Description |
|----------|-------------|
| `skillLevel` | 0-1, affects productivity |
| `minWage` | Daily minimum wage |

### farmer.h (extends consumer)
| Method | Description |
|--------|-------------|
| `addCrop(prod, supply, growth, decay, maxOut)` | Registers a new crop |
| `upgradeTech(level)` | Sets techLevel |
| `calculateSupply(crop, price)` | Returns quantity using `(P - MC) / slope`, capped at maxOutput |
| `pass_day()` | Updates weather, adjusts maxOutput by growth/decay |

### firm.h
| Method | Description |
|--------|-------------|
| `output(L, K)` | Cobb-Douglas: `L^α * K^β * A` or CES: `(L^ρ + K^ρ)^(1/ρ)` |
| `MPofLabor()` | Marginal product: `output(L+1, K) - output(L, K)` |
| `MPofCapital()` | Marginal product: `output(L, K+1) - output(L, K)` |
| `marginalCosts()` | Returns `{MPL/wage, MPK/rentalRate}` |
| `calculateCosts()` | Computes TFC, TVC, TC, AFC, AVC, AC, MC |
| `getCapitalCost()` | Sum of all capital rental rates |

### market.h
| Method | Description |
|--------|-------------|
| `calculateAggregateDemand(consumers)` | Horizontal sum of individual demand curves |
| `calculateAggregateSupply(farmers)` | Horizontal sum of individual supply curves |
| `findEquilibrium()` | Solves `Qd = Qs` → returns `{price, quantity}` |

### world.h
| Method | Description |
|--------|-------------|
| `addConsumer(name, age)` | Creates new consumer |
| `addFarmer(name, age, land, tech)` | Creates new farmer |
| `addlaborer(name, age, skill, minWage)` | Creates new laborer |
| `addFirm(id, cash, cobbDouglas)` | Creates new firm |

---

## Command Categories

### 1. Entity Listing Commands

```cpp
// cmd.h - CommandInfo entries
{"consumers", "List all consumers", {}},
{"laborers", "List all laborers", {}},
{"farmers", "List all farmers", {}},
{"firms", "List all firms", {}},
{"markets", "List all markets", {}},
{"products", "List all products", {}},
```

**Implementation:**
```cpp
void cmdConsumers(const Command &cmd) {
    std::stringstream ss;
    ss << "CONSUMERS (" << simulation.consumers.size() << ")\n";
    ss << std::string(40, '-') << "\n";
    for (auto& c : simulation.consumers) {
        ss << "ID: " << c.id << " | " << c.name 
           << " | Age: " << (c.ageInDays / 365) << "y"
           << " | Savings: $" << c.savings 
           << " | Alive: " << (c.isAlive ? "Yes" : "No") << "\n";
    }
    output(ss.str());
}

void cmdLaborers(const Command &cmd) {
    std::stringstream ss;
    ss << "LABORERS (" << simulation.laborers.size() << ")\n";
    ss << std::string(40, '-') << "\n";
    for (auto& l : simulation.laborers) {
        ss << "ID: " << l.id << " | " << l.name 
           << " | Skill: " << (int)(l.skillLevel * 100) << "%"
           << " | Min Wage: $" << l.minWage << "/day\n";
    }
    output(ss.str());
}

void cmdFarmers(const Command &cmd) {
    std::stringstream ss;
    ss << "FARMERS (" << simulation.farmers.size() << ")\n";
    ss << std::string(40, '-') << "\n";
    for (auto& f : simulation.farmers) {
        ss << "ID: " << f.id << " | " << f.name 
           << " | Land: " << f.land << " acres"
           << " | Tech: " << f.techLevel
           << " | Crops: " << f.crops.size() << "\n";
    }
    output(ss.str());
}

void cmdFirms(const Command &cmd) {
    std::stringstream ss;
    ss << "FIRMS (" << simulation.firms.size() << ")\n";
    ss << std::string(40, '-') << "\n";
    for (auto& f : simulation.firms) {
        ss << "Owner ID: " << f.ownerId 
           << " | Cash: $" << f.cash
           << " | Workers: " << f.workers.size()
           << " | Capital: " << f.capitals.size()
           << " | Output: " << f.currentOutput << "\n";
    }
    output(ss.str());
}

void cmdMarkets(const Command &cmd) {
    std::stringstream ss;
    ss << "MARKETS (" << simulation.markets.size() << ")\n";
    ss << std::string(40, '-') << "\n";
    for (auto& m : simulation.markets) {
        ss << m.prod->name 
           << " | Price: $" << m.price
           << " | Demand: p = " << m.aggregateDemand.c << " - " << m.aggregateDemand.m << "Q"
           << " | Supply: p = " << m.aggregateSupply.c << " + " << m.aggregateSupply.m << "Q\n";
    }
    output(ss.str());
}

void cmdProducts(const Command &cmd) {
    std::stringstream ss;
    ss << "PRODUCTS\n" << std::string(40, '-') << "\n";
    std::vector<product*> prods = {&rice, &cloth, &computer, &phone, &car, &steel};
    for (auto* p : prods) {
        ss << p->name << " | Decay Rate: " << p->decayRate << "/day\n";
    }
    output(ss.str());
}
```

---

### 2. Entity Creation Commands

```cpp
{"add consumer", "Add a consumer", {{"name", "Name"}, {"age", "Age in years"}}},
{"add laborer", "Add a laborer", {{"name", "Name"}, {"age", "Age"}, {"skill", "Skill 0-1"}, {"minwage", "Min wage"}}},
{"add farmer", "Add a farmer", {{"name", "Name"}, {"age", "Age"}, {"land", "Land acres"}, {"tech", "Tech level 0-1"}}},
{"add firm", "Add a firm", {{"ownerid", "Owner consumer ID"}, {"cash", "Initial cash"}, {"alpha", "Cobb-Douglas α"}, {"beta", "Cobb-Douglas β"}}},
```

**Implementation:**
```cpp
void cmdAddConsumer(const Command &cmd) {
    std::string name = std::get<std::string>(cmd.params.at("name"));
    int age = std::get<int>(cmd.params.at("age"));
    simulation.addConsumer(name, age);
    output("Added consumer: " + name);
}

void cmdAddLaborer(const Command &cmd) {
    std::string name = std::get<std::string>(cmd.params.at("name"));
    int age = std::get<int>(cmd.params.at("age"));
    double skill = std::get<double>(cmd.params.at("skill"));
    double minwage = std::get<double>(cmd.params.at("minwage"));
    simulation.addlaborer(name, age, skill, minwage);
    output("Added laborer: " + name);
}

void cmdAddFarmer(const Command &cmd) {
    std::string name = std::get<std::string>(cmd.params.at("name"));
    int age = std::get<int>(cmd.params.at("age"));
    double land = std::get<double>(cmd.params.at("land"));
    double tech = std::get<double>(cmd.params.at("tech"));
    simulation.addFarmer(name, age, land, tech);
    output("Added farmer: " + name);
}

void cmdAddFirm(const Command &cmd) {
    int ownerId = std::get<int>(cmd.params.at("ownerid"));
    double cash = std::get<double>(cmd.params.at("cash"));
    double alpha = std::get<double>(cmd.params.at("alpha"));
    double beta = std::get<double>(cmd.params.at("beta"));
    simulation.addFirm(ownerId, cash, cobbDouglas(alpha, beta, 1.0));
    output("Added firm for owner ID: " + std::to_string(ownerId));
}
```

---

### 3. Selection Commands

```cpp
{"select consumer", "Select a consumer", {{"name", "Consumer name"}}},
{"select laborer", "Select a laborer", {{"name", "Laborer name"}}},
{"select farmer", "Select a farmer", {{"name", "Farmer name"}}},
{"select market", "Select a market", {{"product", "Product name"}}},
{"clear selection", "Clear all selections", {}},
```

**Implementation:**
```cpp
void cmdSelectConsumer(const Command &cmd) {
    std::string name = std::get<std::string>(cmd.params.at("name"));
    for (auto& c : simulation.consumers) {
        if (c.name == name) {
            simulation.selected_consumer = &c;
            output("Selected consumer: " + name);
            return;
        }
    }
    output("Error: Consumer not found: " + name);
}

void cmdSelectLaborer(const Command &cmd) {
    std::string name = std::get<std::string>(cmd.params.at("name"));
    for (auto& l : simulation.laborers) {
        if (l.name == name) {
            simulation.selected_laborer = &l;
            output("Selected laborer: " + name);
            return;
        }
    }
    output("Error: Laborer not found: " + name);
}

void cmdSelectFarmer(const Command &cmd) {
    std::string name = std::get<std::string>(cmd.params.at("name"));
    for (auto& f : simulation.farmers) {
        if (f.name == name) {
            simulation.selected_farmer = &f;
            output("Selected farmer: " + name);
            return;
        }
    }
    output("Error: Farmer not found: " + name);
}

void cmdSelectMarket(const Command &cmd) {
    std::string prodName = std::get<std::string>(cmd.params.at("product"));
    for (auto& m : simulation.markets) {
        if (m.prod->name == prodName) {
            simulation.selected_market = &m;
            output("Selected market: " + prodName);
            return;
        }
    }
    output("Error: Market not found: " + prodName);
}

void cmdClearSelection(const Command &cmd) {
    simulation.selected_consumer = nullptr;
    simulation.selected_laborer = nullptr;
    simulation.selected_farmer = nullptr;
    simulation.selected_market = nullptr;
    output("Selections cleared");
}
```

---

### 4. Market Analysis Commands

```cpp
{"market equilibrium", "Find equilibrium price and quantity", {{"product", "Product name (optional, uses selected)"}}},
{"market demand", "Show aggregate demand curve", {{"product", "Product name (optional)"}}},
{"market supply", "Show aggregate supply curve", {{"product", "Product name (optional)"}}},
```

**Implementation:**
```cpp
void cmdMarketEquilibrium(const Command &cmd) {
    market* m = getMarketFromCmd(cmd); // helper to get from param or selected
    if (!m) { output("Error: No market selected"); return; }

    // Recalculate curves with current data
    m->calculateAggregateDemand(simulation.consumers);
    m->calculateAggregateSupply(simulation.farmers);

    auto eq = m->findEquilibrium();

    std::stringstream ss;
    ss << "MARKET EQUILIBRIUM: " << m->prod->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Demand Curve: P = " << m->aggregateDemand.c << " - " << m->aggregateDemand.m << "Q\n";
    ss << "Supply Curve: P = " << m->aggregateSupply.c << " + " << m->aggregateSupply.m << "Q\n";
    ss << std::string(40, '-') << "\n";
    ss << "Equilibrium Price:    $" << std::fixed << std::setprecision(2) << eq.price << "\n";
    ss << "Equilibrium Quantity: " << std::fixed << std::setprecision(2) << eq.quantity << " units\n";

    m->price = eq.price; // Update market price
    output(ss.str());
}

void cmdMarketDemand(const Command &cmd) {
    market* m = getMarketFromCmd(cmd);
    if (!m) { output("Error: No market selected"); return; }

    m->calculateAggregateDemand(simulation.consumers);

    std::stringstream ss;
    ss << "AGGREGATE DEMAND: " << m->prod->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Curve: P = " << m->aggregateDemand.c << " - " << m->aggregateDemand.m << "Q\n";
    ss << "\nIndividual Demands:\n";
    for (auto& c : simulation.consumers) {
        if (c.dd.find(m->prod) != c.dd.end()) {
            ss << "  " << c.name << ": P = " << c.dd[m->prod].c 
               << " - " << c.dd[m->prod].m << "Q\n";
        }
    }
    output(ss.str());
}

void cmdMarketSupply(const Command &cmd) {
    market* m = getMarketFromCmd(cmd);
    if (!m) { output("Error: No market selected"); return; }

    m->calculateAggregateSupply(simulation.farmers);

    std::stringstream ss;
    ss << "AGGREGATE SUPPLY: " << m->prod->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Curve: P = " << m->aggregateSupply.c << " + " << m->aggregateSupply.m << "Q\n";
    ss << "\nIndividual Supplies:\n";
    for (auto& f : simulation.farmers) {
        if (f.ss.find(m->prod) != f.ss.end()) {
            ss << "  " << f.name << ": P = " << f.ss[m->prod].c 
               << " + " << f.ss[m->prod].m << "Q"
               << " | Max: " << f.maxOutput[m->prod] << "\n";
        }
    }
    output(ss.str());
}
```

---

### 5. Consumer Analysis Commands

```cpp
{"consumer mu", "Show marginal utility for a product", {{"product", "Product name"}}},
{"consumer surplus", "Calculate consumer surplus", {{"product", "Product name"}}},
{"consumer substitution", "Show substitution ratios", {}},
{"consumer needs", "Show consumer needs and consumption", {}},
```

**Implementation:**
```cpp
void cmdConsumerMU(const Command &cmd) {
    consumer* c = simulation.selected_consumer;
    if (!c) { output("Error: No consumer selected"); return; }

    std::string prodName = std::get<std::string>(cmd.params.at("product"));
    product* p = getProductByName(prodName);
    if (!p) { output("Error: Unknown product"); return; }

    double mu = c->getMarginalUtility(*p);
    double muPerDollar = c->getMUperDollar();

    std::stringstream ss;
    ss << "MARGINAL UTILITY: " << c->name << " → " << prodName << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "MU per Dollar: " << std::fixed << std::setprecision(4) << muPerDollar << "\n";
    ss << "WTP (Willingness to Pay): $" << (c->dd[p].c - c->dd[p].m * c->consumed[p]) << "\n";
    ss << "Consumed so far: " << c->consumed[p] << "\n";
    ss << "Marginal Utility: " << std::fixed << std::setprecision(4) << mu << "\n";
    output(ss.str());
}

void cmdConsumerSurplus(const Command &cmd) {
    consumer* c = simulation.selected_consumer;
    if (!c) { output("Error: No consumer selected"); return; }

    std::string prodName = std::get<std::string>(cmd.params.at("product"));
    product* p = getProductByName(prodName);
    market* m = getMarketByProduct(p);
    if (!p || !m) { output("Error: Unknown product/market"); return; }

    double surplus = c->consumerSurplus(*p, m->price);

    std::stringstream ss;
    ss << "CONSUMER SURPLUS: " << c->name << " → " << prodName << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Market Price: $" << m->price << "\n";
    ss << "Quantity Consumed: " << c->consumed[p] << "\n";
    ss << "Consumer Surplus: $" << std::fixed << std::setprecision(2) << surplus << "\n";
    output(ss.str());
}

void cmdConsumerSubstitution(const Command &cmd) {
    consumer* c = simulation.selected_consumer;
    if (!c) { output("Error: No consumer selected"); return; }

    std::stringstream ss;
    ss << "SUBSTITUTION RATIOS: " << c->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "(Relative to Rice)\n\n";

    for (auto& need : c->needs) {
        double ratio = c->updateSubRatio(need);
        ss << need.name << ": " << std::fixed << std::setprecision(3) << ratio << "\n";
    }
    output(ss.str());
}

void cmdConsumerNeeds(const Command &cmd) {
    consumer* c = simulation.selected_consumer;
    if (!c) { output("Error: No consumer selected"); return; }

    std::stringstream ss;
    ss << "NEEDS & CONSUMPTION: " << c->name << "\n";
    ss << std::string(40, '-') << "\n";
    for (auto& need : c->needs) {
        product* p = &need;
        ss << need.name 
           << " | Demand: P = " << c->dd[p].c << " - " << c->dd[p].m << "Q"
           << " | Consumed: " << c->consumed[p] << "\n";
    }
    output(ss.str());
}
```

---

### 6. Farmer Analysis Commands

```cpp
{"farmer supply", "Calculate supply at a price", {{"product", "Crop name"}, {"price", "Market price"}}},
{"farmer crops", "Show farmer crops and supply curves", {}},
{"farmer upgrade", "Upgrade farmer tech level", {{"level", "New tech level 0-1"}}},
{"farmer weather", "Show current weather effect", {}},
```

**Implementation:**
```cpp
void cmdFarmerSupply(const Command &cmd) {
    farmer* f = simulation.selected_farmer;
    if (!f) { output("Error: No farmer selected"); return; }

    std::string prodName = std::get<std::string>(cmd.params.at("product"));
    double price = std::get<double>(cmd.params.at("price"));
    product* p = getProductByName(prodName);
    if (!p) { output("Error: Unknown product"); return; }

    double qty = f->calculateSupply(p, price);

    std::stringstream ss;
    ss << "SUPPLY CALCULATION: " << f->name << " → " << prodName << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Market Price: $" << price << "\n";
    ss << "Supply Curve: P = " << f->ss[p].c << " + " << f->ss[p].m << "Q\n";
    ss << "Labor Cost: $" << f->laborCost << " | Tax: $" << f->tax << "\n";
    ss << "Tech Level: " << f->techLevel << " | Weather: " << std::fixed << std::setprecision(2) << f->weather << "\n";
    ss << "Max Output: " << f->maxOutput[p] << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Quantity Supplied: " << std::fixed << std::setprecision(2) << qty << " units\n";
    output(ss.str());
}

void cmdFarmerCrops(const Command &cmd) {
    farmer* f = simulation.selected_farmer;
    if (!f) { output("Error: No farmer selected"); return; }

    std::stringstream ss;
    ss << "CROPS: " << f->name << "\n";
    ss << std::string(50, '-') << "\n";
    for (auto& crop : f->crops) {
        product* p = &crop;
        ss << crop.name << "\n";
        ss << "  Supply: P = " << f->ss[p].c << " + " << f->ss[p].m << "Q\n";
        ss << "  Growth: " << f->growthRate[p] << "/day | Decay: " << f->decay[p] << "/day\n";
        ss << "  Max Output: " << f->maxOutput[p] << " units\n\n";
    }
    output(ss.str());
}

void cmdFarmerUpgrade(const Command &cmd) {
    farmer* f = simulation.selected_farmer;
    if (!f) { output("Error: No farmer selected"); return; }

    double level = std::get<double>(cmd.params.at("level"));
    double oldLevel = f->techLevel;
    f->upgradeTech(level);

    std::stringstream ss;
    ss << "TECH UPGRADE: " << f->name << "\n";
    ss << "Tech Level: " << oldLevel << " → " << level << "\n";
    output(ss.str());
}

void cmdFarmerWeather(const Command &cmd) {
    farmer* f = simulation.selected_farmer;
    if (!f) { output("Error: No farmer selected"); return; }

    std::stringstream ss;
    ss << "WEATHER: " << f->name << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Current Weather Factor: " << std::fixed << std::setprecision(2) << f->weather << "\n";
    ss << "(0 = Perfect, 1 = Severe conditions)\n";
    ss << "Effect: Slope multiplier on supply curve\n";
    output(ss.str());
}
```

---

### 7. Firm Analysis Commands

```cpp
{"firm costs", "Calculate all cost metrics", {}},
{"firm output", "Show current production output", {}},
{"firm mp", "Show marginal products of L and K", {}},
{"firm efficiency", "Show labor vs capital efficiency", {}},
{"firm hire", "Add a laborer to the firm", {{"laborer", "Laborer name"}}},
{"firm capital", "Add capital to the firm", {{"rental", "Rental rate"}, {"eff", "Efficiency"}}},
```

**Implementation:**
```cpp
void cmdFirmCosts(const Command &cmd) {
    // Find a firm (use first one or add selection)
    if (simulation.firms.empty()) { output("Error: No firms"); return; }
    firm* f = &simulation.firms[0]; // Could add firm selection

    f->calculateCosts();

    std::stringstream ss;
    ss << "FIRM COST ANALYSIS (Owner ID: " << f->ownerId << ")\n";
    ss << std::string(50, '-') << "\n";
    ss << "Workers: " << f->workers.size() << " | Capital: " << f->capitals.size() << "\n";
    ss << "Current Output (Q): " << std::fixed << std::setprecision(2) << f->currentOutput << "\n\n";
    ss << "COSTS:\n";
    ss << "  Total Fixed Cost (TFC):     $" << f->totalFixedCost << "\n";
    ss << "  Total Variable Cost (TVC):  $" << f->totalVariableCost << "\n";
    ss << "  Total Cost (TC):            $" << f->totalCost << "\n\n";
    ss << "AVERAGES:\n";
    ss << "  Average Fixed Cost (AFC):   $" << f->averageFixedCost << "\n";
    ss << "  Average Variable Cost (AVC):$" << f->averageVariableCost << "\n";
    ss << "  Average Cost (AC):          $" << f->averageCost << "\n\n";
    ss << "MARGINAL:\n";
    ss << "  Marginal Cost (MC):         $" << f->marginalCost << "\n";

    if (f->marginalCost < f->averageCost) {
        ss << "\n→ Economies of Scale (MC < AC)\n";
    } else {
        ss << "\n→ Diminishing Returns (MC > AC)\n";
    }
    output(ss.str());
}

void cmdFirmOutput(const Command &cmd) {
    if (simulation.firms.empty()) { output("Error: No firms"); return; }
    firm* f = &simulation.firms[0];

    double L = f->workers.size();
    double K = f->capitals.size();
    double Q = f->prodFunc->output(L, K);

    std::stringstream ss;
    ss << "FIRM OUTPUT (Owner ID: " << f->ownerId << ")\n";
    ss << std::string(40, '-') << "\n";
    ss << "Labor (L): " << (int)L << " workers\n";
    ss << "Capital (K): " << (int)K << " units\n";
    ss << "Production Function: Cobb-Douglas\n";
    ss << "  α = " << f->cdProd.alpha << ", β = " << f->cdProd.beta << ", A = " << f->cdProd.tech << "\n";
    ss << std::string(40, '-') << "\n";
    ss << "Output (Q): " << std::fixed << std::setprecision(2) << Q << " units\n";
    output(ss.str());
}

void cmdFirmMP(const Command &cmd) {
    if (simulation.firms.empty()) { output("Error: No firms"); return; }
    firm* f = &simulation.firms[0];

    double mpL = f->MPofLabor();
    double mpK = f->MPofCapital();

    std::stringstream ss;
    ss << "MARGINAL PRODUCTS (Owner ID: " << f->ownerId << ")\n";
    ss << std::string(40, '-') << "\n";
    ss << "Marginal Product of Labor (MPL):   " << std::fixed << std::setprecision(4) << mpL << "\n";
    ss << "  → Adding 1 worker increases output by " << mpL << " units\n\n";
    ss << "Marginal Product of Capital (MPK): " << std::fixed << std::setprecision(4) << mpK << "\n";
    ss << "  → Adding 1 machine increases output by " << mpK << " units\n";
    output(ss.str());
}

void cmdFirmEfficiency(const Command &cmd) {
    if (simulation.firms.empty()) { output("Error: No firms"); return; }
    firm* f = &simulation.firms[0];

    auto ratios = f->marginalCosts();

    std::stringstream ss;
    ss << "FACTOR EFFICIENCY (Owner ID: " << f->ownerId << ")\n";
    ss << std::string(40, '-') << "\n";
    ss << "Labor Efficiency (MPL/w):   " << std::fixed << std::setprecision(4) << ratios[0] << "\n";
    ss << "Capital Efficiency (MPK/r): " << std::fixed << std::setprecision(4) << ratios[1] << "\n\n";

    double diff = std::abs(ratios[0] - ratios[1]);
    if (diff < 0.05) {
        ss << "STATUS: Optimal Mix (Isoquant tangent to Isocost)\n";
    } else if (ratios[0] > ratios[1]) {
        ss << "RECOMMENDATION: Labor is more efficient. HIRE WORKER.\n";
    } else {
        ss << "RECOMMENDATION: Capital is more efficient. BUY MACHINE.\n";
    }
    output(ss.str());
}

void cmdFirmHire(const Command &cmd) {
    if (simulation.firms.empty()) { output("Error: No firms"); return; }
    firm* f = &simulation.firms[0];

    std::string name = std::get<std::string>(cmd.params.at("laborer"));
    for (auto& l : simulation.laborers) {
        if (l.name == name) {
            f->workers.push_back(l);
            f->calculateCosts();
            output("Hired " + name + ". New output: " + std::to_string(f->currentOutput));
            return;
        }
    }
    output("Error: Laborer not found: " + name);
}

void cmdFirmCapital(const Command &cmd) {
    if (simulation.firms.empty()) { output("Error: No firms"); return; }
    firm* f = &simulation.firms[0];

    double rental = std::get<double>(cmd.params.at("rental"));
    double eff = std::get<double>(cmd.params.at("eff"));
    
    f->capitals.emplace_back(rental, eff);
    f->calculateCosts();
    output("Added capital. New output: " + std::to_string(f->currentOutput));
}
```

---

### 8. Simulation Commands

```cpp
{"pass day", "Advance simulation by one day", {}},
{"status", "Show economic statistics", {}},
{"help", "Show available commands", {{"command", "Command name (optional)"}}},
{"clear", "Clear screen", {}},
{"exit", "Exit simulation", {}},
```

**Implementation:**
```cpp
void cmdPassDay(const Command &cmd) {
    // Pass day for all entities
    for (auto& c : simulation.consumers) c.pass_day();
    for (auto& l : simulation.laborers) l.pass_day();
    for (auto& f : simulation.farmers) f.pass_day();

    // Recalculate market equilibria
    for (auto& m : simulation.markets) {
        m.calculateAggregateDemand(simulation.consumers);
        m.calculateAggregateSupply(simulation.farmers);
        auto eq = m.findEquilibrium();
        m.price = eq.price;
    }

    // Update stats
    updateStats();

    output("Day passed. All entities updated.");
}

void updateStats() {
    auto& s = simulation.currentStats;
    
    // Population
    s.population = simulation.consumers.size() + simulation.laborers.size() + simulation.farmers.size();
    
    // Employed (laborers in firms)
    s.employed = 0;
    for (auto& f : simulation.firms) {
        s.employed += f.workers.size();
    }
    
    // Unemployment rate
    int laborForce = simulation.laborers.size();
    s.unemployment = laborForce > 0 ? (1.0 - (double)s.employed / laborForce) * 100.0 : 0.0;
    
    // Firm count
    s.firms = simulation.firms.size();
    
    // GDP (sum of firm outputs * market prices - rough approximation)
    s.gdp = 0.0;
    for (auto& f : simulation.firms) {
        f.calculateCosts();
        // Approximate: output * average product price
        if (!f.products.empty()) {
            for (auto& m : simulation.markets) {
                if (m.prod->name == f.products[0].name) {
                    s.gdp += f.currentOutput * m.price;
                    break;
                }
            }
        }
    }
}
```

---

## Helper Functions

```cpp
// Get market from command params or selected
market* getMarketFromCmd(const Command &cmd) {
    if (cmd.params.find("product") != cmd.params.end()) {
        std::string name = std::get<std::string>(cmd.params.at("product"));
        for (auto& m : simulation.markets) {
            if (m.prod->name == name) return &m;
        }
    }
    return simulation.selected_market;
}

// Get product by name
product* getProductByName(const std::string& name) {
    std::vector<product*> prods = {&rice, &cloth, &computer, &phone, &car, &steel};
    for (auto* p : prods) {
        if (p->name == name) return p;
    }
    return nullptr;
}

// Get market by product
market* getMarketByProduct(product* p) {
    for (auto& m : simulation.markets) {
        if (m.prod == p) return &m;
    }
    return nullptr;
}
```

---

## Updated availableCommands for CLI Autocomplete

```cpp
std::vector<std::string> availableCommands = {
    // Utility
    "help", "status", "clear", "cls", "exit", "quit", "pass day",
    
    // Entity listing
    "consumers", "laborers", "farmers", "firms", "markets", "products",
    
    // Entity creation
    "add consumer", "add laborer", "add farmer", "add firm",
    
    // Selection
    "select consumer", "select laborer", "select farmer", "select market", "clear selection",
    
    // Market analysis
    "market equilibrium", "market demand", "market supply",
    
    // Consumer analysis
    "consumer mu", "consumer surplus", "consumer substitution", "consumer needs",
    
    // Farmer analysis
    "farmer supply", "farmer crops", "farmer upgrade", "farmer weather",
    
    // Firm analysis
    "firm costs", "firm output", "firm mp", "firm efficiency", "firm hire", "firm capital"
};
```

---

## Commands NOT Implemented (No Available Math)

The following commands from the old system **cannot** be implemented with current `.h` files:

| Command | Reason |
|---------|--------|
| `market.elasticity` | No elasticity calculation function exists |
| `market.welfare` | No producer surplus calculation in farmer.h |
| `market.supply_shock` | Requires external event system |
| `market.tax` | No tax application function on market |
| `market.subsidy` | No subsidy mechanism |
| `market.price_control` | No ceiling/floor enforcement logic |
| `consumer.optimize_bundle` | No utility maximization solver |
| `gov.calculate_gdp` | Need full expenditure/income tracking |
| `stats.cpi` | Need basket weights and price history tracking |
| `stats.inflation` | Need CPI over time (requires state persistence) |
| `centralBank.monetary_policy` | No money supply mechanics |
| `gov.set_policy` | No government entity |
| `economy.ppf` | No multi-sector production frontier |
| `tech.upgrade` | Generic; farmer.upgradeTech exists but sector-wide doesn't |
| `system.show_changes` | Requires change tracking system |
| `system.dependency_chain` | Requires dependency graph |
| All `propagation.*` commands | No propagation system |

---

## Summary

This plan provides **30+ implementable commands** using only the existing methods in your `.h` files:

- **6** listing commands
- **4** creation commands  
- **5** selection commands
- **3** market commands
- **4** consumer commands
- **4** farmer commands
- **6** firm commands
- **5** utility commands
