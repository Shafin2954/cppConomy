#include "EventLogPanel.h"

EventLogPanel::EventLogPanel(wxWindow *parent)
    : wxPanel(parent, wxID_ANY), m_colorInfo(30, 30, 30) // Dark gray
      ,
      m_colorWarning(255, 140, 0) // Orange
      ,
      m_colorAlert(220, 38, 38) // Red
      ,
      m_colorSuccess(16, 124, 16) // Green
{
    SetBackgroundColour(wxColour(255, 255, 255));
    CreateLayout();
}

void EventLogPanel::CreateLayout()
{
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    // Title
    wxStaticText *title = new wxStaticText(this, wxID_ANY, "EVENT LOG");
    wxFont titleFont = title->GetFont();
    titleFont.SetPointSize(10);
    titleFont.SetWeight(wxFONTWEIGHT_BOLD);
    title->SetFont(titleFont);
    title->SetForegroundColour(wxColour(100, 100, 100));
    mainSizer->Add(title, 0, wxALL, 5);

    // Text control for multiline log output
    m_textCtrl = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                                wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 | wxTE_AUTO_URL);

    m_textCtrl->SetBackgroundColour(wxColour(252, 252, 252));

    // Set font
    wxFont font(10, wxFONTFAMILY_MODERN, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
                false, "Consolas");
    m_textCtrl->SetFont(font);

    mainSizer->Add(m_textCtrl, 1, wxEXPAND | wxALL, 5);

    SetSizer(mainSizer);
}

void EventLogPanel::AddEntry(const LogEntry &entry)
{
    // Format the entry
    wxString levelStr = GetLevelString(entry.level);
    wxString tickStr = wxString::Format("[%d]", entry.tick);
    
    // Get the color for this log level
    wxColour color = GetLevelColor(entry.level);
    
    // Set text color for this entry
    wxTextAttr attr;
    attr.SetTextColour(color);
    
    // Get current position
    long insertPos = m_textCtrl->GetLastPosition();
    
    // Build the full message with newlines preserved
    wxString message = entry.message;
    
    // Format: [LEVEL] [tick] message
    wxString fullEntry = levelStr + " " + tickStr + " " + message + "\n";
    
    // Append the text
    m_textCtrl->AppendText(fullEntry);
    
    // Apply color to the newly added text
    m_textCtrl->SetStyle(insertPos, m_textCtrl->GetLastPosition(), attr);
    
    // Auto-scroll to bottom
    m_textCtrl->ShowPosition(m_textCtrl->GetLastPosition());
    
    // Limit log size to prevent memory issues (keep last ~50000 characters)
    if (m_textCtrl->GetLastPosition() > 50000)
    {
        m_textCtrl->Remove(0, 10000);
    }
}

void EventLogPanel::Clear()
{
    m_textCtrl->Clear();
}

wxString EventLogPanel::GetLevelString(LogLevel level) const
{
    switch (level)
    {
    case LogLevel::INFO:
        return "[INFO]";
    case LogLevel::WARNING:
        return "[WARN]";
    case LogLevel::ALERT:
        return "[ALERT]";
    case LogLevel::SUCCESS:
        return "[OK]";
    default:
        return "[?]";
    }
}

wxColour EventLogPanel::GetLevelColor(LogLevel level) const
{
    switch (level)
    {
    case LogLevel::INFO:
        return m_colorInfo;
    case LogLevel::WARNING:
        return m_colorWarning;
    case LogLevel::ALERT:
        return m_colorAlert;
    case LogLevel::SUCCESS:
        return m_colorSuccess;
    default:
        return m_colorInfo;
    }
}
