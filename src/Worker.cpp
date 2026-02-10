#include "Worker.h"
#include <sstream>
#include <algorithm>
#include "Logger.h"

using namespace std;

Worker::Worker(int id, const string &name, double initial_income, double skill)
    : Consumer(id, name, initial_income),
      skill_level(skill),
      min_acceptable_wage(10.0),
      is_employed(false),
      current_wage(0.0),
      employer(""),
      hours_willing_to_work(40)
{
}

void Worker::OfferLabor(double wage_offered, const string &employer)
{
    if (wage_offered >= min_acceptable_wage)
    {
        AcceptJob(wage_offered, employer);
    }
    else
    {
        LoseJob();
    }
}

void Worker::AcceptJob(double wage, const string &employer)
{
    is_employed = true;
    current_wage = wage;
    this->employer = employer;
    monthly_income = wage * 4; // Simple: wage per week * 4
    wallet += monthly_income;
}

void Worker::LoseJob()
{
    is_employed = false;
    current_wage = 0.0;
    employer.clear();
    monthly_income = 0.0;
}

void Worker::RespondToMinimumWage(double government_min_wage)
{
    if (current_wage > 0.0 && current_wage < government_min_wage)
    {
        // If employer doesn't match, worker becomes unemployed
        LoseJob();
    }
}

void Worker::UpdateLaborSupply(double new_wage)
{
    current_wage = new_wage;

    // Simple labor supply response: higher wage => more hours offered up to 60
    if (new_wage > min_acceptable_wage)
    {
        hours_willing_to_work = min(60, 40 + static_cast<int>((new_wage - min_acceptable_wage) * 2));
    }
    else
    {
        hours_willing_to_work = 0;
    }
}

// ========== Economic Decision Methods ==========

void Worker::DecideJobSearch()
{
    // Workers search for jobs when unemployed
    // Job search is costly (time, effort) so workers only search when
    // expected wage > reservation wage

    if (is_employed)
        return; // Already employed, no need to search

    // In a full simulation, this would:
    //   1. Search available job postings
    //   2. Apply to jobs with wage >= reservation wage
    //   3. Accept best offer if any
    //
    // For now, just log that worker is searching
    // The actual hiring happens when employers call OfferLabor()

    LOG_INFO("Worker " + name + " searching for work (reservation wage: " +
             to_string(min_acceptable_wage) + ")");
}

void Worker::UpdateReservationWage()
{
    // Reservation wage adjusts based on economic conditions
    // This demonstrates how unemployment duration affects wage demands

    if (!is_employed)
    {
        // Unemployed: gradually lower reservation wage (desperation effect)
        // Each period unemployed, worker becomes 2% more willing to accept lower wage
        min_acceptable_wage *= 0.98;

        // Floor: don't accept below $5/hour (survival constraint)
        min_acceptable_wage = max(5.0, min_acceptable_wage);

        LOG_INFO("Worker " + name + " lowered reservation wage to " +
                 to_string(min_acceptable_wage) + " due to unemployment");
    }
    else
    {
        // Employed: reservation wage = current wage
        // Workers won't switch jobs unless offered more
        min_acceptable_wage = current_wage;
    }
}

string Worker::GetInfoString() const
{
    ostringstream ss;
    ss << "Worker: " << name << " (ID=" << id << ")\n";
    ss << "Employed: " << (is_employed ? "Yes" : "No")
       << ", Wage: " << current_wage
       << ", Employer: " << (employer.empty() ? "None" : employer) << "\n";
    ss << "Skill: " << skill_level

       << ", Min Wage: " << min_acceptable_wage
       << ", Hours Offered: " << hours_willing_to_work << "\n";
    ss << "Wallet: " << wallet << ", Savings: " << savings << "\n";
    return ss.str();
}
