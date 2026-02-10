#include "EconomicPropagation.h"
#include "Simulation.h"
#include "Logger.h"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <cmath>

using namespace std;

// ============================================================================
// EconomicPropagation.cpp - Implementation of the Economic Relationship Engine
//
// This file contains:
//   1. DependencyGraph - tracking variable relationships
//   2. EconomicPropagation core - event processing, rule matching
//   3. RegisterAllRules() - ALL economic relationships in one place
//
// The philosophy: Every economic relationship lives HERE.
// Entity classes (Farmer, Worker, etc.) just store state.
// This engine decides HOW state changes propagate across the system.
// ============================================================================

// ============================================================================
// DependencyGraph Implementation
// ============================================================================

void DependencyGraph::AddRelation(const string &trigger, const string &target)
{
    // Add forward edge: trigger -> target
    // Avoid duplicate entries
    auto &fwd = outgoing[trigger];
    if (find(fwd.begin(), fwd.end(), target) == fwd.end())
        fwd.push_back(target);

    // Add reverse edge: target <- trigger
    auto &rev = incoming[target];
    if (find(rev.begin(), rev.end(), trigger) == rev.end())
        rev.push_back(trigger);
}

vector<string> DependencyGraph::GetAffected(const string &trigger) const
{
    auto it = outgoing.find(trigger);
    if (it != outgoing.end())
        return it->second;
    return {};
}

vector<string> DependencyGraph::GetCauses(const string &variable) const
{
    auto it = incoming.find(variable);
    if (it != incoming.end())
        return it->second;
    return {};
}

void DependencyGraph::PrintChain(const string &variable, int depth) const
{
    // Print what causes this variable to change, recursively
    // Format:
    //   market.Rice.price is affected by:
    //     <- farmer.*.output_quantity
    //       <- farmer.*.labor_hired
    //     <- government.money_supply

    string indent = "";
    LOG_INFO("Dependency chain for: " + variable);

    // Internal recursive lambda to walk the graph
    function<void(const string &, int, string)> printCauses;
    printCauses = [&](const string &var, int remaining, string prefix)
    {
        if (remaining <= 0)
            return;

        auto it = incoming.find(var);
        if (it == incoming.end())
            return;

        for (const auto &cause : it->second)
        {
            LOG_INFO(prefix + "<- " + cause);
            printCauses(cause, remaining - 1, prefix + "  ");
        }
    };

    printCauses(variable, depth, "  ");
}

void DependencyGraph::ExportDot(const string &filename) const
{
    // Export the dependency graph as a Graphviz DOT file
    // Can be visualized with: dot -Tpng output.dot -o output.png
    ofstream file(filename);
    if (!file.is_open())
    {
        LOG_WARNING("Could not open file for DOT export: " + filename);
        return;
    }

    file << "digraph EconomicSystem {\n";
    file << "  rankdir=LR;\n";
    file << "  node [shape=box, style=filled, fillcolor=lightyellow];\n";
    file << "\n";

    for (const auto &[trigger, targets] : outgoing)
    {
        for (const auto &target : targets)
        {
            // Escape dots in identifiers for Graphviz
            string safeTrigger = "\"" + trigger + "\"";
            string safeTarget = "\"" + target + "\"";
            file << "  " << safeTrigger << " -> " << safeTarget << ";\n";
        }
    }

    file << "}\n";
    file.close();
    LOG_SUCCESS("Dependency graph exported to: " + filename);
}

// ============================================================================
// EconomicPropagation Implementation
// ============================================================================

EconomicPropagation::EconomicPropagation(Simulation *sim)
    : simulation(sim), currentTick(0)
{
    // Register all economic rules on construction
    RegisterAllRules();
    LOG_INFO("Economic Propagation Engine initialized with " +
             to_string(rules.size()) + " rules");
}

// ============================================================================
// Rule Registration
// ============================================================================

void EconomicPropagation::RegisterRule(const PropagationRule &rule)
{
    int ruleIdx = static_cast<int>(rules.size());
    rules.push_back(rule);

    // Index the rule by its trigger pattern for fast lookup
    ruleIndex[rule.trigger].push_back(ruleIdx);

    // Parse targets (comma-separated) and add to dependency graph
    // Example: "market.*.price,market.*.quantity" -> two edges
    string targets = rule.targets;
    size_t pos = 0;
    while ((pos = targets.find(',')) != string::npos)
    {
        string target = targets.substr(0, pos);
        dependencyGraph.AddRelation(rule.trigger, target);
        targets.erase(0, pos + 1);
    }
    if (!targets.empty())
    {
        dependencyGraph.AddRelation(rule.trigger, targets);
    }
}

