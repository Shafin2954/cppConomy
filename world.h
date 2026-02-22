#pragma once
#include <vector>
#include <string>
#include <cstdlib>
#include "style.h"
#include "consumer.h"
#include "laborer.h"
#include "farmer.h"
#include "firm.h"
#include "market.h"

using namespace styledTerminal;
using namespace Box;

class world
{
public:
    enum class change
    {
        Increases,
        Decreases,
        Changes
    };

    struct stats
    {
        double gdp = 0.0;
        double unemployment = 0.0;
        int employed = 0;
        int population = 0;
        double moneySupply = 0.0;
        int firms = 0;
    };

    stats currentStats;
    int dayCount = 0;

    std::vector<consumer> consumers;
    std::vector<laborer> laborers;
    std::vector<farmer> farmers;
    std::vector<firm> firms;
    std::vector<market> markets;

    consumer *selected_consumer = nullptr;
    laborer *selected_laborer = nullptr;
    farmer *selected_farmer = nullptr;
    market *selected_market = nullptr;
    firm *selected_firm = nullptr;

    world() {}

    stats getStats()
    {
        currentStats.population = getPopulation();
        currentStats.firms = firms.size();
        return currentStats;
    }

    // ── INITIALIZATION ────────────────────────────────────────────────────
    void innitialize()
    {
        srand(42); // reproducible but varied

        // ── MARKETS ───────────────────────────────────────────────────────
        std::vector<product *> prods = {&rice, &cloth, &computer, &phone, &potato, &banana, &corn, &jute};
        for (auto *p : prods)
            markets.emplace_back(p);
        selected_market = &markets[0];

        // ── CONSUMERS (urban middle/working class) ─────────────────────────
        // ID block 11–19
        // BD urban wage reference: garment ~420 Tk/day, office ~700, professional ~950+
        addConsumerFull(11, "Rahim", 28, 18000, 420);  // garment worker
        addConsumerFull(12, "Priom", 26, 55000, 700);  // office staff
        addConsumerFull(13, "Rohan", 20, 9000, 280);   // student/part-time
        addConsumerFull(14, "Rudaba", 24, 35000, 580); // junior professional
        addConsumerFull(15, "Atef", 35, 90000, 950);   // senior professional
        addConsumerFull(16, "Siam", 22, 6500, 310);    // entry-level
        addConsumerFull(17, "Nahid", 30, 42000, 640);  // mid-level
        addConsumerFull(18, "Liam", 27, 28000, 510);   // skilled worker
        addConsumerFull(19, "Somio", 32, 62000, 780);  // manager

        // ── LABORERS (factory / daily wage workers) ───────────────────────
        // ID block 31–38
        addLaborerFull(31, "Kowshik", 25, 0.70, 400, 11000, 480);
        addLaborerFull(32, "Cauchy", 22, 0.50, 320, 7000, 370);
        addLaborerFull(33, "Mahin", 28, 0.80, 500, 13000, 530);
        addLaborerFull(34, "Shad", 23, 0.45, 300, 5500, 340);
        addLaborerFull(35, "Mahir", 30, 0.65, 420, 10000, 460);
        addLaborerFull(36, "Labib", 26, 0.55, 350, 8000, 400);
        addLaborerFull(37, "Jubair", 24, 0.40, 290, 4500, 320);
        addLaborerFull(38, "Nabil", 29, 0.75, 450, 12000, 500);

        // ── FARMERS (diverse scale and crops) ─────────────────────────────
        // ID block 21–27
        // addFarmerFull(id, name, age, land, tech, savings, income, tax, weather)
        // then addCrops individually

        // Shafin — 5ac, mixed rice+potato
        {
            farmer f(23, "Shafin", 30, 5.0, 0.60);
            f.savings = 28000;
            f.incomePerDay = 450;
            f.tax = 0.05;
            f.weather = 0.70;
            f.addCrop(&rice, {0.25, 38.0}, 50.0, 2.0, 120.0);
            f.addCrop(&potato, {0.15, 22.0}, 80.0, 4.0, 200.0);
            farmers.push_back(f);
        }
        // Khalek — 3ac, rice only, lower tech
        {
            farmer f(24, "Khalek", 35, 3.0, 0.40);
            f.savings = 14000;
            f.incomePerDay = 300;
            f.tax = 0.05;
            f.weather = 0.70;
            f.addCrop(&rice, {0.28, 42.0}, 45.0, 2.5, 80.0);
            farmers.push_back(f);
        }
        // Sohan — 4ac, corn + jute, moderate tech
        {
            farmer f(25, "Sohan", 40, 4.0, 0.50);
            f.savings = 19000;
            f.incomePerDay = 380;
            f.tax = 0.06;
            f.weather = 0.65;
            f.addCrop(&corn, {0.20, 28.0}, 60.0, 3.0, 150.0);
            f.addCrop(&jute, {0.30, 35.0}, 40.0, 3.5, 90.0);
            farmers.push_back(f);
        }
        // Sadnan — 2ac, banana + potato, small scale
        {
            farmer f(26, "Sadnan", 29, 2.0, 0.35);
            f.savings = 9000;
            f.incomePerDay = 240;
            f.tax = 0.04;
            f.weather = 0.75;
            f.addCrop(&banana, {0.12, 18.0}, 90.0, 5.0, 180.0);
            f.addCrop(&potato, {0.18, 24.0}, 70.0, 4.5, 140.0);
            farmers.push_back(f);
        }
        // Mahin — 8ac, large rice + corn operation, high tech
        {
            farmer f(27, "Mahin", 45, 8.0, 0.75);
            f.savings = 52000;
            f.incomePerDay = 620;
            f.tax = 0.07;
            f.weather = 0.60;
            f.addCrop(&rice, {0.22, 36.0}, 55.0, 1.8, 200.0);
            f.addCrop(&corn, {0.18, 26.0}, 65.0, 2.5, 180.0);
            farmers.push_back(f);
        }
        // Sohag — 1.5ac marginal farmer, potato only
        {
            farmer f(28, "Sohag", 32, 1.5, 0.25);
            f.savings = 5500;
            f.incomePerDay = 190;
            f.tax = 0.03;
            f.weather = 0.80;
            f.addCrop(&potato, {0.22, 28.0}, 55.0, 5.5, 80.0);
            farmers.push_back(f);
        }

        // ── FIRMS ─────────────────────────────────────────────────────────
        // Each firm starts with some workers already hired for non-zero output
        {
            // Rahim's garment firm — cloth, labour-intensive α=0.6
            firm f(11, 600000, cobbDouglas(0.6, 0.4, 1.2));
            f.products.push_back(cloth);
            f.wage = 430;
            f.fixed_overhead = 3500;
            f.workers.push_back(laborers[0]); // Kowshik
            f.workers.push_back(laborers[1]); // Cauchy
            f.capitals.emplace_back(800, 1.5);
            f.calculateCosts();
            firms.push_back(f);
        }
        {
            // Rohan's garment firm — cloth, balanced α=0.5
            firm f(13, 350000, cobbDouglas(0.5, 0.5, 1.5));
            f.products.push_back(cloth);
            f.wage = 410;
            f.fixed_overhead = 2500;
            f.workers.push_back(laborers[3]); // Shad
            f.capitals.emplace_back(600, 1.2);
            f.calculateCosts();
            firms.push_back(f);
        }
        {
            // Priom's electronics firm — computer, CES, high capital
            firm f(12, 1800000, ces(0.5));
            f.products.push_back(computer);
            f.wage = 750;
            f.fixed_overhead = 9000;
            f.workers.push_back(laborers[2]); // Mahin
            f.capitals.emplace_back(2000, 2.0);
            f.capitals.emplace_back(2000, 2.0);
            f.calculateCosts();
            firms.push_back(f);
        }
        {
            // Atef's textile firm — cloth, more workers
            firm f(15, 950000, cobbDouglas(0.55, 0.45, 1.3));
            f.products.push_back(cloth);
            f.wage = 450;
            f.fixed_overhead = 4200;
            f.workers.push_back(laborers[4]); // Mahir
            f.workers.push_back(laborers[5]); // Labib
            f.capitals.emplace_back(900, 1.6);
            f.calculateCosts();
            firms.push_back(f);
        }
        {
            // Somio's small food processing firm — rice/potato, CES
            firm f(19, 420000, cobbDouglas(0.65, 0.35, 1.1));
            f.products.push_back(rice);
            f.wage = 380;
            f.fixed_overhead = 1800;
            f.workers.push_back(laborers[6]); // Jubair
            f.capitals.emplace_back(500, 1.0);
            f.calculateCosts();
            firms.push_back(f);
        }
        {
            // Nahid's phone assembly firm — phone, high-capital CES
            firm f(17, 1200000, ces(0.45));
            f.products.push_back(phone);
            f.wage = 680;
            f.fixed_overhead = 5500;
            f.workers.push_back(laborers[7]); // Nabil
            f.capitals.emplace_back(1800, 1.8);
            f.capitals.emplace_back(1800, 1.8);
            f.calculateCosts();
            firms.push_back(f);
        }

        // ── AGENT CURVES ──────────────────────────────────────────────────
        initializeDemandCurves();
        initializeSupplyCurves();

        // ── INITIAL MARKET EQUILIBRIA ─────────────────────────────────────
        updateAllMarkets();

        // ── DEFAULT SELECTIONS ────────────────────────────────────────────
        selected_consumer = &consumers[0];
        selected_farmer = &farmers[0];
        selected_laborer = &laborers[0];
        selected_market = &markets[0];
        selected_firm = &firms[0];
    }

