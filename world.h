#pragma once
#include <vector>
#include <string>
#include "consumer.h"
#include "laborer.h"
#include "farmer.h"
#include "firm.h"
#include "market.h"

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

    world() : selected_consumer(nullptr), selected_laborer(nullptr), selected_farmer(nullptr), selected_market(nullptr) {}

    stats getStats()
    {
        // Update stats before returning
        currentStats.population = consumers.size() + laborers.size() + farmers.size();
        currentStats.firms = firms.size();
        return currentStats;
    }

    void innitialize()
    {
        // create markets for all defined products

        consumer rahim(11, "Rahim", 28);
        consumer priom(12, "Priom", 26);
        consumer rohan(13, "Rohan", 20);

        farmer shafin(23, "Shafin", 30, 100.0, 0.5);
        shafin.addCrop(&rice, {0.1, 5.0}, 2.0, 0.5, 50.0); // supply line, growth rate, decay rate, initial max output

        farmer khalek(24, "khalek", 30, 100.0, 0.5);
        khalek.addCrop(&rice, {0.1, 5.0}, 2.0, 0.5, 50.0); // supply line, growth rate, decay rate, initial max output

        laborer kowshik(31, "Kowshik", 25, 0.5, 50.0);
        laborer cauchy(32, "Cauchy", 22, 0.7, 70.0);

        //                                    ╭ alpha, bita, A
        firm rahims(11, 10000.0, cobbDouglas(0.5, 0.5, 1.0)); // id 11 is the owner ie Rahim
        rahims.products.push_back(cloth);

        firm rohans(13, 5000.0, cobbDouglas(0.5, 0.5, 1.25)); // higher tech level!
        rohans.products.push_back(cloth);

        //                           ╭ rho
        firm prioms(12, 8000.0, ces(0.5));
        prioms.products.push_back(computer);

        consumers.push_back(rahim);
        consumers.push_back(priom);
        consumers.push_back(rohan);
        selected_consumer = &consumers[0];

        farmers.push_back(shafin);
        farmers.push_back(khalek);
        selected_farmer = &farmers[0];

        laborers.push_back(kowshik);
        laborers.push_back(cauchy);
        selected_laborer = &laborers[0];

        firms.push_back(rahims);
        firms.push_back(prioms);

        addBasicNeeds();

        std::vector<product *> products = {&rice, &cloth, &computer, &phone, &car, &steel};
        for (auto &prod : products)
        {
            markets.emplace_back(prod);
        }
        selected_market = &markets[0];
    };

    void addConsumer(std::string name, int age)
    {
        int id = consumers.size() + 11;
        consumers.emplace_back(id, name, age);
    };

    void addFarmer(std::string name, int age, double land, double techLevel)
    {
        int id = farmers.size() + 21;
        farmers.emplace_back(id, name, age, land, techLevel);
    };

    void addlaborer(std::string name, int age, double skillLevel, double minWage)
    {
        int id = laborers.size() + 31;
        laborers.emplace_back(id, name, age, skillLevel, minWage);
    };

    void addFirm(int id, double cash, cobbDouglas cd)
    {
        firms.emplace_back(id, cash, cd);
    };

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
    };

    consumer *GetSelectedConsumer()
    {
        return selected_consumer;
    };
    laborer *GetSelectedLaborer()
    {

        return selected_laborer;
    };
    farmer *GetSelectedFarmer()
    {

        return selected_farmer;
    };
    market *GetSelectedMarket()
    {
        return selected_market;
    };

    int getPopulation()
    {
        return consumers.size() + laborers.size() + farmers.size();
    }
};