#pragma once
#include <vector>
#include <string>
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
        double gdpGrowth = 0.0;
        double inflation = 0.0;
        double cpi = 100.0;
        double unemployment = 0.0;
        int employed = 0;
        int population = 0;
        double moneySupply = 0.0;
        double interestRate = 0.0;
        double budget = 0.0;
        double debt = 0.0;
        double giniCoefficient = 0.0;
        int firms = 0;
    };

    stats currentStats;

    std::vector<consumer> consumers;
    std::vector<laborer> laborers;
    std::vector<farmer> farmers;
    std::vector<firm> firms;
    std::vector<market> markets;

    consumer *selected_consumer;
    laborer *selected_laborer;
    farmer *selected_farmer;
    market *selected_market;
    firm *selected_firm;

    world() : selected_consumer(nullptr), selected_laborer(nullptr), selected_farmer(nullptr), selected_market(nullptr), selected_firm(nullptr) {}

    stats getStats()
    {
        // Update stats before returning
        currentStats.population = consumers.size() + laborers.size() + farmers.size();
        currentStats.firms = firms.size();
        return currentStats;
    }

    void innitialize()
    {
        // Initialize products and markets FIRST
        std::vector<product *> products = {&rice, &cloth, &computer, &phone, &car, &steel, &potato, &banana};
        for (auto &prod : products)
        {
            markets.emplace_back(prod);
        }
        selected_market = &markets[0];

        // Create consumers with realistic initial conditions
        consumer rahim(11, "Rahim", 28);
        rahim.savings = 5000.0;
        rahim.incomePerDay = 150.0;
        rahim.expenses = 0.0;

        consumer priom(12, "Priom", 26);
        priom.savings = 8000.0;
        priom.incomePerDay = 200.0;
        priom.expenses = 0.0;

        consumer rohan(13, "Rohan", 20);
        rohan.savings = 3000.0;
        rohan.incomePerDay = 100.0;
        rohan.expenses = 0.0;

        // Create farmers with realistic parameters
        farmer shafin(23, "Shafin", 30, 5.0, 0.6); // 5 acres, 60% tech
        shafin.savings = 10000.0;
        shafin.incomePerDay = 200.0;
        shafin.tax = 0.15;    // 15% tax
        shafin.weather = 0.7; // Start with good weather
        shafin.addCrop(&rice, {0.15, 8.0}, 50.0, 0.1, 100.0);
        shafin.addCrop(&potato, {0.2, 6.0}, 80.0, 0.15, 150.0);

        farmer khalek(24, "Khalek", 35, 3.0, 0.4); // 3 acres, 40% tech
        khalek.savings = 6000.0;
        khalek.incomePerDay = 120.0;
        khalek.tax = 0.15;
        khalek.weather = 0.7;
        khalek.addCrop(&rice, {0.15, 8.0}, 45.0, 0.12, 80.0);

        // Create laborers
        laborer kowshik(31, "Kowshik", 25, 0.7, 80.0);
        kowshik.savings = 2000.0;
        kowshik.incomePerDay = 80.0;

        laborer cauchy(32, "Cauchy", 22, 0.5, 60.0);
        cauchy.savings = 1500.0;
        cauchy.incomePerDay = 60.0;

        // Create firms
        firm rahims(11, 10000.0, cobbDouglas(0.6, 0.4, 1.2));
        rahims.products.push_back(cloth);
        rahims.wage = 75.0;
        rahims.fixed_overhead = 500.0;

        firm rohans(13, 5000.0, cobbDouglas(0.5, 0.5, 1.5));
        rohans.products.push_back(cloth);
        rohans.wage = 70.0;
        rohans.fixed_overhead = 300.0;

        firm prioms(12, 8000.0, ces(0.5));
        prioms.products.push_back(computer);
        prioms.wage = 100.0;
        prioms.fixed_overhead = 800.0;

        // Add to vectors
        consumers.push_back(rahim);
        consumers.push_back(priom);
        consumers.push_back(rohan);

        farmers.push_back(shafin);
        farmers.push_back(khalek);

        laborers.push_back(kowshik);
        laborers.push_back(cauchy);

        firms.push_back(rahims);
        firms.push_back(rohans);
        firms.push_back(prioms);

        // Initialize demand for all consumers (including farmers/laborers who inherit from consumer)
        initializeDemandCurves();

        // Calculate initial market equilibria
        updateAllMarkets();

        // Set selections
        selected_consumer = &consumers[0];
        selected_farmer = &farmers[0];
        selected_laborer = &laborers[0];
        selected_market = &markets[0];
        selected_firm = &firms[0];
    }

    void addConsumer(std::string name, int age)
    {
        int id = consumers.size() + 11;
        consumers.emplace_back(id, name, age);
    }

    void addFarmer(std::string name, int age, double land, double techLevel)
    {
        int id = farmers.size() + 21;
        farmers.emplace_back(id, name, age, land, techLevel);
    }

    void addlaborer(std::string name, int age, double skillLevel, double minWage)
    {
        int id = laborers.size() + 31;
        laborers.emplace_back(id, name, age, skillLevel, minWage);
    }

    void addFirm(int id, double cash, cobbDouglas cd)
    {
        firms.emplace_back(id, cash, cd);
    }

    void addBasicNeeds()
    {
        // for each laborer and farmer, add basic needs demand lines for rice
        for (auto &consumer : consumers)
        {
            consumer.needs.push_back(rice);
            consumer.dd[&rice] = {0.1, 10.0}; // m, c
            consumer.consumed[&rice] = 0.0;

            consumer.needs.push_back(cloth);
            consumer.dd[&cloth] = {0.05, 5.0}; // m, c
            consumer.consumed[&cloth] = 0.0;
        }
        for (auto &laborer : laborers)
        {
            laborer.needs.push_back(rice);
            laborer.dd[&rice] = {0.1, 10.0}; // m, c
            laborer.consumed[&rice] = 0.0;

            laborer.needs.push_back(cloth);
            laborer.dd[&cloth] = {0.05, 5.0}; // m, c
            laborer.consumed[&cloth] = 0.0;
        }
        for (auto &farmer : farmers)
        {
            farmer.needs.push_back(rice);
            farmer.dd[&rice] = {0.1, 10.0}; // m, c
            farmer.consumed[&rice] = 0.0;

            farmer.needs.push_back(cloth);
            farmer.dd[&cloth] = {0.05, 5.0}; // m, c
            farmer.consumed[&cloth] = 0.0;
        }
    }

    consumer *GetSelectedConsumer()
    {
        return selected_consumer;
    }
    laborer *GetSelectedLaborer()
    {

        return selected_laborer;
    }
    farmer *GetSelectedFarmer()
    {

        return selected_farmer;
    }
    market *GetSelectedMarket()
    {
        return selected_market;
    }

    firm *GetSelectedFirm()
    {
        for (auto &firm : firms)
        {
            if (firm.ownerId == selected_consumer->id)
                return &firm;
        }
        return nullptr;
    }

    int getPopulation()
    {
        return consumers.size() + laborers.size() + farmers.size();
    }

    void initializeDemandCurves()
    {
        // Combine all consumers (consumers + farmers + laborers)
        std::vector<consumer *> allConsumers;
        for (auto &c : consumers)
            allConsumers.push_back(&c);
        for (auto &f : farmers)
            allConsumers.push_back(&f);
        for (auto &l : laborers)
            allConsumers.push_back(&l);

        for (auto *consumer : allConsumers)
        {
            double income = consumer->incomePerDay;
            double wealth = consumer->savings + income * 30;

            // Rice - necessity (steep demand, low price sensitivity)
            consumer->needs.push_back(rice);
            consumer->dd[&rice] = {0.08, 12.0 + income * 0.01};
            consumer->consumed[&rice] = 0.0;

            // Cloth - normal good
            consumer->needs.push_back(cloth);
            consumer->dd[&cloth] = {0.10, 8.0 + income * 0.015};
            consumer->consumed[&cloth] = 0.0;

            // Computer - luxury (only if wealthy enough)
            if (wealth > 5000.0)
            {
                consumer->needs.push_back(computer);
                consumer->dd[&computer] = {0.5, wealth * 0.002};
                consumer->consumed[&computer] = 0.0;
            }
        }
    }

    void updateAllMarkets()
    {
        // Combine all consumer types for demand
        std::vector<consumer> allConsumers;
        allConsumers.insert(allConsumers.end(), consumers.begin(), consumers.end());
        allConsumers.insert(allConsumers.end(), farmers.begin(), farmers.end());
        allConsumers.insert(allConsumers.end(), laborers.begin(), laborers.end());

        for (auto &market : markets)
        {
            market.calculateAggregateDemand(allConsumers);
            market.calculateAggregateSupply(farmers);

            auto eq = market.findEquilibrium();
            market.price = eq.price > 0 ? eq.price : market.price;

            // Ensure minimum price
            if (market.price < 0.1)
                market.price = 0.1;
        }
    }

    void pass_day()
    {
        double gdpPerCapita = currentStats.gdp / getPopulation();

        // 1. Update market prices FIRST (so agents know current prices)
        updateAllMarkets();

        // 2. Agents react to prices and consume
        for (auto &c : consumers)
        {
            // Update demand based on current market prices
            for (auto &need : c.needs)
            {
                for (auto &market : markets)
                {
                    if (market.prod->name == need.name)
                    {
                        c.updateDemandForPriceChange(&need, market.price);
                        break;
                    }
                }
            }
            c.pass_day(gdpPerCapita);
        }

        for (auto &f : farmers)
        {
            // Update demand for consumed goods
            for (auto &need : f.needs)
            {
                for (auto &market : markets)
                {
                    if (market.prod->name == need.name)
                    {
                        f.updateDemandForPriceChange(&need, market.price);
                        break;
                    }
                }
            }
            f.pass_day(gdpPerCapita);
        }

        for (auto &l : laborers)
        {
            // Update demand for consumed goods
            for (auto &need : l.needs)
            {
                for (auto &market : markets)
                {
                    if (market.prod->name == need.name)
                    {
                        l.updateDemandForPriceChange(&need, market.price);
                        break;
                    }
                }
            }
            l.pass_day(gdpPerCapita);
        }

        // 3. Markets re-equilibrate based on new demand/supply
        updateAllMarkets();

        // 4. Firms optimize production
        for (auto &firm : firms)
        {
            firm.calculateCosts();
        }

        // 5. Update macroeconomic statistics
        calculateStats();

        // 6. Markets adjust prices toward equilibrium (tâtonnement process)
        for (auto &market : markets)
        {
            market.adjustPrice();
        }
    }

    std::string getStyledGDP()
{
    using namespace styledTerminal;
    std::stringstream ss;
    
    ss << Header("GDP BREAKDOWN") << "\n\n";
    
    double total = 0.0;
    for (auto& market : markets)
    {
        auto eq = market.findEquilibrium();
        double marketValue = eq.price * eq.quantity;
        
        if (marketValue > 0.01)  // Only show active markets
        {
            ss << KeyValue(market.prod->name, 
                "$" + std::to_string(twoDecimal(marketValue)) + 
                " (" + std::to_string(twoDecimal(eq.quantity)) + 
                " units @ $" + std::to_string(twoDecimal(eq.price)) + ")") << "\n";
            total += marketValue;
        }
    }
    
    ss << styledTerminal::Separator(60) << "\n";
    ss << Styled("Total GDP: $" + std::to_string(twoDecimal(total)), 
                 std::string(Color::Bold).append(Theme::Success).c_str()) << "\n";
    ss << KeyValue("GDP per Capita", 
                   "$" + std::to_string(twoDecimal(total / getPopulation()))) << "\n";
    
    return ss.str();
}

    void updateDemandCurves()
    {
        std::vector<consumer *> allConsumers;
        for (auto &c : consumers)
            allConsumers.push_back(&c);
        for (auto &f : farmers)
            allConsumers.push_back(&f);
        for (auto &l : laborers)
            allConsumers.push_back(&l);

        for (auto *consumer : allConsumers)
        {
            consumer->muPerDollar = consumer->getMUperDollar();

            // Update demand intercepts based on income (Engel curve effect)
            for (auto &need : consumer->needs)
            {
                if (consumer->dd.find(&need) != consumer->dd.end())
                {
                    double incomeEffect = consumer->incomePerDay * 0.01 * need.eta;
                    consumer->dd[&need].c = std::max(1.0, consumer->dd[&need].c + incomeEffect * 0.1);
                }
            }
        }
    }

    void calculateStats()
    {
        currentStats.population = getPopulation();
        currentStats.firms = firms.size();

        // Calculate GDP = Sum of (Price × Quantity) for all markets
        double totalProduction = 0.0;

        // Get production value from each market
        for (auto &market : markets)
        {
            auto eq = market.findEquilibrium();
            double marketValue = eq.price * eq.quantity;
            totalProduction += marketValue;
        }

        currentStats.gdp = totalProduction;

        // Employment
        currentStats.employed = 0;
        for (const auto &firm : firms)
        {
            currentStats.employed += firm.workers.size();
        }

        int laborForce = laborers.size();
        currentStats.unemployment = laborForce > 0 ? (double)(laborForce - currentStats.employed) / laborForce : 0.0;

        // Money supply = total savings across all agents
        currentStats.moneySupply = 0.0;
        for (const auto &c : consumers)
            currentStats.moneySupply += c.savings;
        for (const auto &f : farmers)
            currentStats.moneySupply += f.savings;
        for (const auto &l : laborers)
            currentStats.moneySupply += l.savings;
        for (const auto &firm : firms)
            currentStats.moneySupply += firm.cash;
    }
};