    // ── ADD HELPERS ───────────────────────────────────────────────────────
    void addConsumerFull(int id, const std::string &name, int age,
                         double savings, double income)
    {
        consumer c(id, name, age);
        c.savings = savings;
        c.incomePerDay = income;
        consumers.push_back(c);
    }

    void addLaborerFull(int id, const std::string &name, int age,
                        double skill, double minwage,
                        double savings, double income)
    {
        laborer l(id, name, age, skill, minwage);
        l.savings = savings;
        l.incomePerDay = income;
        laborers.push_back(l);
    }

    void addConsumer(std::string name, int age)
    {
        int id = 100 + (int)consumers.size();
        consumer c(id, name, age);
        consumers.push_back(c);
    }

    void addFarmer(std::string name, int age, double land, double techLevel)
    {
        int id = 120 + (int)farmers.size();
        farmers.emplace_back(id, name, age, land, techLevel);
    }

    void addlaborer(std::string name, int age, double skillLevel, double minWage)
    {
        int id = 140 + (int)laborers.size();
        laborers.emplace_back(id, name, age, skillLevel, minWage);
    }

    void addFirm(int id, double cash, cobbDouglas cd)
    {
        firms.emplace_back(id, cash, cd);
    }

    int getPopulation() const
    {
        return (int)(consumers.size() + laborers.size() + farmers.size());
    }