// ============================================================================
// RegisterAllRules - THE MASTER LIST OF ALL ECONOMIC RELATIONSHIPS
//
// This is the single source of truth for how variables connect.
// Adding a new relationship? Add it HERE.
//
// Each rule has:
//   trigger:     What variable change activates this rule
//   targets:     What variables are affected
//   apply:       Lambda that calculates the effect
//   strength:    0-1 scaling factor
//   tier:        1=immediate, 2=agent decision, 3=delayed macro
//   delay:       Ticks before effect manifests
//   description: Human-readable explanation
// ============================================================================
void EconomicPropagation::RegisterAllRules()
{
    // ==================================================================
    // TIER 1: IMMEDIATE EFFECTS (same tick)
    // These happen instantly when their trigger fires.
    // Mostly market-clearing and arithmetic consequences.
    // ==================================================================

    // ------------------------------------------------------------------
    // Rule 1: Supply Change -> Price Adjustment
    //
    // Economics: When supply increases, equilibrium price falls.
    //   More supply at every price → rightward shift of supply curve
    //   → new intersection with demand curve is at lower price.
    //
    // Example: Farmer harvests bumper crop → supply floods market → price drops
    // ------------------------------------------------------------------
    RegisterRule({"farmer.*.output_quantity", // trigger: any farmer's output changes
                  "market.*.price",           // target: market price is affected
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      // Extract the farmer name from the event variable
                      // event.variable = "farmer.Shafin.output_quantity"
                      auto parts = EconomicPropagation::SplitVariable(event.variable);
                      if (parts.size() < 2)
                          return 0.0;

                      string farmerName = parts[1];
                      Farmer *farmer = sim->FindFarmer(farmerName);
                      if (!farmer)
                          return 0.0;

                      // Look up the market for this farmer's crop
                      Market *market = sim->FindMarket(farmer->GetCrop());
                      if (!market)
                          return 0.0;

                      // Recalculate equilibrium with updated supply
                      // The market already has the new supply from UpdateMarketFromHarvest
                      market->FindEquilibrium();

                      // Return the new equilibrium price
                      return market->GetEquilibriumPrice();
                  },
                  0.9, // strength: strong direct relationship
                  1,   // tier 1: immediate market clearing
                  0,   // delay: happens same tick
                  "Supply increase shifts supply curve right, lowering equilibrium price (Law of Supply)"});

    // ------------------------------------------------------------------
    // Rule 2: Price Change -> Quantity Demanded Update
    //
    // Economics: Law of Demand - price up → demand down, price down → demand up
    //   Higher price → consumers buy less (move along demand curve)
    //   Lower price → consumers buy more
    //
    // Example: Rice price drops after harvest → consumers demand more rice
    // ------------------------------------------------------------------
    RegisterRule({"market.*.price",             // trigger: market price changes
                  "market.*.quantity_demanded", // target: demand at that price
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      // Extract market name from event variable
                      auto parts = EconomicPropagation::SplitVariable(event.variable);
                      if (parts.size() < 2)
                          return 0.0;

                      string marketName = parts[1];
                      Market *market = sim->FindMarket(marketName);
                      if (!market)
                          return 0.0;

                      // Use the demand function to calculate new quantity demanded
                      // Qd = a - b * P (linear demand curve)
                      double newPrice = event.newValue;
                      double newDemand = market->GetDemandFunction().Calculate(newPrice);

                      market->SetDemand(max(0.0, newDemand));
                      return newDemand;
                  },
                  1.0, // strength: direct mathematical relationship
                  1,   // tier 1: immediate
                  0,   // delay: same tick
                  "Law of Demand: price increase reduces quantity demanded (movement along demand curve)"});

    // ------------------------------------------------------------------
    // Rule 3: Price Change -> Farmer Revenue Recalculation
    //
    // Economics: Revenue = Price × Quantity
    //   When market price changes, farmer revenue changes proportionally
    //   (assuming quantity sold stays constant in the short run)
    //
    // Example: Corn price drops from $50 to $40 → farmer revenue drops by 20%
    // ------------------------------------------------------------------
    RegisterRule({"market.*.price",   // trigger: market price changes
                  "farmer.*.revenue", // target: farmer revenue
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      // Extract market name to find which crop's price changed
                      auto parts = EconomicPropagation::SplitVariable(event.variable);
                      if (parts.size() < 2)
                          return 0.0;

                      string marketName = parts[1];
                      double newPrice = event.newValue;

                      // Update revenue for ALL farmers who produce this crop
                      // Revenue = Price × Quantity
                      double totalRevenue = 0.0;
                      for (const auto &farmer : sim->GetFarmers())
                      {
                          if (farmer->GetCrop() == marketName)
                          {
                              farmer->CalculateProfit(newPrice);
                              totalRevenue += farmer->GetRevenue();
                          }
                      }
                      return totalRevenue;
                  },
                  1.0, // strength: direct arithmetic
                  1,   // tier 1: immediate recalculation
                  0,   // delay: same tick
                  "Revenue = Price × Quantity: price change directly recalculates farmer revenue"});

    // ------------------------------------------------------------------
    // Rule 4: Price Change -> Owner Revenue & Profit Recalculation
    //
    // Economics: Same as farmer - revenue depends on product price.
    //   Owner firms that produce a good see revenue change with market price.
    //
    // Example: Cloth price rises → Soron's revenue increases → profit up
    // ------------------------------------------------------------------
    RegisterRule({"market.*.price",  // trigger: market price changes
                  "owner.*.revenue", // target: owner revenue
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      auto parts = EconomicPropagation::SplitVariable(event.variable);
                      if (parts.size() < 2)
                          return 0.0;

                      string marketName = parts[1];
                      double newPrice = event.newValue;

                      // Update all owners who produce goods sold in this market
                      double totalRevenue = 0.0;
                      for (const auto &owner : sim->GetOwners())
                      {
                          if (owner->GetProductType() == marketName)
                          {
                              owner->SetPrice(newPrice);
                              owner->CalculateRevenue();
                              owner->CalculateProfit();
                              totalRevenue += owner->GetRevenue();
                          }
                      }
                      return totalRevenue;
                  },
                  1.0, // strength: direct arithmetic
                  1,   // tier 1: immediate
                  0,   // delay: same tick
                  "Revenue = Price × Quantity: market price change recalculates owner revenue and profit"});

    // ==================================================================
    // TIER 2: AGENT DECISION EFFECTS (respond to new information)
    // These are behavioral responses where agents react to changes.
    // Slower than market clearing but faster than macro effects.
    // ==================================================================

    // ------------------------------------------------------------------
    // Rule 5: Profit Change -> Farmer Adjusts Labor (Hiring/Firing)
    //
    // Economics: Profit maximization - firms hire until MC = MR
    //   Negative profit → cut costs → fire workers
    //   Positive profit → expand → hire workers
    //   This creates the link: market → prices → profit → employment
    //
    // Example: Corn price drops → farmer profit falls → farmer fires a worker
    // ------------------------------------------------------------------
    RegisterRule({"farmer.*.revenue",     // trigger: farmer revenue changes (proxy for profit)
                  "farmer.*.labor_hired", // target: labor decisions
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      // Find the relevant farmer
                      auto parts = EconomicPropagation::SplitVariable(event.variable);
                      if (parts.size() < 2)
                          return 0.0;

                      // Check all farmers' profits and adjust labor accordingly
                      for (const auto &farmer : sim->GetFarmers())
                      {
                          double profit = farmer->GetProfit();
                          int currentLabor = farmer->GetLaborHired();

                          if (profit < -50.0 && currentLabor > 0)
                          {
                              // Losing money significantly: fire a worker to cut costs
                              // This demonstrates the microeconomic link between
                              // market prices → firm profitability → employment decisions
                              farmer->Fire(1);
                              LOG_INFO("Farmer " + farmer->GetName() + " fired 1 worker (profit: " +
                                       to_string(profit) + ")");
                          }
                          else if (profit > 200.0)
                          {
                              // Strong profits: hire a worker to expand production
                              // Marginal product of labor is positive when profitable
                              farmer->Hire(1);
                              LOG_INFO("Farmer " + farmer->GetName() + " hired 1 worker (profit: " +
                                       to_string(profit) + ")");
                          }
                      }

                      // Return total farmer labor for tracking
                      int totalLabor = 0;
                      for (const auto &f : sim->GetFarmers())
                          totalLabor += f->GetLaborHired();
                      return static_cast<double>(totalLabor);
                  },
                  0.6, // strength: moderate - firms don't react instantly to every fluctuation
                  2,   // tier 2: agent behavioral decision
                  0,   // delay: same tick (decision is made after seeing new numbers)
                  "Profit-driven hiring: negative profit triggers firing, high profit triggers hiring"});

    // ------------------------------------------------------------------
    // Rule 6: Owner Revenue Change -> Owner Production Decision
    //
    // Economics: Firms adjust output based on profitability
    //   The MR = MC rule says: produce where marginal revenue = marginal cost
    //   If revenue drops, firms may shut down (P < AVC) or cut production
    //
    // Example: Cloth price drops → Soron's revenue falls → reduces production
    // ------------------------------------------------------------------
    RegisterRule({"owner.*.revenue",    // trigger: owner revenue changes
                  "owner.*.production", // target: production decision
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      double totalProduction = 0.0;
                      for (const auto &owner : sim->GetOwners())
                      {
                          // Check shutdown condition: if price < AVC, stop producing
                          // This is the fundamental short-run shutdown decision
                          owner->CheckShutdownCondition(owner->GetPrice());

                          if (owner->IsOperating())
                          {
                              // If still operating, produce at current capacity
                              owner->Produce();
                          }
                          totalProduction += owner->GetProduction();
                      }
                      return totalProduction;
                  },
                  0.7, // strength: significant behavioral response
                  2,   // tier 2: agent decision
                  0,   // delay: same tick
                  "Shutdown decision: owners check P vs AVC and adjust production (MR=MC optimization)"});

    // ------------------------------------------------------------------
    // Rule 7: Tax Rate Change -> Consumer Wallet (Disposable Income)
    //
    // Economics: Income tax reduces disposable income
    //   Higher tax rate → less money to spend → demand falls
    //   This is the Keynesian fiscal policy channel:
    //   Tax ↑ → Disposable Income ↓ → Consumption ↓ → GDP ↓
    //
    // Example: Government raises income tax 10% → 15%
    //   → workers' take-home pay drops → they buy less → demand falls
    // ------------------------------------------------------------------
    RegisterRule({"government.income_tax_rate", // trigger: tax rate changes
                  "consumer.*.wallet",          // target: all consumers' wallets
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      double taxRate = event.newValue;
                      double oldTaxRate = event.oldValue;

                      // Calculate how much MORE (or less) tax is being taken
                      // relative to the previous rate
                      double taxChange = taxRate - oldTaxRate;

                      // Apply the tax effect to all workers' wallets
                      // Tax increase → wallet decreases, tax cut → wallet increases
                      double totalWalletChange = 0.0;
                      for (const auto &worker : sim->GetWorkers())
                      {
                          double income = worker->GetMonthlyIncome();
                          double additionalTax = income * taxChange;
                          // PayTax reduces the wallet by the amount
                          if (additionalTax > 0)
                              worker->PayTax(additionalTax);
                          totalWalletChange += worker->GetWallet();
                      }

                      return totalWalletChange;
                  },
                  1.0, // strength: direct fiscal impact
                  2,   // tier 2: behavioral (people adjust spending after tax change)
                  0,   // delay: same tick
                  "Fiscal policy: income tax change reduces disposable income (Keynesian multiplier channel)"});

    // ------------------------------------------------------------------
    // Rule 8: Minimum Wage Change -> Worker Employment Status
    //
    // Economics: Minimum wage is a price floor in the labor market
    //   If min wage > market wage: unemployment increases (surplus labor)
    //   Workers earning below new min wage get fired unless employer matches
    //
    // This demonstrates the classic price floor surplus:
    //   Floor above equilibrium → quantity supplied > quantity demanded → surplus
    //   In labor market: labor surplus = unemployment
    // ------------------------------------------------------------------
    RegisterRule({"government.minimuwage", // trigger: minimum wage changes
                  "worker.*.employment",   // target: worker employment status
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      double newMinWage = event.newValue;
                      int totalEmployed = 0;

                      // Each worker checks if their current wage meets the new minimum
                      for (const auto &worker : sim->GetWorkers())
                      {
                          worker->RespondToMinimumWage(newMinWage);
                          if (worker->IsEmployed())
                              totalEmployed++;
                      }

                      return static_cast<double>(totalEmployed);
                  },
                  1.0, // strength: direct policy impact
                  2,   // tier 2: agent response to regulation
                  0,   // delay: same tick
                  "Price floor effect: minimum wage above market wage causes unemployment (labor surplus)"});

    // ------------------------------------------------------------------
    // Rule 9: Wage Change -> Worker Labor Supply Decision
    //
    // Economics: Labor-leisure tradeoff
    //   Higher wage → substitution effect (leisure more expensive, work more)
    //   Higher wage → income effect (richer, can afford more leisure)
    //   Net effect usually: higher wage → more hours (up to a point)
    //   This creates the backward-bending labor supply curve
    //
    // Example: Wage rises $15 → $20 → worker offers 44 hrs instead of 40
    // ------------------------------------------------------------------
    RegisterRule({"worker.*.wage",           // trigger: worker's wage changes
                  "worker.*.hours_supplied", // target: hours willing to work
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      // Update all workers' labor supply decisions
                      double totalHours = 0.0;
                      for (const auto &worker : sim->GetWorkers())
                      {
                          if (worker->IsEmployed())
                          {
                              worker->UpdateLaborSupply(worker->GetCurrentWage());
                          }
                          totalHours += worker->GetHoursWillingToWork();
                      }
                      return totalHours;
                  },
                  0.5, // strength: moderate behavioral response
                  2,   // tier 2: agent decision
                  0,   // delay: same tick
                  "Labor-leisure tradeoff: wage change alters hours supplied (backward-bending supply curve)"});

    // ==================================================================
    // TIER 3: DELAYED MACRO EFFECTS (slow systemic changes)
    // These take multiple ticks to manifest, representing the
    // real-world lag in macroeconomic adjustments.
    // ==================================================================

    // ------------------------------------------------------------------
    // Rule 10: Money Supply Change -> Inflation
    //
    // Economics: Quantity Theory of Money (MV = PQ)
    //   More money chasing same goods → prices rise
    //   Money Supply ↑ → Price Level ↑ (inflation)
    //   This is a DELAYED effect - printing money doesn't cause
    //   inflation instantly, it takes time to work through the economy.
    //
    // Formula: ΔP/P ≈ ΔM/M (proportional in the long run)
    //
    // Example: Government doubles money supply → inflation rises ~100% over time
    // ------------------------------------------------------------------
    RegisterRule({"government.money_supply", // trigger: money supply changes
                  "government.inflation",    // target: inflation rate
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      // Quantity Theory: MV = PQ
                      // If V and Q are constant, then ΔP = ΔM * V / Q
                      // Simplified: inflation responds proportionally to money supply growth

                      double oldM = event.oldValue;
                      double newM = event.newValue;

                      if (oldM <= 0)
                          return 0.0;

                      // Money supply growth rate
                      double moneyGrowthRate = (newM - oldM) / oldM;

                      // Inflation is a fraction of money growth (not 1:1 immediately)
                      // In reality, velocity and output absorb some of the change
                      double inflationEffect = moneyGrowthRate * 0.6; // 60% pass-through

                      Government *gov = sim->GetGovernment();
                      if (gov)
                      {
                          double currentCPI = gov->GetCPI();
                          double newCPI = currentCPI * (1.0 + inflationEffect);
                          gov->UpdateCPI(newCPI);
                          gov->CalculateInflation(currentCPI, newCPI);
                      }

                      return inflationEffect;
                  },
                  0.6, // strength: partial pass-through (some absorbed by output growth)
                  3,   // tier 3: delayed macro effect
                  1,   // delay: 1 tick lag (inflation doesn't appear instantly)
                  "Quantity Theory of Money: money supply growth causes delayed inflation (MV=PQ)"});

    // ------------------------------------------------------------------
    // Rule 11: Interest Rate Change -> Owner Investment Decision
    //
    // Economics: Interest rate is the cost of capital
    //   Lower interest → cheaper to borrow → more investment → expand
    //   Higher interest → expensive to borrow → less investment → shrink
    //   This is the monetary policy transmission mechanism:
    //   Interest Rate ↓ → Investment ↑ → Output ↑ → GDP ↑
    //
    // Example: Central bank lowers rate 5% → 3% → firms borrow and expand
    // ------------------------------------------------------------------
    RegisterRule({"government.interest_rate", // trigger: interest rate changes
                  "owner.*.capital",          // target: owner investment in capital
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      double oldRate = event.oldValue;
                      double newRate = event.newValue;
                      double rateChange = oldRate - newRate; // Positive if rate decreased (good for investment)

                      double totalCapital = 0.0;
                      for (const auto &owner : sim->GetOwners())
                      {
                          if (rateChange > 0.01) // Rate decreased meaningfully
                          {
                              // Lower rates encourage expansion
                              // Simplified: capital increases by a fraction proportional to rate cut
                              // In reality this would involve NPV calculations for investment projects
                              LOG_INFO("Owner " + owner->GetName() +
                                       " considering expansion (rate cut: " +
                                       to_string(rateChange * 100) + "%)");
                          }
                          else if (rateChange < -0.01) // Rate increased
                          {
                              // Higher rates discourage investment
                              LOG_INFO("Owner " + owner->GetName() +
                                       " delaying investment (rate hike: " +
                                       to_string(-rateChange * 100) + "%)");
                          }
                          totalCapital += owner->GetCapital();
                      }
                      return totalCapital;
                  },
                  0.4, // strength: moderate - investment decisions are cautious
                  3,   // tier 3: delayed (firms don't invest overnight)
                  1,   // delay: 1 tick (takes time to arrange financing)
                  "Monetary transmission: lower interest rates stimulate investment (IS curve shift)"});

    // ------------------------------------------------------------------
    // Rule 12: Unemployment Change -> Government Budget Pressure
    //
    // Economics: Automatic stabilizers
    //   Unemployment ↑ → tax revenue falls (fewer people working)
    //   → government spending on safety nets increases
    //   → budget deficit widens
    //   This is the automatic stabilizer mechanism that cushions recessions.
    //
    // Example: 5% unemployment → 10% → tax revenue drops, deficit grows
    // ------------------------------------------------------------------
    RegisterRule({"stats.unemployment", // trigger: unemployment rate changes
                  "government.budget",  // target: government budget
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      Government *gov = sim->GetGovernment();
                      if (!gov)
                          return 0.0;

                      // Recalculate tax collection with fewer employed people
                      double totalIncome = 0.0;
                      double totalProfits = 0.0;

                      for (const auto &worker : sim->GetWorkers())
                      {
                          if (worker->IsEmployed())
                              totalIncome += worker->GetMonthlyIncome();
                      }
                      for (const auto &farmer : sim->GetFarmers())
                      {
                          totalProfits += max(0.0, farmer->GetProfit());
                      }
                      for (const auto &owner : sim->GetOwners())
                      {
                          totalProfits += max(0.0, owner->GetProfit());
                      }

                      gov->CollectTaxes(totalIncome, totalProfits);
                      gov->UpdateBudget();

                      return -gov->GetBudgetDeficit(); // Negative = deficit
                  },
                  0.8, // strength: strong automatic stabilizer
                  3,   // tier 3: delayed macro effect
                  0,   // delay: budget adjusts with some lag
                  "Automatic stabilizers: unemployment reduces tax revenue, widening budget deficit"});

    // ------------------------------------------------------------------
    // Rule 13: Government Spending Change -> Aggregate Demand (GDP)
    //
    // Economics: Keynesian multiplier
    //   G ↑ → AD ↑ → GDP ↑ (with multiplier effect)
    //   Multiplier = 1 / (1 - MPC) where MPC = marginal propensity to consume
    //   If MPC = 0.8, multiplier = 5: $1 of government spending creates $5 of GDP
    //
    // Example: Government increases spending $1000 → GDP impact = $1000 × multiplier
    // ------------------------------------------------------------------
    RegisterRule({"government.spending", // trigger: government spending changes
                  "stats.gdp",           // target: GDP
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      // GDP = C + I + G + (X - M)
                      // When G changes, GDP changes by G × multiplier
                      double oldG = event.oldValue;
                      double newG = event.newValue;
                      double deltaG = newG - oldG;

                      // Simple Keynesian multiplier (assume MPC = 0.75 → mult = 4)
                      double mpc = 0.75;
                      double multiplier = 1.0 / (1.0 - mpc);
                      double gdpImpact = deltaG * multiplier;

                      LOG_INFO("Keynesian multiplier: spending change of " +
                               to_string(deltaG) + " → GDP impact of " +
                               to_string(gdpImpact) + " (multiplier: " +
                               to_string(multiplier) + ")");

                      // Refresh simulation stats to capture the new GDP
                      sim->RefreshStats();

                      return sim->GetStats().gdp;
                  },
                  0.7, // strength: significant but not instant
                  3,   // tier 3: delayed macro effect
                  1,   // delay: spending takes time to circulate through economy
                  "Keynesian multiplier: government spending increases aggregate demand with multiplied effect"});

    // ------------------------------------------------------------------
    // Rule 14: Subsidy -> Market Supply Shift
    //
    // Economics: Subsidies reduce production costs
    //   Subsidy → cost per unit falls → supply curve shifts right
    //   → equilibrium price falls, quantity rises
    //   This benefits consumers (lower price) at taxpayer expense
    //
    // Example: Government subsidizes rice farmers $5/unit
    //   → rice supply curve shifts right → rice price drops
    // ------------------------------------------------------------------
    RegisterRule({"government.subsidy", // trigger: subsidy granted
                  "market.*.supply",    // target: market supply
                  [](Simulation *sim, const EconomicEvent &event, const string &target) -> double
                  {
                      // Subsidies shift the supply curve right
                      // This is captured through the market's tax/subsidy mechanism
                      for (auto &[name, market] : sim->GetMarkets())
                      {
                          if (market)
                          {
                              market->ApplyTaxAndSubsidy();
                              market->FindEquilibrium();
                          }
                      }
                      return 0.0; // Effect is captured in market state
                  },
                  0.8, // strength: direct policy effect
                  1,   // tier 1: immediate market impact
                  0,   // delay: supply curve shifts immediately when subsidy announced
                  "Subsidy shifts supply curve right: producers receive more, price falls for consumers"});

    LOG_SUCCESS("Registered " + to_string(rules.size()) +
                " economic propagation rules");
}

