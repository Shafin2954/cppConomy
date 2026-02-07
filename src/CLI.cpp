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

CLI::CLI()
{
    // Initialize terminal colors
    TermColors::Init();

    // Create simulation and executor
    m_simulation = std::make_unique<Simulation>();
    m_executor = std::make_unique<CommandExecutor>(*m_simulation);

    // Set up output callback
    m_executor->setOutputCallback([this](const std::string &msg)
                                  { HandleOutput(msg); });

    // Initialize simulation with default entities
    m_simulation->Initialize(2, 1, 1);
    m_simulation->RefreshStats();

    // Initialize command list for autocomplete
    InitializeCommands();
}

void CLI::Run()
{
    ClearScreen();
    ShowStickyHeader();

    while (m_running)
    {
        // Print separator and prompt
        std::cout << Styled(Repeat("─", 88), Theme::Muted) << "\n";
        std::cout << Prompt();
        std::cout << std::flush;

        // Read input with autocomplete
        std::string input = ReadLineWithAutocomplete();
        std::cout << "\n"; // Newline after input

        // Clear suggestion line
        std::cout << "\r" << std::string(88, ' ') << "\r";

        // Print bottom separator
        std::cout << Styled(Repeat("─", 88), Theme::Muted) << "\n";

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
            std::cout << Styled("\n  Goodbye!\n", Theme::Success);
            m_running = false;
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
            std::cout << "\n";
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
        std::cout << "\n";
        ProcessCommand(input);

        // Refresh header after commands that might change selections
        if (input.find("select") != std::string::npos)
        {
            ClearScreen();
            ShowStickyHeader();
        }
    }
}

