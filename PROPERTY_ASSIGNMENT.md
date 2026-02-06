# Property Assignment Syntax

## Overview

Instead of using setter commands like `set_income`, `set_price`, etc., we now use direct property assignment syntax similar to standard programming languages.

## Syntax

```
entity(filter).property = value
```

Where:
- `entity` is the type: `worker`, `farmer`, `owner`, `market`, or `system`
- `filter` (optional) specifies which instance: `name='EntityName'`
- `property` is the field to modify
- `value` is the new value

## Examples

### Worker Properties

```
worker(name='Alice').wage = 1500
worker(name='Alice').income = 2000
worker(name='Alice').skill = 0.8
worker(name='Alice').min_wage = 12
```

### Farmer Properties

```
farmer(name='Bob').fertilizer = 50
```

### Owner Properties

```
owner(name='Charlie').price = 250
```

### Market Properties

```
market(name='rice').price = 150
market(name='rice').demand = 1000
market(name='rice').supply = 800
market(name='rice').tax = 0.05
market(name='rice').subsidy = 0.02
```

### System Properties (Government)

```
system.income_tax_rate = 0.15
system.corporate_tax_rate = 0.20
system.minimum_wage = 15
system.government_spending = 50000
system.money_supply = 100000000
system.interest_rate = 0.05
```

## Variable Change Tracking

When you assign a property, the system:
1. Records the old value
2. Sets the new value
3. Logs the change with related effects
4. Refreshes all statistics
5. Updates the dashboard

Example output:
```
Worker Alice.wage = 1500
Change: worker.Alice.wage increased from 1000 to 1500
Relation: worker.Alice.wage increased -> worker.Alice.wallet expected to increase
```

## Query Properties (Read-only)

You can still query properties without assignment:
```
worker(name='Alice').wage          # Returns current wage
market(name='rice').price          # Returns current price
system.gdp                         # Returns GDP
```

## Available Properties by Entity

### Worker
- `wage` - Current wage rate
- `income` - Monthly income
- `skill` - Skill level (0.0-1.0)
- `min_wage` - Minimum acceptable wage

### Farmer
- `fertilizer` - Fertilizer units

### Owner
- `price` - Product price

### Market
- `price` - Market price
- `demand` - Quantity demanded
- `supply` - Quantity supplied
- `tax` - Tax rate
- `subsidy` - Subsidy rate

### System
- `income_tax_rate` or `income_tax`
- `corporate_tax_rate` or `corporate_tax`
- `minimum_wage` or `min_wage`
- `government_spending` or `spending`
- `money_supply` or `money`
- `interest_rate` or `interest`

## Implementation Notes

- All assignments trigger variable change tracking
- Related variables are automatically logged based on economic relationships
- Statistics are refreshed after each assignment
- Invalid property names return an error message