// ============================================================================
// Event Queueing
// ============================================================================

void EconomicPropagation::QueueEvent(const EconomicEvent &event)
{
    eventQueue.push(event);
    eventHistory.push_back(event);
}

void EconomicPropagation::EmitChange(const string &source,
                                     const string &variable,
                                     double oldValue, double newValue,
                                     const string &description)
{
    // Don't emit events for zero changes (prevents infinite loops)
    if (abs(newValue - oldValue) < 1e-6)
        return;

    EconomicEvent event;
    event.source = source;
    event.variable = variable;
    event.oldValue = oldValue;
    event.newValue = newValue;

    // Calculate change ratio safely (avoid division by zero)
    event.changeRatio = (oldValue != 0.0) ? (newValue / oldValue) : 1.0;

    event.propagationLevel = 0; // Original event, not cascaded
    event.tick = currentTick;
    event.description = description;

    QueueEvent(event);

    LOG_INFO("Event emitted: " + event.ToString());
}

// ============================================================================
// Tier Processing
// ============================================================================

void EconomicPropagation::ProcessTier1()
{
    ProcessEventsForTier(1);
}

void EconomicPropagation::ProcessTier2()
{
    ProcessEventsForTier(2);
}

void EconomicPropagation::ProcessTier3()
{
    // First, process any delayed events that are now due
    ProcessDelayedEvents();

    // Then process tier 3 events in the queue
    ProcessEventsForTier(3);
}