    consumer *GetSelectedConsumer() { return selected_consumer; }
    laborer *GetSelectedLaborer() { return selected_laborer; }
    farmer *GetSelectedFarmer() { return selected_farmer; }
    market *GetSelectedMarket() { return selected_market; }
    firm *GetSelectedFirm()
    {
        if (!selected_consumer)
            return nullptr;
        for (auto &f : firms)
            if (f.ownerId == selected_consumer->id)
                return &f;
        return nullptr;
    }

    void setDemandCurve(consumer &ag, product *prod, double slope, double intercept)
    {
        if (!prod)
            return;
        ag.needs.push_back(*prod);
        ag.dd[prod] = {std::max(0.05, slope), std::max(1.0, intercept)};
        ag.consumed[prod] = 0.0;
    }

    double baseCropCost(product *crop) const
    {
        if (!crop)
            return 30.0;
        if (crop->name == rice.name)
            return 37.0;
        if (crop->name == potato.name)
            return 22.0;
        if (crop->name == banana.name)
            return 18.0;
        if (crop->name == corn.name)
            return 27.0;
        if (crop->name == jute.name)
            return 34.0;
        return 30.0;
    }

    double baseCropSlope(product *crop) const
    {
        if (!crop)
            return 0.20;
        if (crop->name == rice.name)
            return 0.22;
        if (crop->name == potato.name)
            return 0.16;
        if (crop->name == banana.name)
            return 0.14;
        if (crop->name == corn.name)
            return 0.19;
        if (crop->name == jute.name)
            return 0.28;
        return 0.20;
    }

