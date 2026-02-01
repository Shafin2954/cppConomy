#pragma once

#include <wx/wx.h>
#include "../core/Economy.h"

class DashboardPanel : public wxPanel
{
public:
    DashboardPanel(wxWindow *parent);

    void UpdateStats(const EconomicStats &stats);

private:
    void CreateLayout();
    wxString FormatMoney(double value) const;
    wxString FormatPercent(double value, bool showSign = false) const;
    wxColour GetChangeColor(double change) const;

    // Stat labels
    wxStaticText *m_lblGdp;
    wxStaticText *m_lblGdpGrowth;
    wxStaticText *m_lblInflation;
    wxStaticText *m_lblCpi;
    wxStaticText *m_lblUnemployment;
    wxStaticText *m_lblEmployed;
    wxStaticText *m_lblMoneySupply;
    wxStaticText *m_lblInterestRate;
    wxStaticText *m_lblBudget;
    wxStaticText *m_lblDebt;
    wxStaticText *m_lblGini;
    wxStaticText *m_lblPopulation;
    wxStaticText *m_lblFirms;

    // Colors
    wxColour m_colorPositive;
    wxColour m_colorNegative;
    wxColour m_colorNeutral;
};
