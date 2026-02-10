#include "CLI.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace TermColors;
using namespace std;


CLI::CLI()
{
    // Initialize terminal colors
    TermColors::Init();

    // Create simulation and executor
    simulation = make_unique<Simulation>();
    executor = make_unique<CommandExecutor>(*simulation);

    // Set up output callback
    executor->setOutputCallback([this](const string &msg)
                                { HandleOutput(msg); });

    // Initialize simulation with default entities
    simulation->Initialize(2, 1, 1);
    simulation->RefreshStats();

    // Initialize command list for autocomplete
    InitializeCommands();
}

void CLI::Run()
{
    ClearScreen();
    ShowStickyHeader();

    while (running)
    {
        // Print separator and prompt
        cout << Styled(Repeat("─", 88), Theme::Muted) << "\n";
        cout << Prompt();
        cout << flush;

        // Read input with autocomplete
        string input = ReadLineWithAutocomplete();
        cout << "\n"; // Newline after input

        // Clear suggestion line
        cout << "\r" << string(88, ' ') << "\r";

        // Print bottom separator
        cout << Styled(Repeat("─", 88), Theme::Muted) << "\n";

        // Trim whitespace
        input.erase(0, input.find_first_not_of(" \t\n\r"));
        input.erase(input.find_last_not_of(" \t\n\r") + 1);

        // Show suggestions
        ShowCommandSuggestions(input);

        if (input.empty())
            continue;

        // Add to history
        AddToHistory(input);

        // Check for special commands
        if (input == "exit" || input == "quit")
        {
            cout << Styled("\n  Goodbye!\n", Theme::Success);
            running = false;
            break;
        }
        else if (input == "clear" || input == "cls")
        {
            ClearScreen();
            ShowStickyHeader();
            continue;
        }
        else if (input == "dashboard")
        {
            cout << "\n";
            ShowDashboard();
            continue;
        }
        else if (input == "refresh")
        {
            ClearScreen();
            ShowStickyHeader();
            continue;
        }

        // Process command
        cout << "\n";
        ProcessCommand(input);

        // Refresh header after commands that might change selections
        if (input.find("select") != string::npos)
        {
            ClearScreen();
            ShowStickyHeader();
        }
    }
}