    // ── DEMAND CURVE INITIALIZATION ───────────────────────────────────────
    void initializeDemandCurves()
    {
        // Collect all agent pointers
        std::vector<consumer *> all;
        for (auto &c : consumers)
            all.push_back(&c);
        for (auto &f : farmers)
            all.push_back(&f);
        for (auto &l : laborers)
            all.push_back(&l);

        for (auto *ag : all)
        {
            double inc = ag->incomePerDay;
            double wealth = ag->savings + inc * 30;
            double tasteShift = ((ag->id % 5) - 2) * 1.2;

            ag->needs.clear();
            ag->dd.clear();
            ag->consumed.clear();
            ag->substitutionRatios.clear();

            // Calibrated around a Bangladesh-like consumer basket.
            // Curves are chosen so aggregated prices settle in a coherent range.

            // Staples / essentials
            setDemandCurve(*ag, &rice, 2.2, 95.0 + inc * 0.050 + tasteShift);
            setDemandCurve(*ag, &cloth, 2.0, 78.0 + inc * 0.060 + tasteShift);
            setDemandCurve(*ag, &potato, 2.5,
                           std::max(26.0, 44.0 + inc * 0.010 - wealth * 0.00008 + tasteShift));
            setDemandCurve(*ag, &banana, 2.0, 37.0 + inc * 0.030 + tasteShift);
            setDemandCurve(*ag, &corn, 2.2, 46.0 + inc * 0.020 + tasteShift);

            // Jute goods are mostly used by lower/middle income households.
            if (wealth < 90000.0)
            {
                setDemandCurve(*ag, &jute, 2.8, 50.0 + inc * 0.015 + tasteShift);
            }

            // Durables
            if (wealth > 22000.0)
            {
                setDemandCurve(*ag, &phone, 0.25,
                               68.0 + inc * 0.010 + wealth * 0.00045 + tasteShift);
            }
            if (wealth > 55000.0)
            {
                setDemandCurve(*ag, &computer, 0.18,
                               105.0 + inc * 0.020 + wealth * 0.00090 + tasteShift);
            }
        }
    }

    // ── SUPPLY CURVE INITIALIZATION (FARMERS) ─────────────────────────────
    void initializeSupplyCurves()
    {
        for (auto &f : farmers)
        {
            for (auto &[crop, line] : f.ss)
            {
                if (!crop)
                    continue;

                double baseCost = baseCropCost(crop);
                double baseSlope = baseCropSlope(crop);

                double smallFarmPenalty = f.land < 3.0 ? (3.0 - f.land) * 2.5 : 0.0;
                double techDiscount = f.techLevel * 8.0;
                double weatherPenalty = std::max(0.0, 0.65 - f.weather) * 10.0;
                double taxPenalty = f.tax * 30.0;

                line.c = std::max(8.0, baseCost - techDiscount + smallFarmPenalty +
                                           weatherPenalty + taxPenalty);
                line.m = std::max(0.08, baseSlope +
                                            (0.18 / std::max(1.0, f.land)) +
                                            (0.06 * (1.0 - f.techLevel)));
            }
        }
    }

    // ── MARKET UPDATE ─────────────────────────────────────────────────────
    void updateAllMarkets()
    {
        for (auto &m : markets)
        {
            m.calculateAggregateDemand(consumers, farmers, laborers);
            m.calculateAggregateSupply(farmers, firms); // firms contribute supply

            auto eq = m.findEquilibrium();

            // Only update price from equilibrium when BOTH curves are non-trivial.
            // If there's no supply curve, let Walrasian tâtonnement (adjustPrice) drive
            // the price instead of resetting it to the demand x-intercept each tick.
            bool hasDemand = m.aggregateDemand.m > 0.0001;
            bool hasSupply = m.aggregateSupply.m > 0.0001;

            if (hasDemand && hasSupply && eq.price > 0.1)
                m.price = eq.price;
            else if (m.price < 0.1)
                m.price = 0.1;

            // Record history
            m.priceHistory.push_back(m.price);
            if (m.priceHistory.size() > 30)
                m.priceHistory.erase(m.priceHistory.begin());
        }
    }

