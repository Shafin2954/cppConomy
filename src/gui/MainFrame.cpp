#include "MainFrame.h"
#include "DashboardPanel.h"
#include "EventLogPanel.h"
#include "CommandPanel.h"
#include "HistoryPanel.h"
#include "../core/Economy.h"
#include "../commands/CommandExecutor.h"
#include "../utils/Config.h"
#include "../utils/Logger.h"

#include <wx/splitter.h>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_CLOSE(MainFrame::OnClose)
        EVT_TIMER(ID_TIMER_UI, MainFrame::OnTimer)
            EVT_TIMER(ID_TIMER_SIM, MainFrame::OnSimulationTick)
                EVT_MENU(wxID_NEW, MainFrame::OnFileNew)
                    EVT_MENU(wxID_SAVE, MainFrame::OnFileExport)
                        EVT_MENU(wxID_EXIT, MainFrame::OnFileExit)
                            EVT_MENU(ID_SIM_START, MainFrame::OnSimStart)
                                EVT_MENU(ID_SIM_PAUSE, MainFrame::OnSimPause)
                                    EVT_MENU(ID_SIM_RESET, MainFrame::OnSimReset)
                                        EVT_MENU(wxID_ABOUT, MainFrame::OnHelpAbout)
                                            wxEND_EVENT_TABLE()

                                                MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "CppConomy - Economic Simulator",
              wxDefaultPosition, wxSize(1200, 800)),
      m_uiTimer(this, ID_TIMER_UI), m_simTimer(this, ID_TIMER_SIM)
{
    // Initialize economy
    m_economy = std::make_unique<Economy>();
    m_executor = std::make_unique<CommandExecutor>(*m_economy);

    SetupTheme();
    CreateMenuBar();
    CreatePanels();
    CreateStatusBar();
    BindEvents();

    // Initialize the economy
    m_economy->initialize(Config::INITIAL_POPULATION, Config::INITIAL_FIRMS);

    // Update initial display
    if (m_dashboard)
    {
        m_dashboard->UpdateStats(m_economy->getStats());
    }

    // Start UI update timer
    m_uiTimer.Start(Config::GUI_UPDATE_INTERVAL);

    Centre();
}

MainFrame::~MainFrame()
{
    m_uiTimer.Stop();
    m_simTimer.Stop();
}

void MainFrame::SetupTheme()
{
    // Light mode colors
    SetBackgroundColour(wxColour(255, 255, 255));
}

void MainFrame::CreateMenuBar()
{
    wxMenuBar *menuBar = new wxMenuBar();

    // File menu
    wxMenu *fileMenu = new wxMenu();
    fileMenu->Append(wxID_NEW, "&New Simulation\tCtrl+N", "Start a new simulation");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_SAVE, "&Export Data\tCtrl+S", "Export statistics to CSV");
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, "E&xit\tAlt+F4", "Exit the application");
    menuBar->Append(fileMenu, "&File");

    // Simulation menu
    wxMenu *simMenu = new wxMenu();
    simMenu->Append(ID_SIM_START, "&Start\tF5", "Start simulation");
    simMenu->Append(ID_SIM_PAUSE, "&Pause\tF6", "Pause simulation");
    simMenu->AppendSeparator();
    simMenu->Append(ID_SIM_RESET, "&Reset\tCtrl+R", "Reset simulation");
    menuBar->Append(simMenu, "&Simulation");

    // Help menu
    wxMenu *helpMenu = new wxMenu();
    helpMenu->Append(wxID_ABOUT, "&About", "About CppConomy");
    menuBar->Append(helpMenu, "&Help");

    SetMenuBar(menuBar);
}

void MainFrame::CreatePanels()
{
    // Get client size for proportional calculations
    int width = 1200; // Default width
    int height = 800; // Default height

    // Main horizontal splitter (content | history)
    wxSplitterWindow *mainSplitter = new wxSplitterWindow(this, wxID_ANY,
                                                          wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH);
    mainSplitter->SetBackgroundColour(wxColour(255, 255, 255));

    // Left panel container
    wxPanel *leftContainer = new wxPanel(mainSplitter);
    leftContainer->SetBackgroundColour(wxColour(255, 255, 255));

    // Vertical splitter for left side (dashboard/log | command)
    wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);

    // Top section: Dashboard and Event Log
    wxSplitterWindow *topSplitter = new wxSplitterWindow(leftContainer, wxID_ANY,
                                                         wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH);
    topSplitter->SetBackgroundColour(wxColour(255, 255, 255));

    // Dashboard panel
    m_dashboard = new DashboardPanel(topSplitter);

    // Event log panel
    m_eventLog = new EventLogPanel(topSplitter);

    // Top section takes 40% of height (dashboard gets 40% of top section)
    int topHeight = static_cast<int>(height * 0.40);
    topSplitter->SplitHorizontally(m_dashboard, m_eventLog, topHeight);
    topSplitter->SetMinimumPaneSize(100);

    leftSizer->Add(topSplitter, 1, wxEXPAND | wxALL, 5);

    // Command panel at bottom
    m_commandPanel = new CommandPanel(leftContainer, m_executor.get());
    leftSizer->Add(m_commandPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    leftContainer->SetSizer(leftSizer);

    // History panel on the right
    m_historyPanel = new HistoryPanel(mainSplitter);

    // Right sidebar takes 20% width, so left takes 80%
    int leftWidth = static_cast<int>(width * 0.80);
    mainSplitter->SplitVertically(leftContainer, m_historyPanel, leftWidth);
    mainSplitter->SetMinimumPaneSize(150);

    // Main sizer
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(mainSplitter, 1, wxEXPAND);
    SetSizer(mainSizer);
}

