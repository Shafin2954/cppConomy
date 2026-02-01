# CppConomy Development Plan

## Project Overview

**Project Name:** CppConomy  
**Type:** Real-Time Object-Oriented Economic Simulator  
**Language:** C++  
**GUI Framework:** wxWidgets  
**Target Platform:** Windows (cross-platform capable)

CppConomy is a GUI-based simulation engine that models a national economy from the bottom up using Agent-Based Modeling (ABM). Macro-economic indicators (GDP, Inflation, Unemployment) emerge dynamically from individual agent interactions.

---

## 1. Technology Stack

| Component | Technology |
|-----------|------------|
| Language | C++17 or later |
| GUI Framework | wxWidgets 3.2+ |
| Build System | CMake |
| Compiler | MSVC / MinGW-w64 |
| Random Generation | `<random>` (Mersenne Twister) |
| Threading | `std::thread` / wxWidgets threading |

---

## 2. Project Structure

```
cppConomy/
├── CMakeLists.txt
├── plan.md
├── README.md
│
├── src/
│   ├── main.cpp                    # Application entry point
│   │
│   ├── core/                       # Economic simulation engine
│   │   ├── Agent.h                 # Base agent class
│   │   ├── Agent.cpp
│   │   ├── Person.h                # Citizen agent
│   │   ├── Person.cpp
│   │   ├── Firm.h                  # Business agent
│   │   ├── Firm.cpp
│   │   ├── Government.h            # Government singleton
│   │   ├── Government.cpp
│   │   ├── Economy.h               # Main simulation controller
│   │   ├── Economy.cpp
│   │   ├── Market.h                # Goods/Labor market
│   │   └── Market.cpp
│   │
│   ├── models/                     # Economic models & formulas
│   │   ├── PhillipsCurve.h
│   │   ├── OkunLaw.h
│   │   ├── QuantityTheory.h        # MV = PQ
│   │   ├── UtilityFunction.h       # Marginal utility calculations
│   │   └── StochasticShock.h       # Monte Carlo events
│   │
│   ├── gui/                        # wxWidgets GUI components
│   │   ├── MainFrame.h             # Main application window
│   │   ├── MainFrame.cpp
│   │   ├── DashboardPanel.h        # Real-time statistics display
│   │   ├── DashboardPanel.cpp
│   │   ├── EventLogPanel.h         # Economic event log
│   │   ├── EventLogPanel.cpp
│   │   ├── CommandPanel.h          # Command input with autocomplete
│   │   ├── CommandPanel.cpp
│   │   ├── HistoryPanel.h          # Command history sidebar
│   │   ├── HistoryPanel.cpp
│   │   ├── ChartPanel.h            # Data visualization (optional)
│   │   └── ChartPanel.cpp
│   │
│   ├── commands/                   # Command parsing & execution
│   │   ├── CommandParser.h         # Parse Python-like syntax
│   │   ├── CommandParser.cpp
│   │   ├── CommandExecutor.h       # Execute parsed commands
│   │   └── CommandExecutor.cpp
│   │
│   └── utils/                      # Utilities
│       ├── Logger.h
│       ├── Logger.cpp
│       ├── Statistics.h            # Statistical calculations
│       └── Config.h                # Simulation parameters
│
├── include/                        # Public headers (if needed)
│
├── resources/                      # Assets
│   ├── icons/
│   └── themes/
│
└── tests/                          # Unit tests
    ├── test_economy.cpp
    ├── test_agents.cpp
    └── test_commands.cpp
```

---

## 3. Core Architecture

### 3.1 Class Hierarchy

```
Agent (Abstract Base)
├── Person
│   ├── Worker
│   ├── Unemployed
│   └── Retired
└── Firm
    ├── Producer
    └── Retailer

Government (Singleton)
Economy (Simulation Controller)
Market (Exchange Mechanism)
```

### 3.2 Agent Classes

#### `class Agent` (Abstract Base)
```cpp
class Agent {
protected:
    int id;
    double wealth;
    bool active;
public:
    virtual void update(Economy& economy) = 0;
    virtual ~Agent() = default;
};
```

