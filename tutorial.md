# CppConomy Tutorial

A comprehensive guide to understanding, using, and extending the CppConomy economic simulator.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Available Commands](#available-commands)
3. [Adding New Commands](#adding-new-commands)
4. [Adding New Economic Models](#adding-new-economic-models)
5. [Adding New Agent Types](#adding-new-agent-types)
6. [GUI Customization](#gui-customization)
7. [Configuration Options](#configuration-options)
8. [Troubleshooting](#troubleshooting)

---

## Getting Started

### Running the Simulation

1. **Start**: Press F5 or use the menu `Simulation > Start`
2. **Pause**: Press F6 or use the menu `Simulation > Pause`
3. **Reset**: Press Ctrl+R or use the menu `Simulation > Reset`

### Basic Commands

Type commands in the command panel at the bottom:

```
help()                          # Show all available commands
simulate(ticks=100)             # Run 100 simulation ticks
print_stats(variable="all")     # Display all economic statistics
adjust_tax(rate=0.15)           # Set tax rate to 15%
```

---

## Available Commands

| Command | Description | Parameters |
|---------|-------------|------------|
| `adjust_tax` | Set tax rate | `rate` (0.0-1.0), `target` ("income", "corporate", "all") |
| `set_interest` | Set interest rate | `rate` (0.0-0.25) |
| `inject_money` | Quantitative easing | `amount` (money to inject) |
| `grant_stimulus` | Direct payments | `amount`, `sector` (optional) |
| `print_stats` | Display statistics | `variable` ("gdp", "inflation", "unemployment", "all") |
| `simulate` | Run simulation | `ticks` (number of ticks) |
| `pause` | Pause simulation | - |
| `resume` | Resume simulation | - |
| `trigger_shock` | Trigger economic event | `type`, `severity` (0.1-2.0) |
| `add_firms` | Add businesses | `count`, `sector` |
| `add_citizens` | Add population | `count` |
| `export_data` | Export to CSV | `filename` |
| `help` | Show help | `command` (optional) |
| `clear` | Clear event log | - |
| `reset` | Reset simulation | - |
| `status` | Show simulation status | - |

---

## Adding New Commands

### Step 1: Define the Command Info

Open `src/commands/CommandParser.cpp` and add your command to the `m_commands` vector in the constructor:

```cpp
CommandParser::CommandParser()
{
    m_commands = {
        // ... existing commands ...
        
        // Add your new command here
        {"my_command", 
         "Description of what my command does", 
         {
             {"param1", "Description of param1"},
             {"param2", "Description of param2 (optional)"}
         }
        },
    };
}
```

### Step 2: Add the Command Handler

Open `src/commands/CommandExecutor.cpp`:

#### 2a. Add to the execute() switch

In the `execute(const Command& cmd)` function, add your command:

```cpp
bool CommandExecutor::execute(const Command &cmd)
{
    // ... existing code ...
    
    else if (cmd.name == "my_command")
        cmdMyCommand(cmd);
    
    // ... rest of code ...
}
```

#### 2b. Implement the handler function

Add your implementation at the end of the file:

```cpp
void CommandExecutor::cmdMyCommand(const Command &cmd)
{
    // Get parameters with defaults
    double param1 = getParam<double>(cmd, "param1", 0.0);
    std::string param2 = getParam<std::string>(cmd, "param2", "default");
    
    // Validate parameters
    if (param1 < 0 || param1 > 100) {
        output("Error: param1 must be between 0 and 100");
        return;
    }
    
    // Execute the command logic
    // Access economy via m_economy
    // Example: m_economy.someMethod(param1);
    
    // Output result to event log
    std::stringstream ss;
    ss << "My command executed with param1=" << param1;
    output(ss.str());
    
    // Or use logging macros for colored output:
    LOG_SUCCESS("Command completed successfully");
    LOG_WARNING("This is a warning");
    LOG_ALERT("This is an alert");
}
```

### Step 3: Declare the Handler

Open `src/commands/CommandExecutor.h` and add the function declaration:

```cpp
private:
    // ... existing handlers ...
    void cmdMyCommand(const Command& cmd);
```

### Step 4: Rebuild

Rebuild the project using CMake in VS Code.

---

## Adding New Economic Models

### Step 1: Create the Model Header

Create a new file in `src/models/`, e.g., `MyModel.h`:

```cpp
#pragma once

#include <cmath>

class MyModel {
public:
    // Calculate something based on inputs
    static double calculate(double input1, double input2) {
        // Your economic formula here
        return input1 * input2;
    }
    
    // Another calculation
    static double predict(double current, double growth) {
        return current * (1.0 + growth);
    }
};
```

### Step 2: Use the Model

Include and use your model in `Economy.cpp`:

```cpp
#include "../models/MyModel.h"

void Economy::tick() {
    // ... existing code ...
    
    double result = MyModel::calculate(m_gdp, m_inflation);
    // Use the result...
}
```

### Existing Economic Models

- **PhillipsCurve.h**: Inflation-unemployment tradeoff
- **OkunLaw.h**: GDP-unemployment relationship
- **QuantityTheory.h**: Money supply and price level (MV = PQ)
- **UtilityFunction.h**: Consumer utility calculations
- **StochasticShock.h**: Random economic events

---

## Adding New Agent Types

### Step 1: Create the Agent Class

Create header and implementation files in `src/core/`:

**MyAgent.h:**
```cpp
#pragma once

#include "Agent.h"

class MyAgent : public Agent {
public:
    MyAgent(int id);
    
    void update(Economy& economy) override;
    
    // Custom methods
    void doSomething();
    double getValue() const { return m_value; }
    
private:
    double m_value;
};
```

**MyAgent.cpp:**
```cpp
#include "MyAgent.h"
#include "Economy.h"

MyAgent::MyAgent(int id) 
    : Agent(id), m_value(0.0) 
{
}

void MyAgent::update(Economy& economy) {
    if (!isActive()) return;
    
    // Agent behavior logic here
    doSomething();
}

void MyAgent::doSomething() {
    // Implementation
}
```

### Step 2: Add to Economy

Modify `Economy.h` to include a container for your agents:

```cpp
#include "MyAgent.h"

class Economy {
    // ...
private:
    std::vector<std::unique_ptr<MyAgent>> m_myAgents;
};
```

### Step 3: Update CMakeLists.txt

Add the new .cpp file to the SOURCES list:

```cmake
set(SOURCES
    # ... existing sources ...
    src/core/MyAgent.cpp
)
```

---

## GUI Customization

### Panel Layout

Panel sizes are configured in `MainFrame.cpp` in `CreatePanels()`:

```cpp
// Right sidebar width (20% of window)
int leftWidth = static_cast<int>(width * 0.80);
mainSplitter->SplitVertically(leftContainer, m_historyPanel, leftWidth);

// Top panel height (40% of window)
int topHeight = static_cast<int>(height * 0.40);
topSplitter->SplitHorizontally(m_dashboard, m_eventLog, topHeight);
```

### Adding a New Panel

1. Create `MyPanel.h` and `MyPanel.cpp` in `src/gui/`
2. Add to `MainFrame.h`:
   ```cpp
   class MyPanel;
   // ...
   MyPanel* m_myPanel;
   ```
3. Create and add to layout in `MainFrame::CreatePanels()`
4. Add to CMakeLists.txt SOURCES

### Customizing Colors

Colors are defined in each panel's constructor. Example from `EventLogPanel.cpp`:

```cpp
m_colorInfo(30, 30, 30)      // Dark gray
m_colorWarning(255, 140, 0)  // Orange
m_colorAlert(220, 38, 38)    // Red
m_colorSuccess(16, 124, 16)  // Green
```

---

## Configuration Options

All simulation constants are in `src/utils/Config.h`:

### Population Settings
```cpp
static constexpr int INITIAL_POPULATION = 1000;
static constexpr int INITIAL_FIRMS = 50;
static constexpr double BIRTH_RATE = 0.02;
static constexpr double DEATH_RATE = 0.015;
```

### Economic Parameters
```cpp
static constexpr double TAX_RATE = 0.20;
static constexpr double INTEREST_RATE = 0.03;
static constexpr double INITIAL_WAGE = 50000.0;
static constexpr double INITIAL_PRICE = 100.0;
```

### Simulation Settings
```cpp
static constexpr int TICKS_PER_YEAR = 12;  // Monthly ticks
static constexpr int GUI_UPDATE_INTERVAL = 100;  // ms
static constexpr int SIM_TICK_INTERVAL = 500;    // ms per tick
```

### Modifying Constants

Simply edit the values in `Config.h` and rebuild. No other changes needed.

---

## Troubleshooting

### Common Issues

**Build Errors:**
- Ensure wxWidgets is installed: `pacman -S mingw-w64-ucrt-x86_64-wxwidgets3.2-msw`
- Check CMake generator is set to "MinGW Makefiles"
- Run "CMake: Delete Cache and Reconfigure"

**Runtime Issues:**
- "Permission denied" when building: Close the running .exe first
- GUI not updating: Check that timers are started in MainFrame constructor

**IntelliSense Errors:**
- Add to `.vscode/settings.json`:
  ```json
  "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
  ```
- Run "Developer: Reload Window"

### Logging for Debugging

Use the logging macros anywhere in the code:

```cpp
#include "../utils/Logger.h"

LOG_INFO("Information message");
LOG_WARNING("Warning message");
LOG_ALERT("Alert/error message");
LOG_SUCCESS("Success message");
```

Messages appear in the Event Log panel.

### File Structure Reference

```
src/
├── main.cpp                 # Application entry point
├── core/                    # Core simulation classes
│   ├── Agent.h             # Base agent class + Sector enum
│   ├── Person.h/cpp        # Citizen agents
│   ├── Firm.h/cpp          # Business agents
│   ├── Government.h/cpp    # Government singleton
│   ├── Economy.h/cpp       # Main simulation controller
│   └── Market.h/cpp        # Market clearing
├── models/                  # Economic models
│   ├── PhillipsCurve.h
│   ├── OkunLaw.h
│   ├── QuantityTheory.h
│   ├── UtilityFunction.h
│   └── StochasticShock.h
├── commands/                # Command system
│   ├── CommandParser.h/cpp
│   └── CommandExecutor.h/cpp
├── gui/                     # wxWidgets GUI
│   ├── MainFrame.h/cpp
│   ├── DashboardPanel.h/cpp
│   ├── EventLogPanel.h/cpp
│   ├── CommandPanel.h/cpp
│   └── HistoryPanel.h/cpp
└── utils/                   # Utilities
    ├── Config.h            # Configuration constants
    ├── Logger.h            # Logging system
    └── Statistics.h        # Statistical functions
```

---

## Quick Reference

### Command Syntax

```
command_name(param1=value1, param2=value2)
```

- String values: `param="value"` or `param=value`
- Numeric values: `param=123` or `param=0.5`
- Optional parameters can be omitted

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| F5 | Start simulation |
| F6 | Pause simulation |
| Ctrl+R | Reset simulation |
| Ctrl+N | New simulation |
| Ctrl+S | Export data |
| Up/Down | Navigate command history |
| Tab | Autocomplete command |
| Enter | Execute command |

---

*CppConomy - An Agent-Based Economic Simulator*
