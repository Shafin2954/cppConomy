#include "MainFrame.h"
#include "DashboardPanel.h"
#include "EventLogPanel.h"
#include "CommandPanel.h"
#include "SelectionPanel.h"
#include "CommandsPanel.h"
#include "../core/Simulation.h"
#include "../commands/CommandExecutor.h"
#include "../utils/Config.h"
#include "../utils/Logger.h"

#include <wx/splitter.h>
#include <cctype>

wxBEGIN_EVENT_TABLE(MainFrame, wxFrame)
    EVT_CLOSE(MainFrame::OnClose)
        EVT_TIMER(ID_TIMER_UI, MainFrame::OnTimer)
            EVT_MENU(wxID_NEW, MainFrame::OnFileNew)
                EVT_MENU(wxID_SAVE, MainFrame::OnFileExport)
                    EVT_MENU(wxID_EXIT, MainFrame::OnFileExit)
                        EVT_MENU(ID_SIM_RESET, MainFrame::OnSimReset)
                            EVT_MENU(wxID_ABOUT, MainFrame::OnHelpAbout)
                                wxEND_EVENT_TABLE()

                                    MainFrame::MainFrame()
    : wxFrame(nullptr, wxID_ANY, "CppConomy - Economic Simulator",
              wxDefaultPosition, wxSize(1200, 800)),
      m_uiTimer(this, ID_TIMER_UI)
{
    // Initialize simulation
    m_simulation = std::make_unique<Simulation>();
    m_executor = std::make_unique<CommandExecutor>(*m_simulation);

    SetupTheme();
    CreateMenuBar();
    CreatePanels();
    CreateStatusBar();
    BindEvents();
    Bind(wxEVT_CHAR_HOOK, &MainFrame::OnGlobalKeyDown, this);

    // Initialize the simulation
    m_simulation->Initialize(Config::INITIAL_POPULATION, Config::INITIAL_FIRMS, 0);

    // Update initial display
    if (m_dashboard)
    {
        m_dashboard->UpdateStats(m_simulation->GetStats());
    }

    // Start UI update timer
    m_uiTimer.Start(Config::GUI_UPDATE_INTERVAL);

    Centre();
}

MainFrame::~MainFrame()
{
    m_uiTimer.Stop();
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

    // Main horizontal splitter (content | commands panel)
    wxSplitterWindow *mainSplitter = new wxSplitterWindow(this, wxID_ANY,
                                                          wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE | wxSP_3DSASH);
    mainSplitter->SetBackgroundColour(wxColour(255, 255, 255));

    // Left panel container
    wxPanel *leftContainer = new wxPanel(mainSplitter);
    leftContainer->SetBackgroundColour(wxColour(255, 255, 255));

    // Vertical sizer for left side
    wxBoxSizer *leftSizer = new wxBoxSizer(wxVERTICAL);

    // Top section: Dashboard
    m_dashboard = new DashboardPanel(leftContainer);
    leftSizer->Add(m_dashboard, 0, wxEXPAND | wxALL, 5);

    // Middle section: Selection Panel (showing selected entities)
    m_selectionPanel = new SelectionPanel(leftContainer);
    leftSizer->Add(m_selectionPanel, 0, wxEXPAND | wxLEFT | wxRIGHT, 5);

    // Event log panel (takes remaining space)
    m_eventLog = new EventLogPanel(leftContainer);
    leftSizer->Add(m_eventLog, 1, wxEXPAND | wxALL, 5);

    // Command panel at bottom
    m_commandPanel = new CommandPanel(leftContainer, m_executor.get());
    leftSizer->Add(m_commandPanel, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);

    leftContainer->SetSizer(leftSizer);

    // Commands panel on the right (replaces history)
    m_commandsPanel = new CommandsPanel(mainSplitter);
    m_commandsPanel->SetCommands(m_executor->getParser().getAvailableCommands());

    // Right sidebar takes 20% width, so left takes 80%
    int leftWidth = static_cast<int>(width * 0.80);
    mainSplitter->SplitVertically(leftContainer, m_commandsPanel, leftWidth);
    mainSplitter->SetMinimumPaneSize(150);

    // Main sizer
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(mainSplitter, 1, wxEXPAND);
    SetSizer(mainSizer);
}

