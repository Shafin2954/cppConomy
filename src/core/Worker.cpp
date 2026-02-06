#include "Worker.h"
#include <sstream>
#include <algorithm>

Worker::Worker(int id, const std::string &name, double initial_income, double skill)
    : Person(id, name, initial_income),
      m_skill_level(skill),
      m_min_acceptable_wage(10.0),
      m_is_employed(false),
      m_current_wage(0.0),
      m_employer(""),
      m_hours_willing_to_work(40)
{
}

void Worker::OfferLabor(double wage_offered, const std::string &employer)
{
    if (wage_offered >= m_min_acceptable_wage)
    {
        AcceptJob(wage_offered, employer);
    }
    else
    {
        LoseJob();
    }
}

void Worker::AcceptJob(double wage, const std::string &employer)
{
    m_is_employed = true;
    m_current_wage = wage;
    m_employer = employer;
    m_monthly_income = wage * 4; // Simple: wage per week * 4
    m_wallet += m_monthly_income;
}

void Worker::LoseJob()
{
    m_is_employed = false;
    m_current_wage = 0.0;
    m_employer.clear();
    m_monthly_income = 0.0;
}

void Worker::RespondToMinimumWage(double government_min_wage)
{
    if (m_current_wage > 0.0 && m_current_wage < government_min_wage)
    {
        // If employer doesn't match, worker becomes unemployed
        LoseJob();
    }
}

void Worker::UpdateLaborSupply(double new_wage)
{
    m_current_wage = new_wage;

    // Simple labor supply response: higher wage => more hours offered up to 60
    if (new_wage > m_min_acceptable_wage)
    {
        m_hours_willing_to_work = std::min(60, 40 + static_cast<int>((new_wage - m_min_acceptable_wage) * 2));
    }
    else
    {
        m_hours_willing_to_work = 0;
    }
}

std::string Worker::GetInfoString() const
{
    std::ostringstream ss;
    ss << "Worker: " << m_name << " (ID=" << m_id << ")\n";
    ss << "Employed: " << (m_is_employed ? "Yes" : "No")
       << ", Wage: " << m_current_wage
       << ", Employer: " << (m_employer.empty() ? "None" : m_employer) << "\n";
    ss << "Skill: " << m_skill_level
    
       << ", Min Wage: " << m_min_acceptable_wage
       << ", Hours Offered: " << m_hours_willing_to_work << "\n";
    ss << "Wallet: " << m_wallet << ", Savings: " << m_savings << "\n";
    return ss.str();
}