void MainFrame::CreateStatusBar()
{
    wxStatusBar *statusBar = wxFrame::CreateStatusBar(3);
    int widths[] = {-2, -1, -1};
    statusBar->SetStatusWidths(3, widths);
    statusBar->SetStatusText("Ready", 0);
    statusBar->SetStatusText("Population: 0", 1);
    statusBar->SetStatusText("Tick: 0", 2);
}

void MainFrame::BindEvents()
{
    // Set up logger callback
    Logger::getInstance().setCallback([this](const LogEntry &entry)
                                      {
        if (m_eventLog) {
            // Use CallAfter to ensure thread safety
            CallAfter([this, entry]() {
                m_eventLog->AddEntry(entry);
            });
        } });

    // Set up economy stats callback
    m_economy->setStatsCallback([this](const EconomicStats &stats)
                                { CallAfter([this, stats]()
                                            { OnStatsUpdate(stats); }); });

    // Connect command panel events
    if (m_commandPanel)
    {
        m_commandPanel->SetCommandCallback([this](const wxString &cmd)
                                           { OnCommandExecuted(cmd); });
    }

    // Set up command executor output
    m_executor->setOutputCallback([this](const std::string &msg)
                                  {
        if (m_eventLog) {
            CallAfter([this, msg]() {
                LogEntry entry;
                entry.level = LogLevel::INFO;
                entry.message = msg;
                entry.tick = m_economy->getCurrentTick();
                m_eventLog->AddEntry(entry);
            });
        } });
}

void MainFrame::OnClose(wxCloseEvent &event)
{
    m_uiTimer.Stop();
    m_simTimer.Stop();
    event.Skip();
}

void MainFrame::OnTimer(wxTimerEvent &event)
{
    // Update status bar
    const auto &stats = m_economy->getStats();
    SetStatusText(m_economy->isRunning() ? "Running" : "Paused", 0);
    SetStatusText(wxString::Format("Population: %d | Firms: %d",
                                   stats.population, stats.firms),
                  1);
    SetStatusText(wxString::Format("Tick: %d (Year %d, Month %d)",
                                   stats.currentTick,
                                   stats.currentTick / Config::TICKS_PER_YEAR,
                                   (stats.currentTick % Config::TICKS_PER_YEAR) + 1),
                  2);
}

void MainFrame::OnSimulationTick(wxTimerEvent &event)
{
    if (m_economy && m_economy->isRunning())
    {
        m_economy->tick();
    }
}

void MainFrame::OnCommandExecuted(const wxString &command)
{
    if (m_historyPanel)
    {
        m_historyPanel->AddCommand(command);
    }
}

void MainFrame::OnStatsUpdate(const EconomicStats &stats)
{
    if (m_dashboard)
    {
        m_dashboard->UpdateStats(stats);
    }
}

void MainFrame::OnFileNew(wxCommandEvent &event)
{
    m_simTimer.Stop();
    m_economy->reset();
    m_economy->initialize(Config::INITIAL_POPULATION, Config::INITIAL_FIRMS);
    if (m_eventLog)
    {
        m_eventLog->Clear();
    }
    if (m_historyPanel)
    {
        m_historyPanel->Clear();
    }
}

void MainFrame::OnFileExport(wxCommandEvent &event)
{
    wxFileDialog saveDialog(this, "Export Data", "", "economy_data.csv",
                            "CSV files (*.csv)|*.csv", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveDialog.ShowModal() == wxID_OK)
    {
        std::string filename = saveDialog.GetPath().ToStdString();
        Command cmd;
        cmd.name = "export_data";
        cmd.params["filename"] = filename;
        cmd.valid = true;
        m_executor->execute(cmd);
    }
}

void MainFrame::OnFileExit(wxCommandEvent &event)
{
    Close(true);
}

void MainFrame::OnSimStart(wxCommandEvent &event)
{
    m_economy->setRunning(true);
    m_simTimer.Start(Config::SIMULATION_TICK_INTERVAL);
    LOG_INFO("Simulation started");
}

void MainFrame::OnSimPause(wxCommandEvent &event)
{
    m_economy->setRunning(false);
    m_simTimer.Stop();
    LOG_INFO("Simulation paused");
}

void MainFrame::OnSimReset(wxCommandEvent &event)
{
    OnFileNew(event);
}

void MainFrame::OnHelpAbout(wxCommandEvent &event)
{
    wxMessageBox(
        "CppConomy - Economic Simulator\n\n"
        "A real-time object-oriented economic simulation engine.\n\n"
        "Features:\n"
        "• Agent-based modeling (Citizens & Firms)\n"
        "• Phillips Curve, Okun's Law, Quantity Theory\n"
        "• Government policy controls\n"
        "• Stochastic economic shocks\n\n"
        "Built with wxWidgets and C++17",
        "About CppConomy",
        wxOK | wxICON_INFORMATION,
        this);
}
