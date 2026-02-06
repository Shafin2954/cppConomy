#pragma once

#include <wx/wx.h>
#include "../core/Simulation.h"

class SelectionPanel : public wxPanel
{
public:
    SelectionPanel(wxWindow *parent);

    void UpdateSelection(const Simulation *sim);

private:
    void CreateLayout();
    wxString FormatWorkerInfo(const Worker *w);
    wxString FormatFarmerInfo(const Farmer *f);
    wxString FormatOwnerInfo(const Owner *o);
    wxString FormatMarketInfo(const Market *m);

    wxStaticText *m_workerTitle;
    wxStaticText *m_workerInfo;
    wxStaticText *m_farmerTitle;
    wxStaticText *m_farmerInfo;
    wxStaticText *m_ownerTitle;
    wxStaticText *m_ownerInfo;
    wxStaticText *m_marketTitle;
    wxStaticText *m_marketInfo;

    wxColour m_bgColor;
    wxColour m_headerColor;
    wxColour m_textColor;
};