void EconomicPropagation::ProcessDelayedEvents()
{
    // Check if there are any events scheduled for the current tick
    auto it = delayedEvents.find(currentTick);
    if (it != delayedEvents.end())
    {
        // Re-queue all delayed events that are now due
        for (auto &event : it->second)
        {
            LOG_INFO("Delayed event now due: " + event.ToString());
            eventQueue.push(event);
        }
        // Remove processed delayed events
        delayedEvents.erase(it);
    }
}

void EconomicPropagation::ProcessEventsForTier(int tier)
{
    // Process all events currently in the queue that match this tier
    // We snapshot the current queue size to avoid processing events
    // that get added during this processing pass
    int eventsToProcess = static_cast<int>(eventQueue.size());
    int processed = 0;

    // Temporary queue for events that don't match this tier
    queue<EconomicEvent> deferredEvents;

    while (!eventQueue.empty() && processed < eventsToProcess)
    {
        EconomicEvent event = eventQueue.front();
        eventQueue.pop();
        processed++;

        bool anyRuleMatched = false;

        // Try each rule to see if it matches this event
        for (const auto &rule : rules)
        {
            // Only process rules for this tier
            if (rule.tier != tier)
                continue;

            // Check if the event's variable matches the rule's trigger pattern
            if (!MatchesTrigger(event.variable, rule.trigger))
                continue;

            anyRuleMatched = true;

            // Check if this rule has a delay
            if (rule.delay > 0 && event.propagationLevel == 0)
            {
                // Schedule for future processing
                EconomicEvent delayedEvent = event;
                delayedEvent.description += " (delayed " + to_string(rule.delay) + " ticks)";
                delayedEvents[currentTick + rule.delay].push_back(delayedEvent);
                LOG_INFO("Scheduled delayed event for tick " +
                         to_string(currentTick + rule.delay) +
                         ": " + event.variable);
                continue; // Don't apply now, apply later
            }

            // Apply the rule (this may generate cascading events)
            ApplyRule(rule, event);
        }

        // If no rule matched, the event was informational only
        if (!anyRuleMatched)
        {
            // Put back into deferred queue for other tiers
            deferredEvents.push(event);
        }
    }

    // Put deferred events back
    while (!deferredEvents.empty())
    {
        eventQueue.push(deferredEvents.front());
        deferredEvents.pop();
    }
}

