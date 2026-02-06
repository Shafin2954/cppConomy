#include "SelectionPanel.h"
#include <sstream>
#include <iomanip>

SelectionPanel::SelectionPanel(wxWindow *parent)
    : wxPanel(parent, wxID_ANY),
      m_bgColor(248, 248, 248),
      m_headerColor(0, 120, 212),
      m_textColor(60, 60, 60)
{
    SetBackgroundColour(m_bgColor);
    CreateLayout();
}

void SelectionPanel::CreateLayout()
{
    wxBoxSizer *mainSizer = new wxBoxSizer(wxHORIZONTAL);

    auto createEntityBox = [this](const wxString &title, wxStaticText *&titleLabel, wxStaticText *&infoLabel) -> wxPanel *
    {
        wxPanel *box = new wxPanel(this, wxID_ANY);
        box->SetBackgroundColour(wxColour(255, 255, 255));

        wxBoxSizer *boxSizer = new wxBoxSizer(wxVERTICAL);

        titleLabel = new wxStaticText(box, wxID_ANY, title);
        wxFont titleFont = titleLabel->GetFont();
        titleFont.SetPointSize(9);
        titleFont.SetWeight(wxFONTWEIGHT_BOLD);
        titleLabel->SetFont(titleFont);
        titleLabel->SetForegroundColour(m_headerColor);
        boxSizer->Add(titleLabel, 0, wxALL, 5);

        infoLabel = new wxStaticText(box, wxID_ANY, "(none selected)");
        wxFont infoFont = infoLabel->GetFont();
        infoFont.SetPointSize(8);
        infoLabel->SetFont(infoFont);
        infoLabel->SetForegroundColour(m_textColor);
        boxSizer->Add(infoLabel, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

        box->SetSizer(boxSizer);
        return box;
    };

    // Create four boxes for worker, farmer, owner, market
    wxPanel *workerBox = createEntityBox("WORKER", m_workerTitle, m_workerInfo);
    wxPanel *farmerBox = createEntityBox("FARMER", m_farmerTitle, m_farmerInfo);
    wxPanel *ownerBox = createEntityBox("OWNER", m_ownerTitle, m_ownerInfo);
    wxPanel *marketBox = createEntityBox("MARKET", m_marketTitle, m_marketInfo);

    mainSizer->Add(workerBox, 1, wxEXPAND | wxALL, 3);
    mainSizer->Add(farmerBox, 1, wxEXPAND | wxALL, 3);
    mainSizer->Add(ownerBox, 1, wxEXPAND | wxALL, 3);
    mainSizer->Add(marketBox, 1, wxEXPAND | wxALL, 3);

    SetSizer(mainSizer);
}

void SelectionPanel::UpdateSelection(const Simulation *sim)
{
    if (!sim)
        return;

    // Update worker
    const Worker *w = sim->GetSelectedWorker();
    if (w)
    {
        m_workerTitle->SetLabel("WORKER: " + wxString(w->GetName()));
        m_workerInfo->SetLabel(FormatWorkerInfo(w));
    }
    else
    {
        m_workerTitle->SetLabel("WORKER");
        m_workerInfo->SetLabel("(none selected)");
    }

    // Update farmer
    const Farmer *f = sim->GetSelectedFarmer();
    if (f)
    {
        m_farmerTitle->SetLabel("FARMER: " + wxString(f->GetName()));
        m_farmerInfo->SetLabel(FormatFarmerInfo(f));
    }
    else
    {
        m_farmerTitle->SetLabel("FARMER");
        m_farmerInfo->SetLabel("(none selected)");
    }

    // Update owner
    const Owner *o = sim->GetSelectedOwner();
    if (o)
    {
        m_ownerTitle->SetLabel("OWNER: " + wxString(o->GetName()));
        m_ownerInfo->SetLabel(FormatOwnerInfo(o));
    }
    else
    {
        m_ownerTitle->SetLabel("OWNER");
        m_ownerInfo->SetLabel("(none selected)");
    }

    // Update market
    const Market *m = sim->GetSelectedMarket();
    if (m)
    {
        m_marketTitle->SetLabel("MARKET: " + wxString(m->GetProductName()));
        m_marketInfo->SetLabel(FormatMarketInfo(m));
    }
    else
    {
        m_marketTitle->SetLabel("MARKET");
        m_marketInfo->SetLabel("(none selected)");
    }

    Layout();
    Refresh();
}

wxString SelectionPanel::FormatWorkerInfo(const Worker *w)
{
    if (!w)
        return "(none)";
    std::stringstream ss;
    ss << "Income: $" << std::fixed << std::setprecision(0) << w->GetMonthlyIncome() << "\n";
    ss << "Wage: $" << std::fixed << std::setprecision(0) << w->GetCurrentWage() << "\n";
    ss << "Skill: " << std::fixed << std::setprecision(2) << w->GetSkillLevel() << "\n";
    ss << "Employed: " << (w->IsEmployed() ? "Yes" : "No");
    return wxString(ss.str());
}

wxString SelectionPanel::FormatFarmerInfo(const Farmer *f)
{
    if (!f)
        return "(none)";
    std::stringstream ss;
    ss << "Crop: " << f->GetCrop() << "\n";
    ss << "Land: " << std::fixed << std::setprecision(1) << f->GetLandSize() << " acres\n";
    ss << "Output: " << std::fixed << std::setprecision(0) << f->GetOutputQuantity() << "\n";
    ss << "Profit: $" << std::fixed << std::setprecision(0) << f->GetProfit();
    return wxString(ss.str());
}

wxString SelectionPanel::FormatOwnerInfo(const Owner *o)
{
    if (!o)
        return "(none)";
    std::stringstream ss;
    ss << "Product: " << o->GetProductType() << "\n";
    ss << "Capital: $" << std::fixed << std::setprecision(0) << o->GetCapital() << "\n";
    ss << "Output: " << std::fixed << std::setprecision(0) << o->GetProduction() << "\n";
    ss << "Profit: $" << std::fixed << std::setprecision(0) << o->GetProfit();
    return wxString(ss.str());
}

wxString SelectionPanel::FormatMarketInfo(const Market *m)
{
    if (!m)
        return "(none)";
    std::stringstream ss;
    ss << "Price: $" << std::fixed << std::setprecision(2) << m->GetCurrentPrice() << "\n";
    ss << "Demand: " << std::fixed << std::setprecision(0) << m->GetQuantityDemanded() << "\n";
    ss << "Supply: " << std::fixed << std::setprecision(0) << m->GetQuantitySupplied() << "\n";
    ss << "Tax: " << std::fixed << std::setprecision(1) << (m->GetTaxRate() * 100) << "%";
    return wxString(ss.str());
}
