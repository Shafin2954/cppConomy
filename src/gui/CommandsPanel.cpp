#include "CommandsPanel.h"
#include "../commands/CommandParser.h"

wxBEGIN_EVENT_TABLE(CommandsPanel, wxPanel)
    wxEND_EVENT_TABLE()

        CommandsPanel::CommandsPanel(wxWindow *parent)
    : wxPanel(parent, wxID_ANY)
{
    SetBackgroundColour(wxColour(248, 248, 248));
    CreateLayout();
}

void CommandsPanel::CreateLayout()
{
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    // Title
    wxStaticText *title = new wxStaticText(this, wxID_ANY, "AVAILABLE COMMANDS");
    wxFont titleFont = title->GetFont();
    titleFont.SetPointSize(10);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    title->SetForegroundColour(wxColour(100, 100, 100));
    mainSizer->Add(title, 0, wxALL, 10);

    // Subtitle
    wxStaticText *subtitle = new wxStaticText(this, wxID_ANY, "Click to insert into command line");
    wxFont subFont = subtitle->GetFont();
    subFont.SetPointSize(8);
    subtitle->SetFont(subFont);
    subtitle->SetForegroundColour(wxColour(150, 150, 150));
    mainSizer->Add(subtitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    // List box
    m_listBox = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                              0, nullptr, wxLB_SINGLE | wxBORDER_NONE);

    m_listBox->SetBackgroundColour(wxColour(252, 252, 252));

    wxFont listFont(9, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
                    false, "Consolas");
    m_listBox->SetFont(listFont);
    m_listBox->SetForegroundColour(wxColour(60, 60, 60));

    // Bind click event
    m_listBox->Bind(wxEVT_LISTBOX, &CommandsPanel::OnItemClick, this);

    mainSizer->Add(m_listBox, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
}

void CommandsPanel::SetCommands(const std::vector<CommandInfo> &commands)
{
    m_listBox->Clear();
    m_commandTemplates.clear();

    for (const auto &cmd : commands)
    {
        // Create display text
        wxString display = wxString(cmd.name);

        // Create command template
        wxString templ = wxString(cmd.name) + "(";
        if (!cmd.parameters.empty())
        {
            for (size_t i = 0; i < cmd.parameters.size(); ++i)
            {
                templ += wxString(cmd.parameters[i].first);
                if (i < cmd.parameters.size() - 1)
                    templ += ", ";
            }
        }
        templ += ")";

        m_listBox->Append(display);
        m_commandTemplates.push_back(templ);
    }
}

void CommandsPanel::OnItemClick(wxCommandEvent &event)
{
    int selection = m_listBox->GetSelection();
    if (selection != wxNOT_FOUND && selection < static_cast<int>(m_commandTemplates.size()))
    {
        if (m_commandClickCallback)
        {
            m_commandClickCallback(m_commandTemplates[selection]);
        }
    }
}