void CLI::ShowBanner()
{
    stringstream banner;
    banner << R"(
   ____            ____                                 
  / ___|_ __  _ __/ ___|___  _ __   ___  _ __ ___  _   _ 
 | |   | '_ \| '_ \___ \/ _ \| '_ \ / _ \| '_ ` _ \| | | |
 | |___| |_) | |_) |__) | (_) | | | | (_) | | | | | | |_| |
  \____| .__/| .__/____/ \___/|_| |_|\___/|_| |_| |_|\__, |
       |_|   |_|                                      |___/ 
)";

    cout << Styled(banner.str(), Theme::Primary);
    cout << Styled("  Economic Simulation Engine", string(Color::Bold).append(Theme::Highlight).c_str()) << "\n";
    cout << Styled("  Version 2.0-CLI", Theme::Muted) << "\n\n";
}

void CLI::ShowStickyHeader()
{
    // Two-column header box
    stringstream left, right;

    //   _____          _____
    //  / ___/__  ___  / ___/__  ___  ___  __ _  __ __
    // / /__/ _ \/ _ \/ /__/ _ \/ _ \/ _ \/  ' \/ // /
    // \___/ .__/ .__/\___/\___/_//_/\___/_/_/_/\_, /
    //    /_/  /_/                             /___/
    // Left column - ASCII art
    left << "                                                \n";
    left << "   _____          _____                         \n";
    left << "  / ___/__  ___  / ___/__  ___  ___  __ _  __ __\n";
    left << " / /__/ _ \\/ _ \\/ /__/ _ \\/ _ \\/ _ \\/  ' \\/ // /\n";
    left << " \\___/ .__/ .__/\\___/\\___/_//_/\\___/_/_/_/\\_, /\n";
    left << "    /_/  /_/                             /___/\n";
    left << "                                                \n";
    left << "                                                \n";
    left << "                                                \n";

    // Right column - Tips and info
    right << "Economic Simulation v2.0       \n";
    right << "                               \n";
    right << "help       - Show all commands \n";
    right << "dashboard  - Economic stats    \n";
    right << "workers    - List all workers  \n";
    right << "farmers    - List all farmers  \n";
    right << "select     - Select entities   \n";
    right << "                               \n";

    cout << FormatTwoColumnBox(left.str(), right.str(), 90);

    // Entity status boxes
    Worker *selWorker = simulation->GetSelectedWorker();
    Farmer *selFarmer = simulation->GetSelectedFarmer();
    Owner *selOwner = simulation->GetSelectedOwner();
    Market *selMarket = simulation->GetSelectedMarket();

    string farmerBox = FormatEntityBox("FARMER",
                                       selFarmer ? selFarmer->GetName() + "\nLand: " + to_string((int)selFarmer->GetLandSize()) + " acres" : "None selected");

    string workerBox = FormatEntityBox("WORKER",
                                       selWorker ? selWorker->GetName() + "\nIncome: $" + to_string((int)selWorker->GetMonthlyIncome()) : "None selected");

    string ownerBox = FormatEntityBox("OWNER",
                                      selOwner ? selOwner->GetName() + "\nCapital: $" + to_string((int)selOwner->GetCapital()) : "None selected");

    string marketBox = FormatEntityBox("MARKET",
                                       selMarket ? selMarket->GetProductName() + "\nPrice: $" + to_string((int)selMarket->GetCurrentPrice()) : "None selected");

    // Split boxes into lines for side-by-side display
    auto splitLines = [](const string &text)
    {
        vector<string> lines;
        stringstream ss(text);
        string line;
        while (getline(ss, line))
        {
            lines.push_back(line);
        }
        return lines;
    };

    vector<string> farmerLines = splitLines(farmerBox);
    vector<string> workerLines = splitLines(workerBox);
    vector<string> ownerLines = splitLines(ownerBox);
    vector<string> marketLines = splitLines(marketBox);

    size_t maxLines = max(farmerLines.size(), max(workerLines.size(), max(ownerLines.size(), marketLines.size())));

    for (size_t i = 0; i < maxLines; ++i)
    {
        cout << (i < farmerLines.size() ? farmerLines[i] : string(23, ' '));
        cout << (i < workerLines.size() ? workerLines[i] : string(22, ' '));
        cout << (i < ownerLines.size() ? ownerLines[i] : string(22, ' '));
        cout << (i < marketLines.size() ? marketLines[i] : "");
        cout << "\n";
    }

    cout << "\n";
}

void CLI::ShowHelp()
{
    cout << Styled("Available Commands:", Theme::Highlight) << "\n";
    cout << "\n"
         << Styled("Entity Listing:", Theme::Highlight) << "\n";
    cout << "  " << Styled("persons", Theme::Info) << ", " << Styled("workers", Theme::Info) << ", " << Styled("farmers", Theme::Info) << ", " << Styled("owners", Theme::Info) << ", " << Styled("markets", Theme::Info) << "\n";

    cout << "\n"
         << Styled("Entity Details:", Theme::Highlight) << "\n";
    cout << "  " << Styled("person", Theme::Info) << "  - Get person details by name\n";
    cout << "  " << Styled("worker", Theme::Info) << "  - Get worker details by name\n";
    cout << "  " << Styled("farmer", Theme::Info) << "  - Get farmer details by name\n";
    cout << "  " << Styled("owner", Theme::Info) << "   - Get owner details by name\n";
    cout << "  " << Styled("market", Theme::Info) << "  - Get market details by name\n";

    cout << "\n"
         << Styled("Entity Creation:", Theme::Highlight) << "\n";
    cout << "  " << Styled("add_worker", Theme::Info) << "  - Add a worker\n";
    cout << "  " << Styled("add_farmer", Theme::Info) << "  - Add a farmer\n";
    cout << "  " << Styled("add_owner", Theme::Info) << "   - Add a business owner\n";
    cout << "  " << Styled("add_market", Theme::Info) << "  - Add a market\n";

    cout << "\n"
         << Styled("Actions:", Theme::Highlight) << "\n";
    cout << "  " << Styled("select", Theme::Info) << "         - Select entities to show\n";
    cout << "  " << Styled("clear_selection", Theme::Info) << " - Clear selection\n";
    cout << "  " << Styled("buy", Theme::Info) << "            - Worker buys product\n";
    cout << "  " << Styled("harvest", Theme::Info) << "        - Harvest crops\n";
    cout << "  " << Styled("reset", Theme::Info) << "         - Reset simulation\n";

    cout << "\n"
         << Styled("System:", Theme::Highlight) << "\n";
    cout << "  " << Styled("help", Theme::Info) << "      - Show this help\n";
    cout << "  " << Styled("status", Theme::Info) << "    - Show simulation status\n";
    cout << "  " << Styled("dashboard", Theme::Info) << " - View economic dashboard\n";
    cout << "  " << Styled("refresh", Theme::Info) << "   - Refresh screen\n";
    cout << "  " << Styled("clear", Theme::Info) << "     - Clear screen\n";
    cout << "  " << Styled("exit/quit", Theme::Info) << " - Exit program\n";
}

void CLI::ProcessCommand(const string &input)
{
    bool success = executor->execute(input);

    if (!success && !executor->getLastError().empty())
    {
        cout << Error("Command failed: " + executor->getLastError()) << "\n";
    }

    cout << "\n";
}

void CLI::HandleOutput(const string &message)
{
    // Check if it's an error message
    if (message.find("Error:") == 0)
    {
        cout << Error(message.substr(7)) << "\n";
    }
    // Check if it's a success/added message
    else if (message.find("Added") == 0 || message.find("complete") != string::npos ||
             message.find("cleared") != string::npos || message.find("updated") != string::npos)
    {
        cout << Success(message) << "\n";
    }
    // Check if it's a list or detailed output
    else if (message.find(":\n") != string::npos || message.find("---") != string::npos)
    {
        // Format as info box for structured data
        cout << Styled(message, Color::Reset) << "\n";
    }
    else
    {
        cout << message << "\n";
    }
}

void CLI::ShowStatus()
{
    string status = simulation->GetStatusString();
    cout << BoxedText(status, "Simulation Status") << "\n";
}

void CLI::ShowDashboard()
{
    auto stats = simulation->GetStats();

    stringstream ss;
    ss << fixed << setprecision(2);

    // Economic Indicators
    ss << Styled("ECONOMIC INDICATORS", Theme::Highlight) << "\n";
    ss << Styled(Repeat("─", 50), Theme::Muted) << "\n";
    ss << KeyValue("GDP", Styled("$" + to_string((int)stats.gdp), Theme::Success)) << "\n";
    ss << KeyValue("GDP Growth", Styled(to_string(stats.gdpGrowth) + "%", Theme::Info)) << "\n";
    ss << KeyValue("Inflation", Styled(to_string(stats.inflation) + "%", Theme::Warning)) << "\n";
    ss << KeyValue("CPI", to_string(stats.cpi)) << "\n";
    ss << "\n";

    // Labor Market
    ss << Styled("LABOR MARKET", Theme::Highlight) << "\n";
    ss << Styled(Repeat("─", 50), Theme::Muted) << "\n";
    ss << KeyValue("Population", Styled(to_string(stats.population), Theme::Info)) << "\n";
    ss << KeyValue("Employed", Styled(to_string(stats.employed), Theme::Success)) << "\n";
    ss << KeyValue("Unemployment Rate", Styled(to_string(stats.unemployment) + "%",
                                               stats.unemployment > 10 ? Theme::Error : Theme::Info))
       << "\n";
    ss << "\n";

    // Government & Finance
    ss << Styled("GOVERNMENT & FINANCE", Theme::Highlight) << "\n";
    ss << Styled(Repeat("─", 50), Theme::Muted) << "\n";
    ss << KeyValue("Money Supply", Styled("$" + to_string((int)stats.moneySupply), Theme::Success)) << "\n";
    ss << KeyValue("Interest Rate", Styled(to_string(stats.interestRate) + "%", Theme::Info)) << "\n";
    ss << KeyValue("Budget", Styled("$" + to_string((int)stats.budget), Theme::Info)) << "\n";
    ss << KeyValue("Public Debt", Styled("$" + to_string((int)stats.debt), Theme::Warning)) << "\n";
    ss << "\n";

    // Inequality
    ss << Styled("INEQUALITY", Theme::Highlight) << "\n";
    ss << Styled(Repeat("─", 50), Theme::Muted) << "\n";
    ss << KeyValue("Gini Coefficient", Styled(to_string(stats.giniCoefficient), Theme::Info)) << "\n";
    ss << KeyValue("Number of Firms", Styled(to_string(stats.firms), Theme::Info)) << "\n";

    cout << "\n"
         << BoxedText(ss.str(), "Economic Dashboard") << "\n";
}

string CLI::ReadLine()
{
    string input;
    getline(cin, input);
    return input;
}

void CLI::InitializeCommands()
{
    availableCommands = {
        // CLI-specific commands
        "help", "exit", "quit", "clear", "cls", "refresh", "dashboard", "status",
        // Entity listing commands
        "persons", "workers", "farmers", "owners", "markets",
        // Entity detail commands
        "person", "worker", "farmer", "owner", "market",
        // System commands
        "system",
        // Entity creation
        "add_worker", "add_farmer", "add_owner", "add_market",
        // Selection
        "select", "clear_selection",
        // Actions
        "buy", "harvest", "reset"};
}

vector<string> CLI::GetSuggestions(const string &input)
{
    vector<string> suggestions;

    if (input.empty())
        return suggestions;

    // Find commands that start with the input
    for (const auto &cmd : availableCommands)
    {
        if (cmd.find(input) == 0) // Starts with input
        {
            suggestions.push_back(cmd);
        }
    }

    return suggestions;
}

string CLI::ReadLineWithAutocomplete()
{
    string input;
    vector<string> suggestions;
    int historyPos = history.size();

#ifdef _WIN32
    char ch;
    while (true)
    {
        ch = _getch();

        // Enter key
        if (ch == '\r' || ch == '\n')
        {
            return input;
        }
        // Backspace
        else if (ch == '\b' || ch == 127)
        {
            if (!input.empty())
            {
                input.pop_back();
                cout << "\b \b"; // Erase character

                // Clear any previous suggestion text
                cout << "\033[K"; // Clear from cursor to end of line

                // Update and show suggestions
                suggestions = GetSuggestions(input);
                if (!suggestions.empty() && !input.empty())
                {
                    string remaining = suggestions[0].substr(input.length());
                    cout << "\033[s"; // Save cursor
                    cout << Styled(remaining, Color::Dim);
                    cout << "\033[u"; // Restore cursor
                }
            }
        }
        // Tab key - autocomplete
        else if (ch == '\t')
        {
            suggestions = GetSuggestions(input);
            if (!suggestions.empty())
            {
                // Clear current input on screen
                for (size_t i = 0; i < input.length(); ++i)
                    cout << "\b \b";

                input = suggestions[0];
                cout << input;
            }
        }
        // Arrow keys
        else if (ch == 0 || ch == -32 || ch == 224)
        {
            ch = _getch(); // Get actual arrow key code

            // Up arrow (72) - previous command
            if (ch == 72 && !history.empty())
            {
                if (historyPos > 0)
                    historyPos--;

                // Clear current input
                for (size_t i = 0; i < input.length(); ++i)
                    cout << "\b \b";

                if (historyPos < history.size())
                {
                    input = history[historyPos];
                    cout << input;
                }
            }
            // Down arrow (80) - next command
            else if (ch == 80 && !history.empty())
            {
                if (historyPos < history.size())
                    historyPos++;

                // Clear current input
                for (size_t i = 0; i < input.length(); ++i)
                    cout << "\b \b";

                if (historyPos < history.size())
                {
                    input = history[historyPos];
                    cout << input;
                }
                else
                {
                    input = "";
                }
            }
        }
        // Regular character
        else if (ch >= 32 && ch <= 126)
        {
            input += ch;
            cout << ch;

            // Clear any previous suggestion text
            cout << "\033[K"; // Clear from cursor to end of line

            // Show suggestion in gray
            suggestions = GetSuggestions(input);
            if (!suggestions.empty())
            {
                string remaining = suggestions[0].substr(input.length());
                cout << "\033[s"; // Save cursor
                cout << Styled(remaining, Color::Dim);
                cout << "\033[u"; // Restore cursor
            }
        }
    }
#else
    // Fallback for non-Windows
    getline(cin, input);
    return input;
#endif
}

void CLI::AddToHistory(const string &command)
{
    if (!command.empty() && (history.empty() || history.back() != command))
    {
        history.push_back(command);
        historyIndex = history.size();
    }
}

void CLI::ClearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

string CLI::FormatEntityBox(const string &title, const string &content, int width)
{
    stringstream box;

    // Safety check for width
    if (width < 10)
        width = 10;

    // Top border with color
    box << Styled("╭" + Repeat("─", width) + "╮", Theme::Primary);

    // Title line
    int titleLen = title.length();
    int padding = (width - titleLen) / 2;
    if (padding < 0)
        padding = 0;

    int rightPad = width - padding - titleLen;
    if (rightPad < 0)
        rightPad = 0;

    box << "\n"
        << Styled("│", Theme::Primary) << string(padding, ' ') << Styled(title, Theme::Highlight)
        << string(rightPad, ' ') << Styled("│", Theme::Primary);

    // Separator with color
    box << "\n"
        << Styled("├" + Repeat("─", width) + "┤", Theme::Primary);

    // Content lines (split by newline)
    stringstream ss(content);
    string line;
    while (getline(ss, line))
    {
        // Truncate if too long
        if (line.length() > (size_t)(width - 2))
            line = line.substr(0, width - 5) + "...";

        int linePad = width - line.length() - 1;
        if (linePad < 0)
            linePad = 0;

        box << "\n"
            << Styled("│", Theme::Primary) << " " << Styled(line, Color::BrightWhite) << string(linePad, ' ') << Styled("│", Theme::Primary);
    }

    // Add empty line if only one content line
    if (content.find('\n') == string::npos)
    {
        box << "\n"
            << Styled("│", Theme::Primary) << string(width, ' ') << Styled("│", Theme::Primary);
    }

    // Bottom border with color
    box << "\n"
        << Styled("╰" + Repeat("─", width) + "╯", Theme::Primary);

    return box.str();
}

string CLI::FormatTwoColumnBox(const string &leftContent, const string &rightContent, int totalWidth)
{
    stringstream result;

    // Ensure minimum width
    if (totalWidth < 20)
        totalWidth = 20;

    // Split content into lines
    auto splitLines = [](const string &text)
    {
        vector<string> lines;
        stringstream ss(text);
        string line;
        while (getline(ss, line))
        {
            lines.push_back(line);
        }
        return lines;
    };

    vector<string> leftLines = splitLines(leftContent);
    vector<string> rightLines = splitLines(rightContent);

    // Compute max line lengths (for dynamic sizing)
    size_t leftMax = 0;
    for (const auto &l : leftLines)
        leftMax = max(leftMax, l.size());
    size_t rightMax = 0;
    for (const auto &r : rightLines)
        rightMax = max(rightMax, r.size());

    // Prefer a wider left column, but keep right column usable
    int leftWidth = static_cast<int>(leftMax + 3);
    int rightWidth = totalWidth - leftWidth - 5;

    // Safety check
    if (rightWidth < 5)
    {
        rightWidth = 5;
        leftWidth = totalWidth - rightWidth - 3;
    }
    if (leftWidth < 5)
        leftWidth = 5;

    // Top border with color
    result << Styled("╭" + Repeat("─", leftWidth) + "┬" + Repeat("─", rightWidth) + "╮", Theme::Primary) << "\n";

    // Content lines
    for (size_t i = 0; i < max(leftLines.size(), rightLines.size()); ++i)
    {
        string left = i < leftLines.size() ? leftLines[i] : "";
        string right = i < rightLines.size() ? rightLines[i] : "";

        // Pad or truncate - ensure we don't go negative
        int leftPad = leftWidth - left.length() - 1;
        int rightPad = rightWidth - right.length() - 1;

        if (left.length() > (size_t)(leftWidth - 3))
            left = left.substr(0, leftWidth - 3) + "...";
        if (right.length() > (size_t)(rightWidth - 3))
            right = right.substr(0, rightWidth - 3) + "...";

        leftPad = leftWidth - left.length() - 1;
        rightPad = rightWidth - right.length() - 1;

        if (leftPad < 0)
            leftPad = 0;
        if (rightPad < 0)
            rightPad = 0;

        result << Styled("│", Theme::Primary) << " " << Styled(left, Theme::Highlight) << string(leftPad, ' ');
        result << Styled("│", Theme::Primary) << " " << right << string(rightPad, ' ') << Styled("│", Theme::Primary) << "\n";
    }

    // Bottom border with color
    result << Styled("╰" + Repeat("─", leftWidth) + "┴" + Repeat("─", rightWidth) + "╯", Theme::Primary) << "\n";

    return result.str();
}

void CLI::ShowCommandSuggestions(const string &currentInput)
{
    vector<string> suggestions = GetSuggestions(currentInput);

    // Filter out exact matches (don't show suggestion if user typed it exactly)
    vector<string> filtered;
    for (const auto &s : suggestions)
    {
        if (s != currentInput)
            filtered.push_back(s);
    }

    if (!filtered.empty() && !currentInput.empty())
    {
        // cout << Styled("  Suggestions: ", Theme::Muted);
        for (size_t i = 0; i < min(filtered.size(), size_t(3)); ++i)
        {
            cout << Styled(filtered[i], Theme::Info);
            if (i < min(filtered.size(), size_t(3)) - 1)
                cout << Styled(" | ", Theme::Muted);
        }
    }
    cout << "\n";
}