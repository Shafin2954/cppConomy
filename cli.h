#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <conio.h>
#include <sstream>
#include <iomanip>

#include "style.h"
#include "farmer.h"
#include "laborer.h"
#include "firm.h"
#include "market.h"
#include "world.h"
#include "executor.h"

using namespace styledTerminal;
using namespace Box;

class cli
{
public:
    bool running = true;
    std::vector<std::string> history;
    int historyIndex = 0;

    std::vector<std::string> availableCommands;

    world &simulation;
    cli(world &w) : simulation(w)
    {
        styledTerminal::Init(); // Initialize terminal for color support (Windows)
        simulation.innitialize();

        // Initialize available commands from parser
        for (const auto &cmd : executor.parser.getAvailableCommands())
        {
            availableCommands.push_back(cmd.name);
        }
    }

    cmdExec executor{simulation, [this](const std::string &msg)
                     { handleOutput(msg); }};

    void run()
    {

        std::cout << Styled("Innitializing world", Color::BrightYellow);
// pause and clear screen
#ifdef _WIN32
        system("timeout 1 > nul"); // pause for 1 second (Windows)
        std::cout << ".";
        system("timeout 1 > nul"); // pause for 1 second (Windows)
        std::cout << ".";
        system("timeout 1 > nul"); // pause for 1 second (Windows)
        std::cout << ".";
        system("timeout 1 > nul"); // pause for 1 second (Windows)
        system("cls");
#else
        system("sleep 3"); // pause for 3 seconds (Unix)
        system("clear");
#endif

        // show header
        showStickyHeader();

        auto trimInput = [](std::string &value)
        {
            size_t first = value.find_first_not_of(" \t\n\r");
            if (first == std::string::npos)
            {
                value.clear();
                return;
            }
            size_t last = value.find_last_not_of(" \t\n\r");
            value = value.substr(first, last - first + 1);
        };

        while (running)
        {
            try
            {
                std::cout << Styled(Repeat(Horizontal, 93), Color::Gray) << "\n"; // separator
                std::cout << Prompt();
                std::cout << std::flush; // ensure prompt is printed before input

                std::string input = readWithAutoComplete();
                std::cout << "\n"; // Newline after input

                // Clear suggestion line
                std::cout << "\r" << std::string(93, ' ') << "\r";

                // Print bottom separator
                std::cout << Styled(Repeat("─", 93), Theme::Muted) << "\n";

                trimInput(input);

                if (input.empty())
                    continue;

                // Add to history
                AddToHistory(input);

                // Check for special commands
                if (input == "exit" || input == "quit")
                {
                    std::cout << Styled("\nGoodbye!\n", Theme::Success);
                    running = false;
                    break;
                }
                else if (input == "clear" || input == "cls" || input == "refresh")
                {
                    clearScreen();
                    showStickyHeader();
                    continue;
                }
                else if (input == "status")
                {
                    std::cout << "\n";
                    showStatus();
                    continue;
                }

                // Process command
                std::cout << "\n";
                processCommand(input);

                // Refresh header after commands that might change selections
                if (input.find("select") != std::string::npos)
                {
                    clearScreen();
                    showStickyHeader();
                }
            }
            catch (const std::exception &e)
            {
                std::cout << Error(std::string("Unhandled exception: ") + e.what()) << "\n";
            }
            catch (...)
            {
                std::cout << Error("Unhandled exception: unknown") << "\n";
            }
        }
    };