void MainFrame::CreateStatusBar()
{
    wxStatusBar *statusBar = wxFrame::CreateStatusBar(2);
    int widths[] = {-2, -1};
    statusBar->SetStatusWidths(2, widths);
    statusBar->SetStatusText("Ready", 0);
    statusBar->SetStatusText("Population: 0", 1);
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

    // Set up simulation stats callback
    m_simulation->SetStatsCallback([this](const Simulation::EconomicStats &stats)
                                   { CallAfter([this, stats]()
                                               { OnStatsUpdate(stats); }); });

    // Connect command panel events
    if (m_commandPanel)
    {
        m_commandPanel->SetCommandCallback([this](const wxString &cmd)
                                           { OnCommandExecuted(cmd); });
    }

    // Connect commands panel click callback
    if (m_commandsPanel)
    {
        m_commandsPanel->SetCommandClickCallback([this](const wxString &cmd)
                                                 { OnCommandClicked(cmd); });
    }

    // Set up command executor output
    m_executor->setOutputCallback([this](const std::string &msg)
                                  {
        if (m_eventLog) {
            CallAfter([this, msg]() {
                LogEntry entry;
                entry.level = LogLevel::INFO;
                entry.message = msg;
                m_eventLog->AddEntry(entry);
                
                // Update selection panel after command execution
                if (m_selectionPanel) {
                    m_selectionPanel->UpdateSelection(m_simulation.get());
                }
            });
        } });
}

void MainFrame::OnClose(wxCloseEvent &event)
{
    m_uiTimer.Stop();
    event.Skip();
}

void MainFrame::OnTimer(wxTimerEvent &event)
{
    // Update status bar
    const auto &stats = m_simulation->GetStats();
    SetStatusText("Manual", 0);
    SetStatusText(wxString::Format("Population: %d | Firms: %d",
                                   stats.population, stats.firms),
                  1);

    // Update selection panel
    if (m_selectionPanel)
    {
        m_selectionPanel->UpdateSelection(m_simulation.get());
    }
}

void MainFrame::OnCommandExecuted(const wxString &command)
{
    // Update selection panel after command execution
    if (m_selectionPanel)
    {
        m_selectionPanel->UpdateSelection(m_simulation.get());
    }
}

void MainFrame::OnCommandClicked(const wxString &command)
{
    // Insert the clicked command into the command panel
    if (m_commandPanel)
    {
        m_commandPanel->SetCommandText(command);
    }
}

void MainFrame::OnGlobalKeyDown(wxKeyEvent &event)
{
    if (!m_commandPanel)
    {
        event.Skip();
        return;
    }

    wxTextCtrl *input = m_commandPanel->GetInputCtrl();
    if (!input)
    {
        event.Skip();
        return;
    }

    if (FindFocus() == input)
    {
        event.Skip();
        return;
    }

    int keyCode = event.GetKeyCode();
    bool printable = (keyCode >= 32 && keyCode < 127) && std::isprint(static_cast<unsigned char>(keyCode));
    bool controlKey = (keyCode == WXK_BACK) || (keyCode == WXK_DELETE) || (keyCode == WXK_SPACE);

    if (printable || controlKey)
    {
        input->SetFocus();
        wxKeyEvent forwarded(event);
        wxPostEvent(input, forwarded);
        return;
    }

    event.Skip();
}

void MainFrame::OnStatsUpdate(const Simulation::EconomicStats &stats)
{
    if (m_dashboard)
    {
        m_dashboard->UpdateStats(stats);
    }
}

void MainFrame::OnFileNew(wxCommandEvent &event)
{
    m_simulation->Reset();
    m_simulation->Initialize(Config::INITIAL_POPULATION, Config::INITIAL_FIRMS, 0);
    if (m_eventLog)
    {
        m_eventLog->Clear();
    }
    if (m_selectionPanel)
    {
        m_selectionPanel->UpdateSelection(m_simulation.get());
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

void MainFrame::OnSimReset(wxCommandEvent &event)
{
    OnFileNew(event);
}

void MainFrame::OnHelpAbout(wxCommandEvent &event)
{
    wxMessageBox(
        "CppConomy - Economic Simulator\n\n"
        "A simplified economic simulation engine.\n\n"
        "Features:\n"
        "• Person, Worker, Farmer, Owner\n"
        "• Markets and basic policy tools\n"
        "• Interactive commands and dashboard\n\n"
        "Built with wxWidgets and C++17",
        "About CppConomy",
        wxOK | wxICON_INFORMATION,
        this);
}