    // ── PASS DAY ──────────────────────────────────────────────────────────
    void pass_day()
    {
        dayCount++;
        double gdpPerCapita = currentStats.gdp / std::max(1, getPopulation());

        // 1. Markets re-equilibrate FIRST so entities see current prices
        updateAllMarkets();

        // Build price map
        std::map<std::string, double> prices;
        for (auto &m : markets)
            prices[m.prod->name] = m.price;

        // 2. entities respond to prices
        for (auto &c : consumers)
        {
            for (auto &need : c.needs)
            {
                auto it = prices.find(need.name);
                if (it != prices.end())
                    c.updateDemandForPriceChange(c.findKey(need.name), it->second);
            }
            c.pass_day(gdpPerCapita, prices);
        }
        for (auto &f : farmers)
        {
            for (auto &need : f.needs)
            {
                auto it = prices.find(need.name);
                if (it != prices.end())
                    f.updateDemandForPriceChange(f.findKey(need.name), it->second);
            }
            f.pass_day(gdpPerCapita, prices);
        }
        for (auto &l : laborers)
        {
            for (auto &need : l.needs)
            {
                auto it = prices.find(need.name);
                if (it != prices.end())
                    l.updateDemandForPriceChange(l.findKey(need.name), it->second);
            }
            l.pass_day(gdpPerCapita, prices);
        }

        // 3. Re-clear markets with updated demand/supply
        updateAllMarkets();

        // 4. Firms optimize input mix
        for (auto &fi : firms)
            fi.calculateCosts();

        firmOptimize(); // auto-hire / fire based on market conditions

        // 5. Macro stats
        calculateStats();

        // 6. Walrasian tâtonnement price adjustment
        for (auto &m : markets)
            m.adjustPrice();

        // 7. Stochastic income shocks (simulate wage drift, side income, bad days)
        //    Each agent has a small random ±5% daily income jitter to keep things moving
        applyIncomeShocks();

        // 8. Periodic demand shocks every ~7 days (taste/season changes)
        if (dayCount % 7 == 0)
            applyDemandShock();
    }

    // ── INCOME SHOCKS ─────────────────────────────────────────────────────
    void applyIncomeShocks()
    {
        auto jitter = [](double base, double pct = 0.08) -> double
        {
            double delta = ((double)rand() / RAND_MAX - 0.5) * pct;
            return std::max(50.0, base * (1.0 + delta));
        };
        for (auto &c : consumers)
            c.incomePerDay = jitter(c.incomePerDay);
        for (auto &l : laborers)
            l.incomePerDay = jitter(l.incomePerDay);
        for (auto &f : farmers)
            f.incomePerDay = jitter(f.incomePerDay);

        // ── Firm wage dynamics ────────────────────────────────────────────
        double labourForce = (double)laborers.size();
        double totalHired = 0;
        for (auto &fi : firms)
            totalHired += (double)fi.workers.size();
        double empRate = labourForce > 0 ? totalHired / labourForce : 0.5;

        // Tight labour market → wages drift up; slack → drift down
        double wageTrend = empRate > 0.80 ? 1.012 : empRate > 0.55 ? 1.003
                                                                   : 0.994;

        for (auto &fi : firms)
        {
            fi.wage = std::max(250.0, jitter(fi.wage * wageTrend, 0.06));
            fi.calculateCosts();
        }
    }

    // ── FIRM AUTO-HIRE / FIRE ─────────────────────────────────────────────
    static constexpr double FIRM_OUTPUT_SCALE = 80.0;

    void firmOptimize()
    {
        for (auto &fi : firms)
        {
            // Find this firm's primary market price
            double mktPrice = 0.0;
            for (auto &fp : fi.products)
                for (auto &m : markets)
                    if (m.prod->name == fp.name && m.price > mktPrice)
                        mktPrice = m.price;
            if (mktPrice < 1.0)
                continue;

            double mpL = fi.MPofLabor();
            double revPerWorker = mpL * FIRM_OUTPUT_SCALE * mktPrice;
            bool shouldHire = (revPerWorker > fi.wage * 1.05) && ((int)fi.workers.size() < 8);
            bool shouldFire = (revPerWorker < fi.wage * 0.80) && ((int)fi.workers.size() > 1);

            if (shouldHire)
            {
                // Hire the highest-skilled unemployed laborer within wage budget
                laborer *best = nullptr;
                for (auto &l : laborers)
                {
                    if (l.minWage > fi.wage)
                        continue;
                    bool alreadyHired = false;
                    for (auto &fi2 : firms)
                        for (auto &w : fi2.workers)
                            if (w.name == l.name)
                            {
                                alreadyHired = true;
                                break;
                            }
                    if (alreadyHired)
                        continue;
                    if (!best || l.skillLevel > best->skillLevel)
                        best = &l;
                }
                if (best)
                {
                    fi.workers.push_back(*best);
                    fi.calculateCosts();
                }
            }
            else if (shouldFire)
            {
                fi.workers.pop_back();
                fi.calculateCosts();
            }

            // Occasionally add capital if MPK/r is favourable
            if ((rand() % 20 == 0) && fi.MPofCapital() * FIRM_OUTPUT_SCALE * mktPrice > fi.averageCost * 0.5)
            {
                double rental = fi.wage * 1.8 + ((double)rand() / RAND_MAX) * 200.0;
                double eff = 1.0 + ((double)rand() / RAND_MAX) * 1.0;
                fi.capitals.emplace_back(rental, eff);
                fi.calculateCosts();
            }
        }
    }

