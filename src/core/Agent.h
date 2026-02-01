#pragma once

// On Windows, we need to handle conflicts with Windows.h macros
// The issue is that wx/wx.h includes Windows.h which defines macros that
// conflict with our enum names. We handle this by using unique prefixed names.

#include <memory>
#include <string>

// Forward declarations
class Economy;

// EconomicSector enum - using CPPC prefix to avoid ALL Windows macro conflicts
// Windows defines: Industry, SERVICES, Tech, Health, etc. as macros
enum class EconomicSector
{
    CPPC_Agriculture,   // Agriculture
    CPPC_Manufacturing, // Manufacturing
    CPPC_Services,      // Services
    CPPC_Technology,    // Technology
    CPPC_Healthcare,    // Healthcare
    CPPC_Count          // Total count
};

inline const char *sectorToString(EconomicSector s)
{
    switch (s)
    {
    case EconomicSector::CPPC_Agriculture:
        return "Agriculture";
    case EconomicSector::CPPC_Manufacturing:
        return "Manufacturing";
    case EconomicSector::CPPC_Services:
        return "Services";
    case EconomicSector::CPPC_Technology:
        return "Technology";
    case EconomicSector::CPPC_Healthcare:
        return "Healthcare";
    default:
        return "Unknown";
    }
}

inline EconomicSector stringToSector(const std::string &s)
{
    if (s == "agriculture" || s == "Agricultural" || s == "agri")
        return EconomicSector::CPPC_Agriculture;
    if (s == "manufacturing" || s == "Manufacturing" || s == "manu")
        return EconomicSector::CPPC_Manufacturing;
    if (s == "services" || s == "Services" || s == "serv")
        return EconomicSector::CPPC_Services;
    if (s == "technology" || s == "Technology" || s == "tech")
        return EconomicSector::CPPC_Technology;
    if (s == "healthcare" || s == "Healthcare" || s == "health")
        return EconomicSector::CPPC_Healthcare;
    return EconomicSector::CPPC_Services; // default
}

// Type alias for backward compatibility
using Sector = EconomicSector;

class Agent
{
public:
    Agent(int id) : m_id(id), m_wealth(0.0), m_active(true) {}
    virtual ~Agent() = default;

    virtual void update(Economy &economy) = 0;

    int getId() const { return m_id; }
    double getWealth() const { return m_wealth; }
    bool isActive() const { return m_active; }

    void setWealth(double w) { m_wealth = w; }
    void addWealth(double w) { m_wealth += w; }
    void deactivate() { m_active = false; }

protected:
    int m_id;
    double m_wealth;
    bool m_active;
};