void EconomicPropagation::ApplyRule(const PropagationRule &rule, const EconomicEvent &event)
{
    // Safety: prevent infinite cascading
    if (event.propagationLevel >= MAX_PROPAGATION_DEPTH)
    {
        LOG_WARNING("Max propagation depth reached for: " + event.variable +
                    " (depth=" + to_string(event.propagationLevel) + ")");
        return;
    }

    // Apply the rule's function to calculate the effect
    double result = rule.apply(simulation, event, rule.targets);

    // Scale by strength
    // (The rule function returns the absolute new value, strength modulates the
    //  change magnitude for cascaded events)

    // Log the rule application for debugging
    ostringstream logMsg;
    logMsg << "Rule applied [Tier " << rule.tier << "]: "
           << event.variable << " -> " << rule.targets
           << " (result: " << fixed << setprecision(2) << result
           << ", strength: " << rule.strength << ")";
    LOG_INFO(logMsg.str());

    // If this rule produced a meaningful result, create cascading events
    // so downstream rules can also fire
    if (result != 0.0)
    {
        // Parse targets and create cascading events for each
        string targets = rule.targets;
        size_t pos = 0;
        while ((pos = targets.find(',')) != string::npos)
        {
            string target = targets.substr(0, pos);
            EconomicEvent cascadeEvent;
            cascadeEvent.source = "propagation:" + rule.description;
            cascadeEvent.variable = target;
            cascadeEvent.oldValue = event.newValue; // Approximate
            cascadeEvent.newValue = result;
            cascadeEvent.changeRatio = (event.newValue != 0.0) ? (result / event.newValue) : 1.0;
            cascadeEvent.propagationLevel = event.propagationLevel + 1;
            cascadeEvent.tick = currentTick;
            cascadeEvent.description = "Cascaded from: " + event.variable;

            QueueEvent(cascadeEvent);
            targets.erase(0, pos + 1);
        }
        if (!targets.empty())
        {
            EconomicEvent cascadeEvent;
            cascadeEvent.source = "propagation:" + rule.description;
            cascadeEvent.variable = targets;
            cascadeEvent.oldValue = event.newValue;
            cascadeEvent.newValue = result;
            cascadeEvent.changeRatio = (event.newValue != 0.0) ? (result / event.newValue) : 1.0;
            cascadeEvent.propagationLevel = event.propagationLevel + 1;
            cascadeEvent.tick = currentTick;
            cascadeEvent.description = "Cascaded from: " + event.variable;

            QueueEvent(cascadeEvent);
        }
    }
}

