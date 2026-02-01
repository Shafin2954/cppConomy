#include "HistoryPanel.h"

wxBEGIN_EVENT_TABLE(HistoryPanel, wxPanel)
    wxEND_EVENT_TABLE()

        HistoryPanel::HistoryPanel(wxWindow *parent)
    : wxPanel(parent, wxID_ANY)
{
    SetBackgroundColour(wxColour(248, 248, 248));
    CreateLayout();
}

void HistoryPanel::CreateLayout()
{
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    // Title
    wxStaticText *title = new wxStaticText(this, wxID_ANY, "COMMAND HISTORY");
    wxFont titleFont = title->GetFont();
    titleFont.SetPointSize(10);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    title->SetForegroundColour(wxColour(100, 100, 100));
    mainSizer->Add(title, 0, wxALL, 10);

    // List box
    m_listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                              0, nullptr, wxLB_SINGLE | wxBORDER_NONE);

    m_listBox->SetBackgroundColour(wxColour(252, 252, 252));

    wxFont listFont(9, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
                    false, "Consolas");
    m_listBox->SetFont(listFont);
    m_listBox->SetForegroundColour(wxColour(60, 60, 60));

    // Bind double-click event
    m_listBox->Bind(wxEVT_LISTBOX_DCLICK, &HistoryPanel::OnItemDoubleClick, this);

    mainSizer->Add(m_listBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
}

void HistoryPanel::AddCommand(const wxString &command)
{
    m_history.push_back(command);

    // Format for display (truncate if too long)
    wxString display = "> " + command;
    if (display.Length() > 30)
    {
        display = display.Left(27) + "...";
    }

    m_listBox->Append(display);

    // Scroll to bottom
    m_listBox->SetSelection(m_listBox->GetCount() - 1);
    m_listBox->Deselect(m_listBox->GetSelection());

    // Limit history size
    while (m_listBox->GetCount() > 100)
    {
        m_listBox->Delete(0);
        m_history.erase(m_history.begin());
    }
}

void HistoryPanel::Clear()
{
    m_listBox->Clear();
    m_history.clear();
}

void HistoryPanel::OnItemDoubleClick(wxCommandEvent &event)
{
    int selection = m_listBox->GetSelection();
    if (selection != wxNOT_FOUND && selection < static_cast<int>(m_history.size()))
    {
        // Could emit an event to re-execute the command
        // For now, just select it
        wxString cmd = m_history[selection];
        // TODO: Send to command panel
    }
}
