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
    void ShowCommandSuggestions(const string &currentInput);

    // Display help text
    void ShowHelp();

    // Process a single command line
    void ProcessCommand(const string &input);

    // Custom output callback for CommandExecutor
    void HandleOutput(const string &message);

    // Display simulation status in a styled format
    void ShowStatus();

    // Display dashboard with economic stats
    void ShowDashboard();

    // Read a line of input with command history support
    string ReadLine();

    // Read a line with autocomplete support (like Claude CLI)
    string ReadLineWithAutocomplete();

    // Get autocomplete suggestions for current input
    vector<string> GetSuggestions(const string &input);

    // Add to command history
    void AddToHistory(const string &command);

    // Clear the screen
    void ClearScreen();

    // Format entity info for status box
    string FormatEntityBox(const string &title, const string &content, int width = 20);

    // Format a two-column layout box
    string FormatTwoColumnBox(const string &leftContent, const string &rightContent, int totalWidth = 120);

    // Move cursor to input line
    void MoveCursorToInput();

    // Clear input line
    void ClearInputLine();

    unique_ptr<Simulation> simulation;
    unique_ptr<CommandExecutor> executor;
    vector<string> history;
    size_t historyIndex = 0;
    bool running = true;

    // Available commands for autocomplete
    vector<string> availableCommands;

    // Initialize command list
    void InitializeCommands();
};
