#include "DashboardPanel.h"
#include <wx/statline.h>
#include <sstream>
#include <iomanip>
#include <cmath>

DashboardPanel::DashboardPanel(wxWindow *parent)
    : wxPanel(parent, wxID_ANY), m_colorPositive(16, 124, 16) // Green
      ,
      m_colorNegative(220, 38, 38) // Red
      ,
      m_colorNeutral(30, 30, 30) // Dark gray
{
    SetBackgroundColour(wxColour(250, 250, 250));
    CreateLayout();
}

void DashboardPanel::CreateLayout()
{
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    // Title
    wxStaticText *title = new wxStaticText(this, wxID_ANY, "ECONOMIC DASHBOARD");
    wxFont titleFont = title->GetFont();
    titleFont.SetPointSize(12);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    title->SetForegroundColour(wxColour(0, 120, 212));
    mainSizer->Add(title, 0, wxALL, 10);

    // Stats grid
    wxFlexGridSizer *gridSizer = new wxFlexGridSizer(4, 15, 25);

    auto addStatPair = [this, gridSizer](const wxString &label, wxStaticText *&valueLabel,
                                         const wxString &initialValue = "---")
    {
        wxStaticText *lbl = new wxStaticText(this, wxID_ANY, label);
        lbl->SetForegroundColour(wxColour(100, 100, 100));
        gridSizer->Add(lbl, 0, wxALIGN_LEFT);

        valueLabel = new wxStaticText(this, wxID_ANY, initialValue);
        wxFont font = valueLabel->GetFont();
        font.SetWeight(wxFONTWEIGHT_BOLD);
        valueLabel->SetFont(font);
        gridSizer->Add(valueLabel, 0, wxALIGN_LEFT);
    };

    // Row 1
    addStatPair("GDP:", m_lblGdp);
    addStatPair("Growth:", m_lblGdpGrowth);
    addStatPair("Inflation:", m_lblInflation);
    addStatPair("CPI:", m_lblCpi);

    // Row 2
    addStatPair("Unemployment:", m_lblUnemployment);
    addStatPair("Employed:", m_lblEmployed);
    addStatPair("Money Supply:", m_lblMoneySupply);
    addStatPair("Interest Rate:", m_lblInterestRate);

    // Row 3
    addStatPair("Gov Budget:", m_lblBudget);
    addStatPair("Gov Debt:", m_lblDebt);
    addStatPair("Gini Index:", m_lblGini);
    addStatPair("Population:", m_lblPopulation);

    mainSizer->Add(gridSizer, 0, wxALL | wxEXPAND, 10);

    SetSizer(mainSizer);
}

void DashboardPanel::UpdateStats(const Simulation::EconomicStats &stats)
{
    // GDP
    m_lblGdp->SetLabel(FormatMoney(stats.gdp));

    // GDP Growth
    m_lblGdpGrowth->SetLabel(FormatPercent(stats.gdpGrowth * 100, true));
    m_lblGdpGrowth->SetForegroundColour(GetChangeColor(stats.gdpGrowth));

    // Inflation
    m_lblInflation->SetLabel(FormatPercent(stats.inflation * 100));
    m_lblInflation->SetForegroundColour(
        stats.inflation > 0.03 ? m_colorNegative : stats.inflation < 0 ? m_colorNegative
                                                                       : m_colorNeutral);

    // CPI
    m_lblCpi->SetLabel(wxString::Format("%.1f", stats.cpi));

    // Unemployment
    m_lblUnemployment->SetLabel(FormatPercent(stats.unemployment * 100));
    m_lblUnemployment->SetForegroundColour(
        stats.unemployment > 0.06 ? m_colorNegative : stats.unemployment < 0.03 ? m_colorPositive
                                                                                : m_colorNeutral);

    // Employed
    m_lblEmployed->SetLabel(wxString::Format("%d / %d", stats.employed, stats.population));

    // Money Supply
    m_lblMoneySupply->SetLabel(FormatMoney(stats.moneySupply));

    // Interest Rate
    m_lblInterestRate->SetLabel(FormatPercent(stats.interestRate * 100));

    // Budget
    m_lblBudget->SetLabel(FormatMoney(stats.budget));
    m_lblBudget->SetForegroundColour(stats.budget >= 0 ? m_colorPositive : m_colorNegative);

    // Debt
    m_lblDebt->SetLabel(FormatMoney(stats.debt));
    m_lblDebt->SetForegroundColour(stats.debt > 0 ? m_colorNegative : m_colorNeutral);

    // Gini
    m_lblGini->SetLabel(wxString::Format("%.3f", stats.giniCoefficient));
    m_lblGini->SetForegroundColour(
        stats.giniCoefficient > 0.4 ? m_colorNegative : stats.giniCoefficient < 0.3 ? m_colorPositive
                                                                                    : m_colorNeutral);

    // Population
    m_lblPopulation->SetLabel(wxString::Format("%d (Firms: %d)",
                                               stats.population, stats.firms));

    Layout();
    Refresh();
}

wxString DashboardPanel::FormatMoney(double value) const
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1);

    double absValue = std::abs(value);
    wxString prefix = value < 0 ? "-" : "";

    if (absValue >= 1e12)
    {
        ss << "$" << (absValue / 1e12) << "T";
    }
    else if (absValue >= 1e9)
    {
        ss << "$" << (absValue / 1e9) << "B";
    }
    else if (absValue >= 1e6)
    {
        ss << "$" << (absValue / 1e6) << "M";
    }
    else if (absValue >= 1e3)
    {
        ss << "$" << (absValue / 1e3) << "K";
    }
    else
    {
        ss << "$" << absValue;
    }

    return prefix + wxString(ss.str());
}

wxString DashboardPanel::FormatPercent(double value, bool showSign) const
{
    wxString sign = "";
    if (showSign && value > 0)
    {
        sign = "+";
    }
    return wxString::Format("%s%.1f%%", sign, value);
}

wxColour DashboardPanel::GetChangeColor(double change) const
{
    if (change > 0.001)
        return m_colorPositive;
    if (change < -0.001)
        return m_colorNegative;
    return m_colorNeutral;
}
