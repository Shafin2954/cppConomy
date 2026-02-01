#pragma once

#include "Agent.h"
#include "../utils/Config.h"
#include <vector>
#include <queue>
#include <functional>

class Person;
class Firm;

struct JobOpening
{
    Firm *firm;
    int positions;
    double wage;
};

struct JobSeeker
{
    Person *person;
    double reservationWage; // Minimum acceptable wage
};

class Market
{
public:
    enum class Type
    {
        LABOR,
        GOODS
    };

    Market(Type type) : m_type(type) {}

    // Labor market operations
    void addJobOpening(Firm *firm, int positions);
    void addJobSeeker(Person *person);
    void clearLaborMarket();

    // Goods market operations
    void addSupply(Firm *firm, double quantity, double price);
    void addDemand(double amount);
    void clearGoodsMarket();

    // Stats
    int getJobOpenings() const;
    int getJobSeekers() const { return static_cast<int>(m_jobSeekers.size()); }
    double getTotalDemand() const { return m_totalDemand; }
    double getTotalSupply() const { return m_totalSupply; }
    double getAveragePrice() const { return m_averagePrice; }
    double getAverageWage() const { return m_averageWage; }

    void reset();

private:
    Type m_type;

    // Labor market
    std::vector<JobOpening> m_jobOpenings;
    std::vector<JobSeeker> m_jobSeekers;
    double m_averageWage = Config::INITIAL_WAGE / Config::TICKS_PER_YEAR;

    // Goods market
    struct Supply
    {
        Firm *firm;
        double quantity;
        double price;
    };
    std::vector<Supply> m_supplies;
    double m_totalDemand = 0.0;
    double m_totalSupply = 0.0;
    double m_averagePrice = Config::INITIAL_PRICE;
};