void CLI::ShowBanner()
{
    std::stringstream banner;
    banner << R"(
   ____            ____                                 
  / ___|_ __  _ __/ ___|___  _ __   ___  _ __ ___  _   _ 
 | |   | '_ \| '_ \___ \/ _ \| '_ \ / _ \| '_ ` _ \| | | |
 | |___| |_) | |_) |__) | (_) | | | | (_) | | | | | | |_| |
  \____| .__/| .__/____/ \___/|_| |_|\___/|_| |_| |_|\__, |
       |_|   |_|                                      |___/ 
)";

    std::cout << Styled(banner.str(), Theme::Primary);
    std::cout << Styled("  Economic Simulation Engine", std::string(Color::Bold).append(Theme::Highlight).c_str()) << "\n";
    std::cout << Styled("  Version 2.0-CLI", Theme::Muted) << "\n\n";
}

void CLI::ShowStickyHeader()
{
    // Two-column header box
    std::stringstream left, right;

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

    std::cout << FormatTwoColumnBox(left.str(), right.str(), 90);

    // Entity status boxes
    Worker *selWorker = m_simulation->GetSelectedWorker();
    Farmer *selFarmer = m_simulation->GetSelectedFarmer();
    Owner *selOwner = m_simulation->GetSelectedOwner();
    Market *selMarket = m_simulation->GetSelectedMarket();

    std::string farmerBox = FormatEntityBox("FARMER",
                                            selFarmer ? selFarmer->GetName() + "\nLand: " + std::to_string((int)selFarmer->GetLandSize()) + " acres" : "None selected");

    std::string workerBox = FormatEntityBox("WORKER",
                                            selWorker ? selWorker->GetName() + "\nIncome: $" + std::to_string((int)selWorker->GetMonthlyIncome()) : "None selected");

    std::string ownerBox = FormatEntityBox("OWNER",
                                           selOwner ? selOwner->GetName() + "\nCapital: $" + std::to_string((int)selOwner->GetCapital()) : "None selected");

    std::string marketBox = FormatEntityBox("MARKET",
                                            selMarket ? selMarket->GetProductName() + "\nPrice: $" + std::to_string((int)selMarket->GetCurrentPrice()) : "None selected");

    // Split boxes into lines for side-by-side display
    auto splitLines = [](const std::string &text)
    {
        std::vector<std::string> lines;
        std::stringstream ss(text);
        std::string line;
        while (std::getline(ss, line))
        {
            lines.push_back(line);
        }
        return lines;
    };

    std::vector<std::string> farmerLines = splitLines(farmerBox);
    std::vector<std::string> workerLines = splitLines(workerBox);
    std::vector<std::string> ownerLines = splitLines(ownerBox);
    std::vector<std::string> marketLines = splitLines(marketBox);

    size_t maxLines = std::max({farmerLines.size(), workerLines.size(), ownerLines.size(), marketLines.size()});

    for (size_t i = 0; i < maxLines; ++i)
    {
        std::cout << (i < farmerLines.size() ? farmerLines[i] : std::string(23, ' '));
        std::cout << (i < workerLines.size() ? workerLines[i] : std::string(22, ' '));
        std::cout << (i < ownerLines.size() ? ownerLines[i] : std::string(22, ' '));
        std::cout << (i < marketLines.size() ? marketLines[i] : "");
        std::cout << "\n";
    }

    std::cout << "\n";
}

void CLI::ShowHelp()
{
    std::cout << Styled("Available Commands:", Theme::Highlight) << "\n";
    std::cout << "\n"
              << Styled("Entity Listing:", Theme::Highlight) << "\n";
    std::cout << "  " << Styled("persons", Theme::Info) << ", " << Styled("workers", Theme::Info) << ", " << Styled("farmers", Theme::Info) << ", " << Styled("owners", Theme::Info) << ", " << Styled("markets", Theme::Info) << "\n";

    std::cout << "\n"
              << Styled("Entity Details:", Theme::Highlight) << "\n";
    std::cout << "  " << Styled("person", Theme::Info) << "  - Get person details by name\n";
    std::cout << "  " << Styled("worker", Theme::Info) << "  - Get worker details by name\n";
    std::cout << "  " << Styled("farmer", Theme::Info) << "  - Get farmer details by name\n";
    std::cout << "  " << Styled("owner", Theme::Info) << "   - Get owner details by name\n";
    std::cout << "  " << Styled("market", Theme::Info) << "  - Get market details by name\n";

    std::cout << "\n"
              << Styled("Entity Creation:", Theme::Highlight) << "\n";
    std::cout << "  " << Styled("add_worker", Theme::Info) << "  - Add a worker\n";
    std::cout << "  " << Styled("add_farmer", Theme::Info) << "  - Add a farmer\n";
    std::cout << "  " << Styled("add_owner", Theme::Info) << "   - Add a business owner\n";
    std::cout << "  " << Styled("add_market", Theme::Info) << "  - Add a market\n";

    std::cout << "\n"
              << Styled("Actions:", Theme::Highlight) << "\n";
    std::cout << "  " << Styled("select", Theme::Info) << "         - Select entities to show\n";
    std::cout << "  " << Styled("clear_selection", Theme::Info) << " - Clear selection\n";
    std::cout << "  " << Styled("buy", Theme::Info) << "            - Worker buys product\n";
    std::cout << "  " << Styled("harvest", Theme::Info) << "        - Harvest crops\n";
    std::cout << "  " << Styled("reset", Theme::Info) << "         - Reset simulation\n";

    std::cout << "\n"
              << Styled("System:", Theme::Highlight) << "\n";
    std::cout << "  " << Styled("help", Theme::Info) << "      - Show this help\n";
    std::cout << "  " << Styled("status", Theme::Info) << "    - Show simulation status\n";
    std::cout << "  " << Styled("dashboard", Theme::Info) << " - View economic dashboard\n";
    std::cout << "  " << Styled("refresh", Theme::Info) << "   - Refresh screen\n";
    std::cout << "  " << Styled("clear", Theme::Info) << "     - Clear screen\n";
    std::cout << "  " << Styled("exit/quit", Theme::Info) << " - Exit program\n";
}

void CLI::ProcessCommand(const std::string &input)
{
    bool success = m_executor->execute(input);

    if (!success && !m_executor->getLastError().empty())
    {
        std::cout << Error("Command failed: " + m_executor->getLastError()) << "\n";
    }

    std::cout << "\n";
}

void CLI::HandleOutput(const std::string &message)
{
    // Check if it's an error message
    if (message.find("Error:") == 0)
    {
        std::cout << Error(message.substr(7)) << "\n";
    }
    // Check if it's a success/added message
    else if (message.find("Added") == 0 || message.find("complete") != std::string::npos ||
             message.find("cleared") != std::string::npos || message.find("updated") != std::string::npos)
    {
        std::cout << Success(message) << "\n";
    }
    // Check if it's a list or detailed output
    else if (message.find(":\n") != std::string::npos || message.find("---") != std::string::npos)
    {
        // Format as info box for structured data
        std::cout << Styled(message, Color::Reset) << "\n";
    }
    else
    {
        std::cout << message << "\n";
    }
}

void CLI::ShowStatus()
{
    std::string status = m_simulation->GetStatusString();
    std::cout << BoxedText(status, "Simulation Status") << "\n";
}

void CLI::ShowDashboard()
{
    auto stats = m_simulation->GetStats();

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2);

    // Economic Indicators
    ss << Styled("ECONOMIC INDICATORS", Theme::Highlight) << "\n";
    ss << Styled(Repeat("─", 50), Theme::Muted) << "\n";
    ss << KeyValue("GDP", Styled("$" + std::to_string((int)stats.gdp), Theme::Success)) << "\n";
    ss << KeyValue("GDP Growth", Styled(std::to_string(stats.gdpGrowth) + "%", Theme::Info)) << "\n";
    ss << KeyValue("Inflation", Styled(std::to_string(stats.inflation) + "%", Theme::Warning)) << "\n";
    ss << KeyValue("CPI", std::to_string(stats.cpi)) << "\n";
    ss << "\n";

    // Labor Market
    ss << Styled("LABOR MARKET", Theme::Highlight) << "\n";
    ss << Styled(Repeat("─", 50), Theme::Muted) << "\n";
    ss << KeyValue("Population", Styled(std::to_string(stats.population), Theme::Info)) << "\n";
    ss << KeyValue("Employed", Styled(std::to_string(stats.employed), Theme::Success)) << "\n";
    ss << KeyValue("Unemployment Rate", Styled(std::to_string(stats.unemployment) + "%",
                                               stats.unemployment > 10 ? Theme::Error : Theme::Info))
       << "\n";
    ss << "\n";

    // Government & Finance
    ss << Styled("GOVERNMENT & FINANCE", Theme::Highlight) << "\n";
    ss << Styled(Repeat("─", 50), Theme::Muted) << "\n";
    ss << KeyValue("Money Supply", Styled("$" + std::to_string((int)stats.moneySupply), Theme::Success)) << "\n";
    ss << KeyValue("Interest Rate", Styled(std::to_string(stats.interestRate) + "%", Theme::Info)) << "\n";
    ss << KeyValue("Budget", Styled("$" + std::to_string((int)stats.budget), Theme::Info)) << "\n";
    ss << KeyValue("Public Debt", Styled("$" + std::to_string((int)stats.debt), Theme::Warning)) << "\n";
    ss << "\n";

    // Inequality
    ss << Styled("INEQUALITY", Theme::Highlight) << "\n";
    ss << Styled(Repeat("─", 50), Theme::Muted) << "\n";
    ss << KeyValue("Gini Coefficient", Styled(std::to_string(stats.giniCoefficient), Theme::Info)) << "\n";
    ss << KeyValue("Number of Firms", Styled(std::to_string(stats.firms), Theme::Info)) << "\n";

    std::cout << "\n"
              << BoxedText(ss.str(), "Economic Dashboard") << "\n";
}

std::string CLI::ReadLine()
{
    std::string input;
    std::getline(std::cin, input);
    return input;
}

void CLI::InitializeCommands()
{
    m_availableCommands = {
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

std::vector<std::string> CLI::GetSuggestions(const std::string &input)
{
    std::vector<std::string> suggestions;

    if (input.empty())
        return suggestions;

    // Find commands that start with the input
    for (const auto &cmd : m_availableCommands)
    {
        if (cmd.find(input) == 0) // Starts with input
        {
            suggestions.push_back(cmd);
        }
    }

    return suggestions;
}

std::string CLI::ReadLineWithAutocomplete()
{
    std::string input;
    std::vector<std::string> suggestions;
    int historyPos = m_history.size();

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
                std::cout << "\b \b"; // Erase character

                // Clear any previous suggestion text
                std::cout << "\033[K"; // Clear from cursor to end of line

                // Update and show suggestions
                suggestions = GetSuggestions(input);
                if (!suggestions.empty() && !input.empty())
                {
                    std::string remaining = suggestions[0].substr(input.length());
                    std::cout << "\033[s"; // Save cursor
                    std::cout << Styled(remaining, Color::Dim);
                    std::cout << "\033[u"; // Restore cursor
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
                    std::cout << "\b \b";

                input = suggestions[0];
                std::cout << input;
            }
        }
        // Arrow keys
        else if (ch == 0 || ch == -32 || ch == 224)
        {
            ch = _getch(); // Get actual arrow key code

            // Up arrow (72) - previous command
            if (ch == 72 && !m_history.empty())
            {
                if (historyPos > 0)
                    historyPos--;

                // Clear current input
                for (size_t i = 0; i < input.length(); ++i)
                    std::cout << "\b \b";

                if (historyPos < m_history.size())
                {
                    input = m_history[historyPos];
                    std::cout << input;
                }
            }
            // Down arrow (80) - next command
            else if (ch == 80 && !m_history.empty())
            {
                if (historyPos < m_history.size())
                    historyPos++;

                // Clear current input
                for (size_t i = 0; i < input.length(); ++i)
                    std::cout << "\b \b";

                if (historyPos < m_history.size())
                {
                    input = m_history[historyPos];
                    std::cout << input;
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
            std::cout << ch;

            // Clear any previous suggestion text
            std::cout << "\033[K"; // Clear from cursor to end of line

            // Show suggestion in gray
            suggestions = GetSuggestions(input);
            if (!suggestions.empty())
            {
                std::string remaining = suggestions[0].substr(input.length());
                std::cout << "\033[s"; // Save cursor
                std::cout << Styled(remaining, Color::Dim);
                std::cout << "\033[u"; // Restore cursor
            }
        }
    }
#else
    // Fallback for non-Windows
    std::getline(std::cin, input);
    return input;
#endif
}

void CLI::AddToHistory(const std::string &command)
{
    if (!command.empty() && (m_history.empty() || m_history.back() != command))
    {
        m_history.push_back(command);
        m_historyIndex = m_history.size();
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

std::string CLI::FormatEntityBox(const std::string &title, const std::string &content, int width)
{
    std::stringstream box;

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
        << Styled("│", Theme::Primary) << std::string(padding, ' ') << Styled(title, Theme::Highlight)
        << std::string(rightPad, ' ') << Styled("│", Theme::Primary);

    // Separator with color
    box << "\n"
        << Styled("├" + Repeat("─", width) + "┤", Theme::Primary);

    // Content lines (split by newline)
    std::stringstream ss(content);
    std::string line;
    while (std::getline(ss, line))
    {
        // Truncate if too long
        if (line.length() > (size_t)(width - 2))
            line = line.substr(0, width - 5) + "...";

        int linePad = width - line.length() - 1;
        if (linePad < 0)
            linePad = 0;

        box << "\n"
            << Styled("│", Theme::Primary) << " " << Styled(line, Color::BrightWhite) << std::string(linePad, ' ') << Styled("│", Theme::Primary);
    }

    // Add empty line if only one content line
    if (content.find('\n') == std::string::npos)
    {
        box << "\n"
            << Styled("│", Theme::Primary) << std::string(width, ' ') << Styled("│", Theme::Primary);
    }

    // Bottom border with color
    box << "\n"
        << Styled("╰" + Repeat("─", width) + "╯", Theme::Primary);

    return box.str();
}

std::string CLI::FormatTwoColumnBox(const std::string &leftContent, const std::string &rightContent, int totalWidth)
{
    std::stringstream result;

    // Ensure minimum width
    if (totalWidth < 20)
        totalWidth = 20;

    // Split content into lines
    auto splitLines = [](const std::string &text)
    {
        std::vector<std::string> lines;
        std::stringstream ss(text);
        std::string line;
        while (std::getline(ss, line))
        {
            lines.push_back(line);
        }
        return lines;
    };

    std::vector<std::string> leftLines = splitLines(leftContent);
    std::vector<std::string> rightLines = splitLines(rightContent);

    // Compute max line lengths (for dynamic sizing)
    size_t leftMax = 0;
    for (const auto &l : leftLines)
        leftMax = std::max(leftMax, l.size());
    size_t rightMax = 0;
    for (const auto &r : rightLines)
        rightMax = std::max(rightMax, r.size());

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
    for (size_t i = 0; i < std::max(leftLines.size(), rightLines.size()); ++i)
    {
        std::string left = i < leftLines.size() ? leftLines[i] : "";
        std::string right = i < rightLines.size() ? rightLines[i] : "";

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

        result << Styled("│", Theme::Primary) << " " << Styled(left, Theme::Highlight) << std::string(leftPad, ' ');
        result << Styled("│", Theme::Primary) << " " << right << std::string(rightPad, ' ') << Styled("│", Theme::Primary) << "\n";
    }

    // Bottom border with color
    result << Styled("╰" + Repeat("─", leftWidth) + "┴" + Repeat("─", rightWidth) + "╯", Theme::Primary) << "\n";

    return result.str();
}

void CLI::ShowCommandSuggestions(const std::string &currentInput)
{
    std::vector<std::string> suggestions = GetSuggestions(currentInput);

    // Filter out exact matches (don't show suggestion if user typed it exactly)
    std::vector<std::string> filtered;
    for (const auto &s : suggestions)
    {
        if (s != currentInput)
            filtered.push_back(s);
    }

    if (!filtered.empty() && !currentInput.empty())
    {
        // std::cout << Styled("  Suggestions: ", Theme::Muted);
        for (size_t i = 0; i < std::min(filtered.size(), size_t(3)); ++i)
        {
            std::cout << Styled(filtered[i], Theme::Info);
            if (i < std::min(filtered.size(), size_t(3)) - 1)
                std::cout << Styled(" | ", Theme::Muted);
        }
    }
    std::cout << "\n";
}