#### `class Person : public Agent`
| Attribute | Type | Description |
|-----------|------|-------------|
| `age` | `int` | Current age (0-100) |
| `wealth` | `double` | Current money holdings |
| `skills` | `std::vector<Skill>` | Skill set for employment |
| `employed` | `bool` | Employment status |
| `employer` | `Firm*` | Pointer to employer (if any) |
| `needs` | `NeedVector` | Basic needs (food, housing, etc.) |

| Method | Description |
|--------|-------------|
| `consume()` | Purchase goods (Law of Demand + Marginal Utility) |
| `save()` | Allocate income (Life-Cycle Hypothesis) |
| `seekJob()` | Search labor market |
| `age_tick()` | Age by one period |
| `die()` | Stochastic death (age/health probability) |

#### `class Firm : public Agent`
| Attribute | Type | Description |
|-----------|------|-------------|
| `capital` | `double` | Production capital |
| `inventory` | `double` | Current stock of goods |
| `price` | `double` | Current unit price |
| `laborForce` | `std::vector<Person*>` | Employed workers |
| `sector` | `Sector` | Industry sector enum |

| Method | Description |
|--------|-------------|
| `produce()` | Output where MC = MR |
| `setPrice()` | Adjust based on inventory (supply/demand) |
| `hire()` | Add workers from labor market |
| `fire()` | Remove workers |
| `payWages()` | Distribute wages to employees |

#### `class Government` (Singleton)
| Attribute | Type | Description |
|-----------|------|-------------|
| `taxRate` | `double` | Income/corporate tax rate |
| `interestRate` | `double` | Central bank rate |
| `moneySupply` | `double` | Total currency in circulation |
| `budget` | `double` | Government treasury |

| Method | Description |
|--------|-------------|
| `adjustTax()` | Modify tax rates |
| `setInterestRate()` | Monetary policy |
| `injectMoney()` | Quantitative easing |
| `grantStimulus()` | Direct transfers |
| `collectTaxes()` | Revenue collection |

### 3.3 Economy Controller

```cpp
class Economy {
private:
    std::vector<std::unique_ptr<Person>> citizens;
    std::vector<std::unique_ptr<Firm>> firms;
    Government& government;
    Market laborMarket;
    Market goodsMarket;
    
    // Aggregates (emergent)
    double gdp;
    double cpi;           // Consumer Price Index
    double unemployment;
    double inflation;
    
public:
    void tick();          // Advance simulation one period
    void addAgent();
    void removeAgent();
    void calculateAggregates();
    void triggerShock(ShockType type);
};
```

---

## 4. GUI Design (wxWidgets)

### 4.1 Window Layout

```
┌─────────────────────────────────────────────────────────────────────────┐
│  CppConomy - Economic Simulator                              [_][□][X] │
├─────────────────────────────────────────────────────┬───────────────────┤
│                                                     │  COMMAND HISTORY  │
│   ┌─────────────────────────────────────────────┐   │                   │
│   │         DASHBOARD PANEL                     │   │  > adjust_tax(... │
│   │                                             │   │  > print_stats(.. │
│   │  GDP: $12.5T    ▲ 2.3%    Unemployment: 4.2%│   │  > grant_stimulus │
│   │  CPI: 142.3     Inflation: 2.1%             │   │  > set_interest.. │
│   │  Money Supply: $8.2T    Interest: 3.5%      │   │  > simulate(tick. │
│   │                                             │   │                   │
│   └─────────────────────────────────────────────┘   │                   │
│                                                     │                   │
│   ┌─────────────────────────────────────────────┐   │                   │
│   │         EVENT LOG PANEL                     │   │                   │
│   │                                             │   │                   │
│   │  [INFO]  Tick 1042: GDP grew by 0.3%        │   │                   │
│   │  [WARN]  Inflation rising: 2.1% → 2.4%      │   │                   │
│   │  [ALERT] Unemployment spike in manufacturing│   │                   │
│   │  [INFO]  Firm #2847 hired 12 workers        │   │                   │
│   │  [INFO]  Tax revenue collected: $340B       │   │                   │
│   │                                             │   │                   │
│   └─────────────────────────────────────────────┘   │                   │
│                                                     │                   │
├─────────────────────────────────────────────────────┴───────────────────┤
│  ┌─ Suggestions: adjust_tax | adjust_interest | add_stimulus ─────────┐ │
│  │ > adjust_tax(rate=0.15, target="corporate")                      │ │
│  └────────────────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────────────┘
```

