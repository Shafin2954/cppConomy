#pragma once

#include <wx/wx.h>
#include <wx/listbox.h>
#include <vector>
#include <functional>

struct CommandInfo;

class CommandsPanel : public wxPanel
{
public:
    CommandsPanel(wxWindow *parent);

    void SetCommands(const std::vector<CommandInfo> &commands);
    void SetCommandClickCallback(std::function<void(const wxString &)> callback)
    {
        m_commandClickCallback = callback;
    }

private:
    void CreateLayout();
    void OnItemClick(wxCommandEvent &event);

    wxListBox *m_listBox;
    std::vector<wxString> m_commandTemplates;
    std::function<void(const wxString &)> m_commandClickCallback;

    wxDECLARE_EVENT_TABLE();
};