// ============================================================================
// Pattern Matching
// ============================================================================

bool EconomicPropagation::MatchesTrigger(const string &eventVariable,
                                         const string &triggerPattern) const
{
    // Split both into dot-separated parts
    // Event:   "farmer.Shafin.output_quantity"
    // Pattern: "farmer.*.output_quantity"
    //
    // Rules:
    //   - "farmer.*.output_quantity" matches any farmer's output
    //   - "market.*.price" matches any market's price
    //   - "government.income_tax_rate" matches exactly
    //   - "*" alone matches everything

    auto eventParts = SplitVariable(eventVariable);
    auto patternParts = SplitVariable(triggerPattern);

    // Different number of parts => no match
    // (unless pattern has fewer parts, allowing prefix matching)
    if (patternParts.size() > eventParts.size())
        return false;

    // Compare part by part
    for (size_t i = 0; i < patternParts.size(); ++i)
    {
        if (patternParts[i] == "*")
            continue; // Wildcard matches anything

        if (patternParts[i] != eventParts[i])
            return false; // Mismatch
    }

    return true;
}

vector<string> EconomicPropagation::SplitVariable(const string &variable)
{
    vector<string> parts;
    istringstream stream(variable);
    string part;

    while (getline(stream, part, '.'))
    {
        if (!part.empty())
            parts.push_back(part);
    }

    return parts;
}