### 4.2 wxWidgets Components

| Panel | wxWidget Class | Description |
|-------|----------------|-------------|
| MainFrame | `wxFrame` | Top-level window |
| DashboardPanel | `wxPanel` + `wxStaticText` | Real-time stat display |
| EventLogPanel | `wxListCtrl` / `wxRichTextCtrl` | Scrollable event log |
| HistoryPanel | `wxListBox` | Command history list |
| CommandPanel | `wxTextCtrl` + `wxPopupWindow` | Input with autocomplete |

### 4.3 Theme: Light Mode

```cpp
// Color scheme
const wxColour BG_COLOR(255, 255, 255);        // White background
const wxColour TEXT_COLOR(30, 30, 30);         // Near-black text
const wxColour ACCENT_COLOR(0, 120, 212);      // Blue accent
const wxColour SUCCESS_COLOR(16, 124, 16);     // Green for positive
const wxColour WARNING_COLOR(255, 140, 0);     // Orange for warnings
const wxColour ALERT_COLOR(220, 38, 38);       // Red for alerts
const wxColour BORDER_COLOR(200, 200, 200);    // Light gray borders
```

### 4.4 UX Features

1. **Autocomplete System**
   - Popup suggestions as user types
   - Tab to complete
   - Show parameter hints

2. **Command History**
   - Up/Down arrow key navigation
   - Persistent history (save to file)
   - Click to re-execute

3. **Visual Feedback**
   - Color-coded log entries
   - Animated stat changes (↑↓ indicators)
   - Status bar with simulation state

---

## 5. Command System

### 5.1 Syntax Format

```
function_name(param1=value1, param2=value2, ...)
```

### 5.2 Available Commands

| Command | Parameters | Description |
|---------|------------|-------------|
| `adjust_tax` | `rate`, `target` | Set tax rate for target group |
| `set_interest` | `rate` | Set central bank interest rate |
| `inject_money` | `amount` | Increase money supply |
| `grant_stimulus` | `amount`, `sector` | Direct payments |
| `print_stats` | `variable` | Display specific statistic |
| `simulate` | `ticks` | Run N simulation ticks |
| `pause` | - | Pause simulation |
| `resume` | - | Resume simulation |
| `trigger_shock` | `type`, `severity` | Manual shock event |
| `add_firms` | `count`, `sector` | Spawn new firms |
| `add_citizens` | `count` | Spawn new citizens |
| `export_data` | `filename` | Export to CSV |
| `help` | `command` | Show help |

### 5.3 Command Parser Implementation

```cpp
struct Command {
    std::string name;
    std::map<std::string, std::variant<int, double, std::string>> params;
};

class CommandParser {
public:
    Command parse(const std::string& input);
private:
    std::string extractName(const std::string& input);
    std::map<std::string, Value> extractParams(const std::string& input);
};
```

---

## 6. Economic Models

### 6.1 Phillips Curve
$$\pi = \pi^e - \beta(u - u^*)$$

Where:
- $\pi$ = Actual inflation
- $\pi^e$ = Expected inflation  
- $u$ = Unemployment rate
- $u^*$ = Natural unemployment rate
- $\beta$ = Sensitivity coefficient

### 6.2 Okun's Law
$$\frac{Y - Y^*}{Y^*} = -\gamma(u - u^*)$$

Where:
- $Y$ = Actual GDP
- $Y^*$ = Potential GDP
- $\gamma$ = Okun coefficient (~2-3)

### 6.3 Quantity Theory of Money
$$MV = PQ$$

Where:
- $M$ = Money supply
- $V$ = Velocity of money
- $P$ = Price level
- $Q$ = Real output

### 6.4 Utility & Demand

**Marginal Utility:**
$$MU = \frac{\partial U}{\partial Q} = \alpha Q^{-\beta}$$

**Demand Function:**
$$Q_d = f(P, I, P_s, P_c, T)$$

### 6.5 Production Function (Cobb-Douglas)
$$Y = A \cdot K^\alpha \cdot L^{1-\alpha}$$

