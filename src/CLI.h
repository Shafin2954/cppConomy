#pragma once

#include <string>
#include <vector>
#include <memory>
#include "Simulation.h"
#include "CommandExecutor.h"
#include "TermColors.h"

// ============================================================================
// CLI.h - Interactive command-line interface for cppConomy
//
// Features:
// - Stylistic prompts and formatting
// - Command history
// - Beautiful output formatting
// - REPL (Read-Eval-Print Loop)
// ============================================================================

class CLI
{
public:
    CLI();
    ~CLI() = default;

    // Main entry point - starts the REPL
    void Run();

private:
    // Display welcome banner
    void ShowBanner();

    // Display sticky header with two-column layout (Claude style)
    void ShowStickyHeader();

    // Display the main UI frame
    void ShowMainFrame();

    // Display command suggestions at bottom
    void ShowCommandSuggestions(const std::string &currentInput);

    // Display help text
    void ShowHelp();

    // Process a single command line
    void ProcessCommand(const std::string &input);

    // Custom output callback for CommandExecutor
    void HandleOutput(const std::string &message);

    // Display simulation status in a styled format
    void ShowStatus();

    // Display dashboard with economic stats
    void ShowDashboard();

    // Read a line of input with command history support
    std::string ReadLine();

    // Read a line with autocomplete support (like Claude CLI)
    std::string ReadLineWithAutocomplete();

    // Get autocomplete suggestions for current input
    std::vector<std::string> GetSuggestions(const std::string &input);

    // Add to command history
    void AddToHistory(const std::string &command);

    // Clear the screen
    void ClearScreen();

    // Format entity info for status box
    std::string FormatEntityBox(const std::string &title, const std::string &content, int width = 20);

    // Format a two-column layout box
    std::string FormatTwoColumnBox(const std::string &leftContent, const std::string &rightContent, int totalWidth = 120);

    // Move cursor to input line
    void MoveCursorToInput();

    // Clear input line
    void ClearInputLine();

    std::unique_ptr<Simulation> m_simulation;
    std::unique_ptr<CommandExecutor> m_executor;
    std::vector<std::string> m_history;
    size_t m_historyIndex = 0;
    bool m_running = true;

    // Available commands for autocomplete
    std::vector<std::string> m_availableCommands;

    // Initialize command list
    void InitializeCommands();
};