    void showStickyHeader()
    {
        std::vector<std::string> left = {
            "                                                      ",
            "     _____          _____                             ",
            "    / ___/__  ___  / ___/__  ___  ___  __ _  __ __    ",
            "   / /__/ _ \\/ _ \\/ /__/ _ \\/ _ \\/ _ \\/  ' \\/ // /    ",
            "   \\___/ .__/ .__/\\___/\\___/_//_/\\___/_/_/_/\\_, /     ",
            "      /_/  /_/                             /___/      ",
            "                                                      ",
            "                                                      ",
            "                                                      "};

        // Right column - Tips and info (each line is a string)
        std::vector<std::string> right = {
            "  Quick Commands:                 ",
            "                                  ",
            "  help       - Show all commands  ",
            "  dashboard  - Economic stats     ",
            "  workers    - List all workers   ",
            "  farmers    - List all farmers   ",
            "  select     - Select entities    ",
            "                                  "};

        size_t leftWidth = 55;
        size_t rightWidth = 35;

        //                ╭ from box namespace                                                                             ╭ char + 4
        std::cout << Styled(TopLeft + Repeat(Horizontal, 2) + std::string(" Economic Engine v2.0 ") + Repeat(Horizontal, leftWidth - 24) + Horizontal + Repeat(Horizontal, rightWidth) + TopRight, Theme::Primary) << "\n";

        //                     ╭ lines
        for (size_t i = 0; i < 8; ++i)
        {
            std::cout << Styled(Vertical, Theme::Primary) << " " << Styled(left[i], Theme::Primary);
            std::cout << Styled(Vertical, Color::Gray) << " " << Styled(right[i], (i == 0) ? Theme::Primary : Theme::Secondary) << Styled(Vertical, Theme::Primary) << "\n";
        }

        std::cout << Styled(BottomLeft + Repeat(Horizontal, leftWidth) + Horizontal + Repeat(Horizontal, rightWidth) + BottomRight, Theme::Primary) << "\n";

        laborer *selLaborer = simulation.GetSelectedLaborer();
        farmer *selFarmer = simulation.GetSelectedFarmer();
        consumer *selConsumer = simulation.GetSelectedConsumer();
        market *selMarket = simulation.GetSelectedMarket();

        std::string marketPriceText;
        if (selMarket)
        {
            std::ostringstream priceStream;
            priceStream << std::fixed << std::setprecision(2) << selMarket->price;
            marketPriceText = priceStream.str();
        }

        std::string cropNames = "";
        if (selFarmer)
        {
            for (size_t i = 0; i < selFarmer->crops.size(); ++i)
            {
                cropNames += selFarmer->crops[i].name;
                if (i < selFarmer->crops.size() - 1)
                    cropNames += ", ";
            }
        }

        std::vector<std::string> slot1 = createSlot("LABORER",
                                                    selLaborer ? selLaborer->name +
                                                                     "\nSkill Level: " + std::to_string((int)(selLaborer->skillLevel * 100)) + "%" +
                                                                     "\nMin Wage: " + std::to_string((int)selLaborer->minWage) + "TK per day"
                                                               : "\nNone selected\n ",
                                                    24);

        std::vector<std::string> slot2 = createSlot("FARMER",
                                                    selFarmer ? selFarmer->name +
                                                                    "\nLand: " + std::to_string((int)selFarmer->land) + " acres" +
                                                                    "\nCrops: " + cropNames
                                                              : "\nNone selected\n ");

        std::vector<std::string> slot3 = createSlot("CONSUMER",
                                                    selConsumer ? selConsumer->name +
                                                                      "\nAge: " + std::to_string(selConsumer->ageInDays / 365) + " years" +
                                                                      "\nSavings: $" + std::to_string((int)selConsumer->savings)
                                                                : "\nNone selected\n ");

        std::vector<std::string> slot4 = createSlot("MARKET",
                                                    selMarket ? selMarket->prod->name +
                                                                    "\nPrice: $" + marketPriceText +
                                                                    "\n "
                                                              : "\nNone selected\n ",
                                                    19);

        // Print slots side by side
        for (size_t i = 0; i < slot1.size(); ++i)
        {
            std::cout << slot1[i] << slot2[i] << slot3[i] << slot4[i] << "\n";
        }
    };

