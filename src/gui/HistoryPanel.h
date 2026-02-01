#pragma once

#include <wx/wx.h>
#include <wx/listbox.h>
#include <vector>

class HistoryPanel : public wxPanel
{
public:
    HistoryPanel(wxWindow *parent);

    void AddCommand(const wxString &command);
    void Clear();
    const std::vector<wxString> &GetHistory() const { return m_history; }

private:
    void CreateLayout();
    void OnItemDoubleClick(wxCommandEvent &event);

    wxListBox *m_listBox;
    std::vector<wxString> m_history;

    wxDECLARE_EVENT_TABLE();
};
