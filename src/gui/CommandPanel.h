#pragma once

#include <wx/wx.h>
#include <vector>
#include <memory>
#include <functional>

class CommandExecutor;
class CommandParser;

class CommandPanel : public wxPanel
{
public:
    CommandPanel(wxWindow *parent, CommandExecutor *executor);

    void SetCommandCallback(std::function<void(const wxString &)> callback)
    {
        m_commandCallback = callback;
    }

    // Set text in the command input
    void SetCommandText(const wxString &text);

    // Access input control
    wxTextCtrl *GetInputCtrl() const { return m_textCtrl; }

private:
    void CreateLayout();
    void OnTextChanged(wxCommandEvent &event);
    void OnKeyDown(wxKeyEvent &event);
    void OnTextEnter(wxCommandEvent &event);
    void ExecuteCommand();
    void UpdateSuggestions();
    void ShowSuggestions(const std::vector<wxString> &suggestions);
    void HideSuggestions();
    void ApplySuggestion(int index);

    wxTextCtrl *m_textCtrl;
    wxStaticText *m_suggestLabel;
    wxStaticText *m_hintLabel;

    CommandExecutor *m_executor;
    std::unique_ptr<CommandParser> m_parser;

    std::vector<wxString> m_history;
    int m_historyIndex;

    std::vector<wxString> m_currentSuggestions;
    int m_suggestionIndex;

    std::function<void(const wxString &)> m_commandCallback;

    wxDECLARE_EVENT_TABLE();
};