### 6.6 Stochastic Shocks (Monte Carlo)

| Shock Type | Probability | Impact |
|------------|-------------|--------|
| Pandemic | 0.5% / tick | -10% GDP, +5% unemployment |
| Market Crash | 1% / tick | -15% wealth, -5% investment |
| Tech Boom | 2% / tick | +3% productivity |
| Natural Disaster | 0.3% / tick | Regional damage |
| Oil Shock | 1% / tick | +20% production costs |

---

## 7. Development Phases

### Phase 1: Foundation (Week 1-2)
- [ ] Set up CMake project with wxWidgets
- [ ] Create basic MainFrame window
- [ ] Implement Agent base class
- [ ] Implement Person class (basic)
- [ ] Implement Firm class (basic)
- [ ] Basic Economy tick loop

### Phase 2: Core Simulation (Week 3-4)
- [ ] Implement Market class (labor + goods)
- [ ] Person behaviors: consume, save, seekJob
- [ ] Firm behaviors: produce, setPrice, hire/fire
- [ ] Government class with policies
- [ ] Aggregate calculations

### Phase 3: Economic Models (Week 5-6)
- [ ] Phillips Curve integration
- [ ] Okun's Law integration
- [ ] Quantity Theory (MV=PQ)
- [ ] Utility functions
- [ ] Stochastic shock system

### Phase 4: GUI Development (Week 7-8)
- [ ] Dashboard panel with live stats
- [ ] Event log panel with colors
- [ ] Command input with autocomplete
- [ ] History panel
- [ ] Timer-based UI updates

### Phase 5: Command System (Week 9)
- [ ] Command parser (Python-like syntax)
- [ ] Command executor
- [ ] All government commands
- [ ] Help system
- [ ] Command validation

### Phase 6: Polish & Testing (Week 10)
- [ ] Unit tests for economic models
- [ ] Integration tests
- [ ] Performance optimization
- [ ] Data export functionality
- [ ] Documentation

---

## 8. Build Instructions

### Prerequisites
```powershell
# Install vcpkg (if not installed)
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat

# Install wxWidgets
.\vcpkg\vcpkg install wxwidgets:x64-windows
```

### CMake Configuration
```cmake
cmake_minimum_required(VERSION 3.16)
project(CppConomy VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})

add_executable(CppConomy WIN32
    src/main.cpp
    # ... other sources
)

target_link_libraries(CppConomy PRIVATE ${wxWidgets_LIBRARIES})
```

### Build Commands
```powershell
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg-root]/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release
```

---

## 9. Configuration Parameters

```cpp
// config.h
namespace Config {
    // Simulation
    constexpr int INITIAL_POPULATION = 10000;
    constexpr int INITIAL_FIRMS = 500;
    constexpr int TICKS_PER_YEAR = 12;  // Monthly ticks
    
    // Economic
    constexpr double INITIAL_MONEY_SUPPLY = 1e12;
    constexpr double NATURAL_UNEMPLOYMENT = 0.04;
    constexpr double TARGET_INFLATION = 0.02;
    
    // Behavioral
    constexpr double SAVINGS_RATE_YOUNG = -0.05;   // Borrow
    constexpr double SAVINGS_RATE_MIDDLE = 0.15;   // Save
    constexpr double SAVINGS_RATE_OLD = -0.08;     // Dissave
    
    // Stochastic
    constexpr double SHOCK_PROBABILITY = 0.02;
    constexpr int RANDOM_SEED = 42;  // For reproducibility
}
```

---

## 10. Future Enhancements

- [ ] International trade (multiple economies)
- [ ] Financial markets (stocks, bonds)
- [ ] Banking sector with lending
- [ ] Real estate market
- [ ] Education & skill development
- [ ] Graphical charts (wxCharts / wxMathPlot)
- [ ] Scenario save/load
- [ ] Network multiplayer (competing economies)

---

## References

1. Mankiw, N.G. - *Macroeconomics*
2. Varian, H.R. - *Intermediate Microeconomics*
3. wxWidgets Documentation: https://docs.wxwidgets.org/
4. Agent-Based Computational Economics (ACE) literature