    // ── PERIODIC DEMAND SHOCKS ────────────────────────────────────────────
    // Every 7 days, apply a small random shift to one product's aggregate demand
    // (mimics taste change, seasonal variation, news events, etc.)
    void applyDemandShock()
    {
        // Pick a random market
        if (markets.empty())
            return;
        int idx = rand() % (int)markets.size();
        market &m = markets[idx];

        // Shift the aggregate demand intercept by ±5%
        double shock = 1.0 + ((double)rand() / RAND_MAX - 0.5) * 0.10;

        // Propagate to all entities who demand this product
        auto shiftAgent = [&](consumer &ag)
        {
            product *key = ag.findKey(m.prod->name);
            if (key)
                ag.dd[key].c = std::max(1.0, ag.dd[key].c * shock);
        };
        for (auto &c : consumers)
            shiftAgent(c);
        for (auto &f : farmers)
            shiftAgent(static_cast<consumer &>(f));
        for (auto &l : laborers)
            shiftAgent(static_cast<consumer &>(l));
    }

    // ── MACRO STATS ───────────────────────────────────────────────────────
    void calculateStats()
    {
        currentStats.population = getPopulation();
        currentStats.firms = (int)firms.size();

        double totalProduction = 0.0;
        for (auto &m : markets)
        {
            auto eq = m.findEquilibrium();
            totalProduction += eq.price * eq.quantity;
        }
        currentStats.gdp = totalProduction;

        currentStats.employed = 0;
        for (const auto &fi : firms)
            currentStats.employed += (int)fi.workers.size();

        int labourForce = (int)laborers.size();
        currentStats.unemployment = labourForce > 0
                                        ? (double)(labourForce - currentStats.employed) / labourForce
                                        : 0.0;

        currentStats.moneySupply = 0.0;
        for (const auto &c : consumers)
            currentStats.moneySupply += c.savings;
        for (const auto &f : farmers)
            currentStats.moneySupply += f.savings;
        for (const auto &l : laborers)
            currentStats.moneySupply += l.savings;
        for (const auto &fi : firms)
            currentStats.moneySupply += fi.cash;
    }

    std::string getStyledGDP()
    {
        std::stringstream ss;
        ss << Header("GDP BREAKDOWN") << "\n\n";
        double total = 0.0;
        for (auto &m : markets)
        {
            auto eq = m.findEquilibrium();
            double mv = eq.price * eq.quantity;
            if (mv > 0.01)
            {
                ss << KeyValue(m.prod->name,
                               "Tk " + std::to_string(twoDecimal(mv)) +
                                   " (" + std::to_string(twoDecimal(eq.quantity)) +
                                   " units @ Tk" + std::to_string(twoDecimal(eq.price)) + ")")
                   << "\n";
                total += mv;
            }
        }
        ss << styledTerminal::Separator(60) << "\n";
        ss << Styled("Total GDP: Tk " + std::to_string(twoDecimal(total)),
                     std::string(Color::Bold).append(Theme::Success).c_str())
           << "\n";
        ss << KeyValue("GDP per Capita",
                       "Tk " + std::to_string(twoDecimal(total / std::max(1, getPopulation()))))
           << "\n";
        return ss.str();
    }

    void updateDemandCurves()
    {
        std::vector<consumer *> all;
        for (auto &c : consumers)
            all.push_back(&c);
        for (auto &f : farmers)
            all.push_back(&f);
        for (auto &l : laborers)
            all.push_back(&l);

        for (auto *ag : all)
        {
            ag->muPerTk = ag->getMUperTk();
            for (auto &need : ag->needs)
            {
                product *key = ag->findKey(need.name);
                if (key)
                {
                    double incomeEffect = ag->incomePerDay * 0.01 * need.eta;
                    ag->dd[key].c = std::max(1.0, ag->dd[key].c + incomeEffect * 0.1);
                }
            }
        }
    }
};
