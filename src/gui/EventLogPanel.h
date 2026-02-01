#pragma once

#include <wx/wx.h>
#include "../utils/Logger.h"

class EventLogPanel : public wxPanel
{
public:
    EventLogPanel(wxWindow *parent);

    void AddEntry(const LogEntry &entry);
    void Clear();

private:
    void CreateLayout();
    wxString GetLevelString(LogLevel level) const;
    wxColour GetLevelColor(LogLevel level) const;

    wxTextCtrl *m_textCtrl;

    // Colors
    wxColour m_colorInfo;
    wxColour m_colorWarning;
    wxColour m_colorAlert;
    wxColour m_colorSuccess;
};
