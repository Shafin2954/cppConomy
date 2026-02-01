#pragma once

#include <wx/wx.h>
#include <wx/timer.h>
#include <memory>

class Economy;
class CommandExecutor;
class DashboardPanel;
class EventLogPanel;
class CommandPanel;
class HistoryPanel;

class MainFrame : public wxFrame
{
public:
    MainFrame();
    ~MainFrame();

private:
    void CreateMenuBar();
    void CreatePanels();
    void CreateStatusBar();
    void SetupTheme();
    void BindEvents();

    // Event handlers
    void OnClose(wxCloseEvent &event);
    void OnTimer(wxTimerEvent &event);
    void OnSimulationTick(wxTimerEvent &event);
    void OnCommandExecuted(const wxString &command);
    void OnStatsUpdate(const struct EconomicStats &stats);

    // Menu handlers
    void OnFileNew(wxCommandEvent &event);
    void OnFileExport(wxCommandEvent &event);
    void OnFileExit(wxCommandEvent &event);
    void OnSimStart(wxCommandEvent &event);
    void OnSimPause(wxCommandEvent &event);
    void OnSimReset(wxCommandEvent &event);
    void OnHelpAbout(wxCommandEvent &event);

    // Core components
    std::unique_ptr<Economy> m_economy;
    std::unique_ptr<CommandExecutor> m_executor;

    // GUI panels
    DashboardPanel *m_dashboard;
    EventLogPanel *m_eventLog;
    CommandPanel *m_commandPanel;
    HistoryPanel *m_historyPanel;

    // Timers
    wxTimer m_uiTimer;
    wxTimer m_simTimer;

    // Menu IDs
    enum
    {
        ID_SIM_START = wxID_HIGHEST + 1,
        ID_SIM_PAUSE,
        ID_SIM_RESET,
        ID_TIMER_UI,
        ID_TIMER_SIM
    };

    wxDECLARE_EVENT_TABLE();
};
