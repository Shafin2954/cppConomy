#pragma once

#include "Agent.h"
#include "../utils/Config.h"
#include <vector>
#include <string>

class Person;
class Economy;

class Firm : public Agent
{
public:
    Firm(int id, Sector sector = Sector::CPPC_Services);

    void update(Economy &economy) override;

    // Production behaviors
    void produce(Economy &economy);
    void setPrice(Economy &economy);
    void hire(Person *worker);
    void fire(Person *worker);
    void payWages();

    // Getters
    Sector getSector() const { return m_sector; }
    double getCapital() const { return m_capital; }
    double getInventory() const { return m_inventory; }
    double getPrice() const { return m_price; }
    double getWage() const { return m_wage; }
    double getRevenue() const { return m_revenue; }
    double getProfit() const { return m_profit; }
    double getProduction() const { return m_lastProduction; }
    int getWorkerCount() const { return static_cast<int>(m_laborForce.size()); }
    const std::vector<Person *> &getLaborForce() const { return m_laborForce; }
    int getJobOpenings() const { return m_jobOpenings; }

    // Setters
    void setJobOpenings(int openings) { m_jobOpenings = openings; }
    void addCapital(double amount) { m_capital += amount; }
    void sellGoods(double quantity, double price);

private:
    double calculateMarginalCost() const;
    double calculateMarginalRevenue(Economy &economy) const;
    double calculateOptimalOutput(Economy &economy) const;
    double calculateProductionCapacity() const;

    Sector m_sector;
    double m_capital;
    double m_inventory;
    double m_price;
    double m_wage;
    double m_revenue;
    double m_profit;
    double m_lastProduction;
    double m_targetInventory;
    int m_jobOpenings;

    std::vector<Person *> m_laborForce;
};
