#include "Person.h"
#include "Economy.h"
#include "Firm.h"
#include "../utils/Logger.h"
#include <cmath>
#include <sstream>

Person::Person(int id, int age)
    : Agent(id), m_age(age), m_employmentStatus(age < Config::WORKING_AGE_MIN ? EmploymentStatus::CHILD : age >= Config::RETIREMENT_AGE ? EmploymentStatus::RETIRED
                                                                                                                                        : EmploymentStatus::UNEMPLOYED),
      m_employer(nullptr), m_income(0.0), m_savings(0.0), m_lastConsumption(0.0), m_health(1.0)
{
    // Initialize with random starting wealth based on age
    auto &rng = Statistics::Random::getInstance();
    m_wealth = rng.uniform(1000, 50000) * (1.0 + m_age * 0.01);

    // Initialize random skills
    int numSkills = rng.uniformInt(1, 3);
    for (int i = 0; i < numSkills; ++i)
    {
        Sector s = static_cast<Sector>(rng.uniformInt(0, static_cast<int>(Sector::CPPC_Count) - 1));
        double level = rng.uniform(0.3, 1.0);
        addSkill(s, level);
    }
}

void Person::update(Economy &economy)
{
    if (!m_active)
        return;

    ageTick();

    if (checkDeath())
    {
        deactivate();
        return;
    }

    // Update employment status based on age
    if (m_age >= Config::RETIREMENT_AGE && m_employmentStatus != EmploymentStatus::RETIRED)
    {
        retire();
    }

    if (m_employmentStatus == EmploymentStatus::UNEMPLOYED && canWork())
    {
        seekJob(economy);
    }

    save();
    consume(economy);
}

void Person::consume(Economy &economy)
{
    // Calculate consumption based on income and wealth
    double disposableIncome = m_income * (1.0 - economy.getGovernment().getTaxRate());
    double savingsRate = calculateSavingsRate();

    double targetConsumption = disposableIncome * (1.0 - savingsRate);

    // Add wealth-based consumption (propensity to consume out of wealth)
    targetConsumption += m_wealth * 0.02;

    // Can't consume more than we have
    double actualConsumption = std::min(targetConsumption, m_wealth + disposableIncome);
    actualConsumption = std::max(0.0, actualConsumption);

    m_lastConsumption = actualConsumption;
    m_wealth -= actualConsumption;

    // This consumption goes to firms through the goods market
    economy.recordConsumption(actualConsumption);
}

void Person::save()
{
    double savingsRate = calculateSavingsRate();
    double savingsAmount = m_income * savingsRate;
    m_savings += savingsAmount;
    m_wealth += savingsAmount;
}

void Person::seekJob(Economy &economy)
{
    // Try to find a job through the labor market
    economy.getLaborMarket().addJobSeeker(this);
}

void Person::ageTick()
{
    // Age by one month (1/12 of a year)
    static int monthCounter = 0;
    monthCounter++;
    if (monthCounter >= Config::TICKS_PER_YEAR)
    {
        m_age++;
        monthCounter = 0;

        // Health declines with age
        auto &rng = Statistics::Random::getInstance();
        if (m_age > 50)
        {
            m_health -= rng.uniform(0.001, 0.01) * (m_age - 50) / 50.0;
            m_health = std::max(0.0, m_health);
        }
    }
}

bool Person::checkDeath()
{
    if (m_age >= Config::MAX_AGE)
        return true;

    auto &rng = Statistics::Random::getInstance();

    // Base mortality increases with age
    double baseMortality = 0.0;
    if (m_age > 60)
    {
        baseMortality = std::pow((m_age - 60.0) / 40.0, 3) * 0.1;
    }

    // Health affects mortality
    double healthFactor = 1.0 - m_health;
    double deathProb = baseMortality + healthFactor * 0.05;

    return rng.bernoulli(deathProb / Config::TICKS_PER_YEAR);
}

void Person::setEmployer(Firm *firm, double wage)
{
    m_employer = firm;
    m_income = wage;
    m_employmentStatus = EmploymentStatus::EMPLOYED;
}

void Person::loseJob()
{
    m_employer = nullptr;
    m_income = 0.0;
    if (canWork())
    {
        m_employmentStatus = EmploymentStatus::UNEMPLOYED;
    }
}

void Person::retire()
{
    if (m_employer)
    {
        m_employer = nullptr;
    }
    m_employmentStatus = EmploymentStatus::RETIRED;
    m_income = m_savings * 0.04; // Draw from savings
}

void Person::addSkill(Sector sector, double level)
{
    for (auto &skill : m_skills)
    {
        if (skill.sector == sector)
        {
            skill.level = std::max(skill.level, level);
            return;
        }
    }
    m_skills.push_back({sector, level});
}

double Person::getSkillLevel(Sector sector) const
{
    for (const auto &skill : m_skills)
    {
        if (skill.sector == sector)
        {
            return skill.level;
        }
    }
    return 0.0;
}

double Person::calculateSavingsRate() const
{
    // Life-Cycle Hypothesis: Young borrow, Middle-aged save, Old dissave
    if (m_age < Config::YOUNG_AGE_MAX)
    {
        return Config::SAVINGS_RATE_YOUNG;
    }
    else if (m_age < Config::MIDDLE_AGE_MAX)
    {
        return Config::SAVINGS_RATE_MIDDLE;
    }
    else
    {
        return Config::SAVINGS_RATE_OLD;
    }
}

double Person::calculateMarginalUtility(double quantity) const
{
    // Diminishing marginal utility: MU = alpha * Q^(-beta)
    const double alpha = 1.0;
    const double beta = 0.5;
    if (quantity <= 0)
        return alpha;
    return alpha * std::pow(quantity, -beta);
}
