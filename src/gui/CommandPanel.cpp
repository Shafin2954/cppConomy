#include "CommandPanel.h"
#include "../commands/CommandParser.h"
#include "../commands/CommandExecutor.h"

wxBEGIN_EVENT_TABLE(CommandPanel, wxPanel)
    wxEND_EVENT_TABLE()

        CommandPanel::CommandPanel(wxWindow *parent, CommandExecutor *executor)
    : wxPanel(parent, wxID_ANY), m_executor(executor), m_parser(std::make_unique<CommandParser>()), m_historyIndex(-1), m_suggestionIndex(-1)
{
    SetBackgroundColour(wxColour(245, 245, 245));
    CreateLayout();
}

void CommandPanel::CreateLayout()
{
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    // Suggestions label
    m_suggestLabel = new wxStaticText(this, wxID_ANY, "");
    m_suggestLabel->SetForegroundColour(wxColour(100, 100, 100));
    wxFont suggestFont = m_suggestLabel->GetFont();
    suggestFont.SetPointSize(9);
    m_suggestLabel->SetFont(suggestFont);
    mainSizer->Add(m_suggestLabel, 0, wxLEFT | wxRIGHT | wxTOP, 10);

    // Input area
    wxBoxSizer *inputSizer = new wxBoxSizer(wxHORIZONTAL);

    // Prompt
    wxStaticText *prompt = new wxStaticText(this, wxID_ANY, " > ");
    wxFont promptFont(12, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD,
                      false, "Consolas");
    prompt->SetFont(promptFont);
    prompt->SetForegroundColour(wxColour(0, 120, 212));
    inputSizer->Add(prompt, 0, wxALIGN_CENTER_VERTICAL);

    // Text input
    m_textCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                                wxTE_PROCESS_ENTER | wxBORDER_NONE);
    wxFont inputFont(11, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
                     false, "Consolas");
    m_textCtrl->SetFont(inputFont);
    m_textCtrl->SetBackgroundColour(wxColour(255, 255, 255));
    m_textCtrl->SetForegroundColour(wxColour(30, 30, 30));

    // Bind events
    m_textCtrl->Bind(wxEVT_TEXT, &CommandPanel::OnTextChanged, this);
    m_textCtrl->Bind(wxEVT_KEY_DOWN, &CommandPanel::OnKeyDown, this);
    m_textCtrl->Bind(wxEVT_TEXT_ENTER, &CommandPanel::OnTextEnter, this);

    inputSizer->Add(m_textCtrl, 1, wxEXPAND | wxALL, 5);

    mainSizer->Add(inputSizer, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    // Hint label
    m_hintLabel = new wxStaticText(this, wxID_ANY,
                                   "Type a command (e.g., help(), set_price('cloth', 2)) | ↑↓ History | Tab Complete");
    m_hintLabel->SetForegroundColour(wxColour(150, 150, 150));
    wxFont hintFont = m_hintLabel->GetFont();
    hintFont.SetPointSize(8);
    m_hintLabel->SetFont(hintFont);
    mainSizer->Add(m_hintLabel, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    SetSizer(mainSizer);
}

void CommandPanel::OnTextChanged(wxCommandEvent &event)
{
    UpdateSuggestions();
    event.Skip();
}

void CommandPanel::OnKeyDown(wxKeyEvent &event)
{
    int keyCode = event.GetKeyCode();

    if (keyCode == WXK_UP)
    {
        // Navigate history up
        if (!m_history.empty() && m_historyIndex < static_cast<int>(m_history.size()) - 1)
        {
            m_historyIndex++;
            m_textCtrl->SetValue(m_history[m_history.size() - 1 - m_historyIndex]);
            m_textCtrl->SetInsertionPointEnd();
        }
    }
    else if (keyCode == WXK_DOWN)
    {
        // Navigate history down
        if (m_historyIndex > 0)
        {
            m_historyIndex--;
            m_textCtrl->SetValue(m_history[m_history.size() - 1 - m_historyIndex]);
            m_textCtrl->SetInsertionPointEnd();
        }
        else if (m_historyIndex == 0)
        {
            m_historyIndex = -1;
            m_textCtrl->Clear();
        }
    }
    else if (keyCode == WXK_TAB)
    {
        // Autocomplete
        if (!m_currentSuggestions.empty())
        {
            m_suggestionIndex = (m_suggestionIndex + 1) % m_currentSuggestions.size();
            ApplySuggestion(m_suggestionIndex);
        }
    }
    else if (keyCode == WXK_ESCAPE)
    {
        m_textCtrl->Clear();
        HideSuggestions();
    }
    else
    {
        event.Skip();
    }
}

void CommandPanel::OnTextEnter(wxCommandEvent &event)
{
    ExecuteCommand();
}

void CommandPanel::ExecuteCommand()
{
    wxString command = m_textCtrl->GetValue().Trim().Trim(false);

    if (command.IsEmpty())
    {
        return;
    }

    // Add to history
    m_history.push_back(command);
    m_historyIndex = -1;

    // Notify callback
    if (m_commandCallback)
    {
        m_commandCallback(command);
    }

    // Execute
    if (m_executor)
    {
        m_executor->execute(command.ToStdString());
    }

    // Clear input
    m_textCtrl->Clear();
    HideSuggestions();
}

void CommandPanel::UpdateSuggestions()
{
    wxString text = m_textCtrl->GetValue();

    if (text.IsEmpty())
    {
        HideSuggestions();
        return;
    }

    // Get command name (before parenthesis)
    wxString cmdName = text.BeforeFirst('(').Trim();

    // Get suggestions
    std::vector<std::string> suggestions = m_parser->getSuggestions(cmdName.ToStdString());

    if (suggestions.empty())
    {
        HideSuggestions();
        return;
    }

    m_currentSuggestions.clear();
    wxString suggestText = "Suggestions: ";

    for (size_t i = 0; i < suggestions.size() && i < 5; ++i)
    {
        m_currentSuggestions.push_back(wxString(suggestions[i]));
        if (i > 0)
            suggestText += " | ";
        suggestText += suggestions[i];
    }

    m_suggestLabel->SetLabel(suggestText);

    // Show parameter hints if command is matched
    if (suggestions.size() == 1 && text.Contains("("))
    {
        wxString hints = m_parser->getParameterHints(suggestions[0]);
        m_suggestLabel->SetLabel("Usage: " + hints);
    }

    Layout();
}

void CommandPanel::ShowSuggestions(const std::vector<wxString> &suggestions)
{
    m_currentSuggestions = suggestions;
    m_suggestionIndex = -1;
}

void CommandPanel::HideSuggestions()
{
    m_suggestLabel->SetLabel("");
    m_currentSuggestions.clear();
    m_suggestionIndex = -1;
}

void CommandPanel::ApplySuggestion(int index)
{
    if (index >= 0 && index < static_cast<int>(m_currentSuggestions.size()))
    {
        wxString suggestion = m_currentSuggestions[index];
        m_textCtrl->SetValue(suggestion + "(");
        m_textCtrl->SetInsertionPointEnd();
        UpdateSuggestions();
    }
}

void CommandPanel::SetCommandText(const wxString &text)
{
    m_textCtrl->SetValue(text);
    m_textCtrl->SetInsertionPointEnd();
    m_textCtrl->SetFocus();
    UpdateSuggestions();
}