// ============================================================================
// Queries
// ============================================================================

vector<string> EconomicPropagation::GetAffectedVariables(const string &trigger) const
{
    return dependencyGraph.GetAffected(trigger);
}

vector<string> EconomicPropagation::GetVariableCauses(const string &variable) const
{
    return dependencyGraph.GetCauses(variable);
}

vector<EconomicEvent> EconomicPropagation::GetHistoryForVariable(const string &variable) const
{
    vector<EconomicEvent> filtered;
    for (const auto &event : eventHistory)
    {
        if (event.variable == variable)
            filtered.push_back(event);
    }
    return filtered;
}

// ============================================================================
// Visualization & Debugging
// ============================================================================

void EconomicPropagation::PrintDependencyChain(const string &variable) const
{
    dependencyGraph.PrintChain(variable);
}

string EconomicPropagation::GetEventLog(int forTick) const
{
    // If forTick is -1, show events from the current tick
    int targetTick = (forTick < 0) ? currentTick : forTick;

    ostringstream ss;
    ss << "=== Event Log for Tick " << targetTick << " ===\n";

    bool found = false;
    for (const auto &event : eventHistory)
    {
        if (event.tick == targetTick)
        {
            ss << event.ToString() << "\n";
            found = true;
        }
    }

    if (!found)
        ss << "(No events recorded for this tick)\n";

    ss << "=== End Event Log ===\n";
    return ss.str();
}

void EconomicPropagation::ExportDependencyGraph(const string &filename) const
{
    dependencyGraph.ExportDot(filename);
}
