#pragma once

#include <wx/wx.h>
#include <wx/timer.h>
#include <memory>
#include "../core/Simulation.h"

class CommandExecutor;
class DashboardPanel;
class EventLogPanel;
class CommandPanel;
class SelectionPanel;
class CommandsPanel;

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
    void OnCommandExecuted(const wxString &command);
    void OnStatsUpdate(const Simulation::EconomicStats &stats);
    void OnCommandClicked(const wxString &command);
    void OnGlobalKeyDown(wxKeyEvent &event);

    // Menu handlers
    void OnFileNew(wxCommandEvent &event);
    void OnFileExport(wxCommandEvent &event);
    void OnFileExit(wxCommandEvent &event);
    void OnSimReset(wxCommandEvent &event);
    void OnHelpAbout(wxCommandEvent &event);

    // Core components
    std::unique_ptr<Simulation> m_simulation;
    std::unique_ptr<CommandExecutor> m_executor;

    // GUI panels
    DashboardPanel *m_dashboard;
    EventLogPanel *m_eventLog;
    CommandPanel *m_commandPanel;
    SelectionPanel *m_selectionPanel;
    CommandsPanel *m_commandsPanel;

    // Timers
    wxTimer m_uiTimer;

    // Menu IDs
    enum
    {
        ID_SIM_RESET = wxID_HIGHEST + 1,
        ID_TIMER_UI
    };

    wxDECLARE_EVENT_TABLE();
};
