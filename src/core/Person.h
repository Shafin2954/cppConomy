#pragma once

#include "Agent.h"
#include "../utils/Config.h"
#include "../utils/Statistics.h"
#include <vector>
#include <string>

class Firm;

enum class EmploymentStatus
{
    EMPLOYED,
    UNEMPLOYED,
    RETIRED,
    CHILD
};

struct Skill
{
    Sector sector;
    double level; // 0.0 to 1.0
};

class Person : public Agent
{
public:
    Person(int id, int age = 25);

    void update(Economy &economy) override;

    // Behaviors
    void consume(Economy &economy);
    void save();
    void seekJob(Economy &economy);
    void ageTick();
    bool checkDeath();

    // Getters
    int getAge() const { return m_age; }
    EmploymentStatus getEmploymentStatus() const { return m_employmentStatus; }
    bool isEmployed() const { return m_employmentStatus == EmploymentStatus::EMPLOYED; }
    bool canWork() const
    {
        return m_age >= Config::WORKING_AGE_MIN && m_age < Config::RETIREMENT_AGE;
    }
    double getIncome() const { return m_income; }
    double getConsumption() const { return m_lastConsumption; }
    Firm *getEmployer() const { return m_employer; }
    const std::vector<Skill> &getSkills() const { return m_skills; }

    // Setters
    void setEmployer(Firm *firm, double wage);
    void loseJob();
    void retire();

    // Skill management
    void addSkill(Sector sector, double level);
    double getSkillLevel(Sector sector) const;

private:
    double calculateSavingsRate() const;
    double calculateMarginalUtility(double quantity) const;

    int m_age;
    EmploymentStatus m_employmentStatus;
    Firm *m_employer;
    double m_income;
    double m_savings;
    double m_lastConsumption;
    std::vector<Skill> m_skills;
    double m_health; // 0.0 to 1.0
};