    std::vector<std::string> createSlot(const std::string &title, const std::string &content, int width = 21)
    {

        std::vector<std::string> box;

        int titleLen = title.length() + 2;

        // Top border with color
        box.push_back(Styled(TopLeft + Repeat(Horizontal, 1) + " " + title + " " + Repeat(Horizontal, width - titleLen - 1) + TopRight, Theme::Primary));

        // Content lines (split by newline)
        std::stringstream ss(content);
        std::string line;
        while (getline(ss, line))
        {
            // Truncate if too long
            if (line.length() > (size_t)(width - 2))
                line = line.substr(0, width - 5) + "...";

            int linePad = width - line.length() - 1;
            if (linePad < 0)
                linePad = 0;

            box.push_back(
                Styled("│", Theme::Primary) + " " + Styled(line, Color::BrightWhite) + std::string(linePad, ' ') + Styled("│", Theme::Primary));
        }

        // Add empty line if only one content line
        if (content.find('\n') == std::string::npos)
        {
            box.push_back(
                Styled("│", Theme::Primary) + std::string(width, ' ') + Styled("│", Theme::Primary));
        }

        // Bottom border with color
        box.push_back(
            Styled(BottomLeft + Repeat(Horizontal, width) + BottomRight, Theme::Primary));

        return box;
    }

    std::vector<std::string> GetSuggestions(const std::string &prefix)
    {
        std::vector<std::string> suggestions;

        if (prefix.empty())
            return suggestions;

        // Find commands that start with the prefix
        for (const auto &cmd : availableCommands)
        {
            if (cmd.find(prefix) == 0) // Starts with prefix
            {
                suggestions.push_back(cmd);
            }
        }

        return suggestions;
    }

    void AddToHistory(const std::string &command)
    { // Avoid adding empty or duplicate consecutive commands
        if (!command.empty() && (history.empty() || history.back() != command))
        {
            history.push_back(command);
            historyIndex = history.size();
        }
    }

    void clearScreen()
    {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void showStatus()
    {
        auto stats = simulation.getStats();

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

    void processCommand(const std::string &input)
    {
        bool success = executor.execute(input);

        // Check if this was an exit command
        if (success && (input == "exit" || input.find("exit") == 0))
        {
            running = false;
            return;
        }

        if (!success && !executor.getLastError().empty())
        {
            std::cout << Error("Command failed: " + executor.getLastError()) << "\n";
        }

        std::cout << "\n";
    }

    void handleOutput(const std::string &message)
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
            std::cout << message << "\n";
        }
        else
        {
            std::cout << message << "\n";
        }
    }

    std::string readWithAutoComplete()
    {
#ifdef _WIN32
        std::string input;
        std::vector<std::string> suggestions;
        int historyPos = history.size();
        size_t cursorPos = 0;
        int displayedSuggestionCount = 0;

        // Reserve 4 lines below for suggestions by scrolling if needed
        // Print 4 newlines to create scroll buffer, then move back up
        std::cout << "\n\n\n\n";
        std::cout << "\033[4A"; // Move up 4 lines
        std::cout << "\033[2K"; // Clear line
        std::cout << Prompt();  // Reprint prompt so cursor is after it
        std::cout.flush();

        // Helper to refresh suggestion lines printed below the input line.
        // We move down from the input line, print suggestions, then move back up.
        auto refreshSuggestions = [&](const std::vector<std::string> &lines)
        {
            int newCount = (int)lines.size();
            int oldCount = displayedSuggestionCount;
            int maxLines = (newCount > oldCount) ? newCount : oldCount;

            if (maxLines == 0)
            {
                displayedSuggestionCount = 0;
                return;
            }

            // Move down and print/clear each suggestion line
            for (int i = 0; i < maxLines; ++i)
            {
                std::cout << "\033[1E"; // Move to beginning of next line
                std::cout << "\033[2K"; // Clear entire line

                if (i < newCount)
                {
                    std::cout << lines[i];
                    std::cout << "\033[2m"; // Reset colors after each suggestion
                }
            }

            // Move cursor back up to the input line
            std::cout << "\033[" << maxLines << "F"; // Move up maxLines lines and to column 1

            // Reprint the input line: clear it, print prompt and full input, position cursor
            std::cout << "\033[2K"; // Clear the input line
            std::cout << Prompt();
            std::cout << Styled(input, Theme::Primary);

            // Move cursor back to cursorPos within input
            size_t charsAfterCursor = input.length() - cursorPos;
            if (charsAfterCursor > 0)
            {
                std::cout << "\033[" << charsAfterCursor << "D"; // Move left
            }

            std::cout.flush();
            displayedSuggestionCount = newCount;
        };

        char ch;
        while (true)
        {
            ch = _getch();

            // Ctrl+C
            if (ch == 3)
            {   
                running = false;
                refreshSuggestions({});
                std::cout << "ctrl + c\n"
                          << Styled("Goodbye!\n", Theme::Success);
                exit(0);
            }

            // Enter key
            if (ch == '\r' || ch == '\n')
            {
                // clear suggestion area before returning
                refreshSuggestions({});
                return input;
            }

            // Backspace
            else if (ch == '\b' || ch == 127)
            {
                if (!input.empty() && cursorPos > 0)
                {
                    if (cursorPos == input.length())
                    {
                        input.pop_back();
                        cursorPos = input.length();
                        std::cout << "\b \b"; // Erase character at end
                    }
                    else
                    {
                        // remove char before cursor and redraw tail
                        input.erase(cursorPos - 1, 1);
                        // move cursor left
                        std::cout << "\033[D";
                        cursorPos--;
                        // clear to end and print tail
                        std::cout << "\033[K";
                        std::cout << input.substr(cursorPos);
                        // move cursor back to caret position
                        for (size_t m = 0; m < input.length() - cursorPos; ++m)
                            std::cout << "\033[D";
                    }

                    // Update and show suggestions (printed on next lines)
                    suggestions = GetSuggestions(input);
                    if (!suggestions.empty() && !input.empty())
                    {
                        std::vector<std::string> lines;
                        int show = std::min<int>(4, (int)suggestions.size());
                        for (int i = 0; i < show; ++i)
                            lines.push_back(Styled(suggestions[i], Color::Dim));
                        refreshSuggestions(lines);
                    }
                    else
                    {
                        refreshSuggestions({});
                    }
                }
            }
            // Tab key - autocomplete
            else if (ch == '\t')
            {
                suggestions = GetSuggestions(input);
                if (!suggestions.empty())
                {
                    // Ensure caret at end before clearing
                    if (cursorPos < input.length())
                    {
                        for (size_t mv = 0; mv < input.length() - cursorPos; ++mv)
                            std::cout << "\033[C";
                        cursorPos = input.length();
                    }

                    // Clear current input on screen
                    for (size_t i = 0; i < input.length(); ++i)
                        std::cout << "\b \b";

                    // Fill only up to first '(' if present, otherwise full suggestion
                    std::string suggestion = suggestions[0];
                    size_t parenPos = suggestion.find('(');
                    std::string fill;
                    if (parenPos != std::string::npos)
                    {
                        fill = suggestion.substr(0, parenPos + 1); // include '('
                    }
                    else
                    {
                        fill = suggestion;
                    }

                    input = fill;
                    std::cout << input;
                    cursorPos = input.length();

                    // refresh suggestion list under input
                    suggestions = GetSuggestions(input);
                    if (!suggestions.empty())
                    {
                        std::vector<std::string> lines;
                        int show = std::min<int>(4, (int)suggestions.size());
                        for (int i = 0; i < show; ++i)
                            lines.push_back(Styled(suggestions[i], Color::Dim));
                        refreshSuggestions(lines);
                    }
                    else
                    {
                        refreshSuggestions({});
                    }
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

                    // Move caret to end before clearing
                    if (cursorPos < input.length())
                    {
                        for (size_t mv = 0; mv < input.length() - cursorPos; ++mv)
                            std::cout << "\033[C";
                    }

                    // Clear current input
                    for (size_t i = 0; i < input.length(); ++i)
                        std::cout << "\b \b";

                    if (historyPos < history.size())
                    {
                        input = history[historyPos];
                        std::cout << input;
                        cursorPos = input.length();

                        // refresh suggestions for new input from history
                        suggestions = GetSuggestions(input);
                        if (!suggestions.empty())
                        {
                            std::vector<std::string> lines;
                            int show = std::min<int>(4, (int)suggestions.size());
                            for (int i = 0; i < show; ++i)
                                lines.push_back(Styled(suggestions[i], Color::Dim));
                            refreshSuggestions(lines);
                        }
                        else
                        {
                            refreshSuggestions({});
                        }
                    }
                }
                // Down arrow (80) - next command
                else if (ch == 80 && !history.empty())
                {
                    if (historyPos < history.size())
                        historyPos++;

                    // Move caret to end before clearing
                    if (cursorPos < input.length())
                    {
                        for (size_t mv = 0; mv < input.length() - cursorPos; ++mv)
                            std::cout << "\033[C";
                    }

                    // Clear current input
                    for (size_t i = 0; i < input.length(); ++i)
                        std::cout << "\b \b";

                    if (historyPos < history.size())
                    {
                        input = history[historyPos];
                        std::cout << input;
                        cursorPos = input.length();

                        // refresh suggestions for new input from history
                        suggestions = GetSuggestions(input);
                        if (!suggestions.empty())
                        {
                            std::vector<std::string> lines;
                            int show = std::min<int>(4, (int)suggestions.size());
                            for (int i = 0; i < show; ++i)
                                lines.push_back(Styled(suggestions[i], Color::Dim));
                            refreshSuggestions(lines);
                        }
                        else
                        {
                            refreshSuggestions({});
                        }
                    }
                    else
                    {
                        input = "";
                        cursorPos = 0;
                        refreshSuggestions({});
                    }
                }
                // Left arrow (75) - move caret left
                else if (ch == 75)
                {
                    if (cursorPos > 0)
                    {
                        std::cout << "\033[D";
                        cursorPos--;
                    }
                }
                // Right arrow (77) - move caret right
                else if (ch == 77)
                {
                    if (cursorPos < input.length())
                    {
                        std::cout << "\033[C";
                        cursorPos++;
                    }
                }
            }
            // Regular character
            else if (ch >= 32 && ch <= 126)
            {
                if (cursorPos == input.length())
                {
                    input += ch;
                    std::cout << Styled(std::string(1, ch), Theme::Primary);
                    cursorPos++;
                }
                else
                {
                    // insert at caret and redraw tail
                    input.insert(input.begin() + cursorPos, ch);
                    // clear from cursor and print tail (including inserted char)
                    std::cout << "\033[K";
                    std::cout << Styled(input.substr(cursorPos), Theme::Primary);
                    // move cursor back to position after inserted char
                    size_t tailLen = input.length() - cursorPos - 1;
                    for (size_t m = 0; m < tailLen; ++m)
                        std::cout << "\033[D";
                    cursorPos++;
                }

                // refresh suggestion list under input
                suggestions = GetSuggestions(input);
                if (!suggestions.empty())
                {
                    std::vector<std::string> lines;
                    int show = std::min<int>(4, (int)suggestions.size());
                    for (int i = 0; i < show; ++i)
                        lines.push_back(Styled(suggestions[i], Color::Dim));
                    refreshSuggestions(lines);
                }
                else
                {
                    refreshSuggestions({});
                }
            }
#else
        // Fallback for non-Windows
        getline(cin, input);
        return input;
#endif
        }
    }
};
