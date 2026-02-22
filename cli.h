#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

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
    // ── Single source of truth for terminal width ──────────────────────────
    // Change this one number to reflow all horizontal lines, separators,
    // the sticky header box, and executor output rules.
    static constexpr int screen_width = 100;

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

    cmdExec executor{simulation,
                     [this](const std::string &msg)
                     { handleOutput(msg); },
                     [this]()
                     {
                         clearScreen();
                         showStickyHeader();
                     },
                     screen_width};

    void run()
    {

        std::cout << Styled("Innitializing world", Color::BrightYellow);
// pause and clear screen
#ifdef _WIN32 //todo: remove comments
        // system("timeout 1 > nul"); // pause for 1 second (Windows)
        // std::cout << ".";
        // system("timeout 1 > nul"); // pause for 1 second (Windows)
        // std::cout << ".";
        // system("timeout 1 > nul"); // pause for 1 second (Windows)
        // std::cout << ".";
        // system("timeout 1 > nul"); // pause for 1 second (Windows)
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

                std::cout << "\n" << Prompt();
                std::cout << std::flush; // ensure prompt is printed before input

                std::string input = readWithAutoComplete();
                std::cout << "\n"; // Newline after input

                // Clear suggestion line
                std::cout << "\r" << std::string(screen_width, ' ') << "\r";

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

                // Refresh header after select commands that might change selections
                if (input.find("select") != std::string::npos)
                {
                    clearScreen();
                    showStickyHeader();
                }
            }
            catch (const std::exception &e)
            {
                std::cout << "\n  " << Styled("[✗]", Theme::Error) << "  "
                          << Styled(std::string("Exception: ") + e.what(), Theme::Highlight) << "\n\n";
            }
            catch (...)
            {
                std::cout << "\n  " << Styled("[✗]", Theme::Error) << "  "
                          << Styled("Unknown exception", Theme::Highlight) << "\n\n";
            }
        }
    };

    void showStickyHeader()
    {
        // Fixed-width ASCII art (54 chars per line).
        // Extra horizontal space from screen_width is split evenly as padding.
        static const std::vector<std::string> artLines = {
            "                                                      ",
            "     _____          _____                             ",
            "    / ___/__  ___  / ___/__  ___  ___  __ _  __ __    ",
            "   / /__/ _ \\/ _ \\/ /__/ _ \\/ _ \\/ _ \\/  ' \\/ // /    ",
            "   \\___/ .__/ .__/\\___/\\___/_//_/\\___/_/_/_/\\_, /     ",
            "      /_/  /_/                             /___/      ",
            "                                                      ",
            "                                                      "};
        static constexpr int artWidth = 54;

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

        // Layout math — all widths derived from screen_width
        // Total box: │ leftWidth │ rightWidth │  = screen_width
        //   borders: 1 + leftWidth + 1 + rightWidth + 1 = screen_width
        //   so:      leftWidth + rightWidth = screen_width - 3
        static constexpr int rightWidth = 35;
        const int leftWidth  = screen_width - rightWidth - 3;

        // Art padding: the art is artWidth chars; the content area inside the
        // left panel is leftWidth-1 (after the leading space).
        // Split extra evenly: more on the right to balance visual weight.
        const int artExtra   = (leftWidth - 1) - artWidth;
        const int artLPad    = artExtra / 2;
        const int artRPad    = artExtra - artLPad;
        const std::string lPad(artLPad > 0 ? artLPad : 0, ' ');
        const std::string rPad(artRPad > 0 ? artRPad : 0, ' ');

        // Header border
        std::cout << Styled(
            TopLeft + Repeat(Horizontal, 2) + " Economic Engine v2.0 " +
            Repeat(Horizontal, leftWidth - 24) + Horizontal +
            Repeat(Horizontal, rightWidth) + TopRight,
            Theme::Primary) << "\n";

        // Art + right-panel rows
        for (size_t i = 0; i < artLines.size(); ++i)
        {
            std::cout << Styled(Vertical, Theme::Primary)
                      << " " << lPad << Styled(artLines[i], Theme::Primary) << rPad
                      << Styled(Vertical, Color::Gray)
                      << " " << Styled(right[i], (i == 0) ? Theme::Primary : Theme::Secondary)
                      << Styled(Vertical, Theme::Primary) << "\n";
        }

        std::cout << Styled(
            BottomLeft + Repeat(Horizontal, leftWidth) + Horizontal +
            Repeat(Horizontal, rightWidth) + BottomRight,
            Theme::Primary) << "\n";

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

        // Slot widths: 4 boxes each with left+right border = screen_width total
        // Each box visual width = slotWidth + 2 (borders), so 4*(sw+2) = screen_width
        // → slotWidth = (screen_width - 8) / 4, remainder distributed left-to-right
        const int slotBase = (screen_width - 8) / 4;
        const int slotRem  = (screen_width - 8) % 4;
        const int sw1 = slotBase + (slotRem > 0 ? 1 : 0);
        const int sw2 = slotBase + (slotRem > 1 ? 1 : 0);
        const int sw3 = slotBase + (slotRem > 2 ? 1 : 0);
        const int sw4 = slotBase;

        std::vector<std::string> slot1 = createSlot("LABORER",
                                                    selLaborer ? selLaborer->name +
                                                                     "\nSkill Level: " + std::to_string((int)(selLaborer->skillLevel * 100)) + "%" +
                                                                     "\nMin Wage: " + std::to_string((int)selLaborer->minWage) + " Tk/day"
                                                               : "\nNone selected\n ",
                                                    sw1);

        std::vector<std::string> slot2 = createSlot("FARMER",
                                                    selFarmer ? selFarmer->name +
                                                                    "\nLand: " + std::to_string((int)selFarmer->land) + " acres" +
                                                                    "\nCrops: " + cropNames
                                                              : "\nNone selected\n ",
                                                    sw2);

        std::vector<std::string> slot3 = createSlot("CONSUMER",
                                                    selConsumer ? selConsumer->name +
                                                                      "\nAge: " + std::to_string(selConsumer->ageInDays / 365) + " years" +
                                                                      "\nSavings: Tk " + std::to_string((int)selConsumer->savings)
                                                                : "\nNone selected\n ",
                                                    sw3);

        std::vector<std::string> slot4 = createSlot("MARKET",
                                                    selMarket ? selMarket->prod->name +
                                                                    "\nPrice: Tk " + marketPriceText +
                                                                    "\n "
                                                              : "\nNone selected\n ",
                                                    sw4);

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

        // helpers mirroring executor style (inline since sH/kv live in cmdExec)
        auto fmtD = [](double v, int p = 2) {
            std::ostringstream os;
            os << std::fixed << std::setprecision(p) << v;
            return os.str();
        };
        auto padStr = [](const std::string &s, int w) {
            if ((int)s.size() >= w) return s;
            return s + std::string(w - (int)s.size(), ' ');
        };
        auto sH = [&](const std::string &title) {
            std::cout << "\n  " << Styled(title, Theme::BoldPrimary)
                      << "\n  " << Styled(Repeat(Horizontal, screen_width - 4), Theme::Muted) << "\n";
        };
        auto kv = [&](const std::string &key, const std::string &val) {
            std::cout << "    " << Styled(padStr(key, 24), Theme::Info)
                      << Styled(val, Theme::Highlight) << "\n";
        };
        auto hline = [&]() {
            std::cout << "  " << Styled(Repeat(Horizontal, screen_width - 4), Theme::Muted) << "\n";
        };
        auto section = [&](const std::string &label) {
            std::cout << "  " << Styled(label, Theme::Warning) << "\n";
        };

        sH("ECONOMIC STATUS");

        section("OUTPUT");
        kv("GDP",          "Tk " + fmtD(stats.gdp));
        double gdpPerCap = stats.population > 0 ? stats.gdp / stats.population : 0.0;
        kv("GDP per capita", "Tk " + fmtD(gdpPerCap));
        hline();

        section("MARKET PRICES");
        for (auto &m : simulation.markets)
        {
            if (m.price > 0.1)
            {
                std::string trendStr;
                if (m.priceHistory.size() > 1) {
                    double delta = m.price - m.priceHistory[m.priceHistory.size() - 2];
                    trendStr = delta > 0.5 ? Styled("  ▲", Theme::Warning) :
                               delta < -0.5 ? Styled("  ▼", Theme::Info) :
                               Styled("  ─", Theme::Muted);
                }
                kv(m.prod->name, "Tk " + fmtD(m.price) + trendStr);
            }
        }
        hline();

        section("LABOR MARKET");
        kv("Population",   std::to_string(stats.population));
        kv("Employed",     std::to_string(stats.employed));
        {
            double uPct = stats.unemployment * 100.0;
            const char *uColor = uPct < 5.0 ? Theme::Success
                               : uPct < 10.0 ? Theme::Warning
                               : Theme::Error;
            std::cout << "    " << Styled(padStr("Unemployment", 24), Theme::Info)
                      << Styled(fmtD(uPct) + "%", uColor) << "\n";
        }
        kv("Firms",        std::to_string(stats.firms));
        hline();

        section("WEALTH");
        kv("Money Supply", "Tk " + fmtD(stats.moneySupply));
        std::cout << "\n";
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
            std::cout << "\n  " << Styled("[✗]", Theme::Error) << "  "
                      << Styled(executor.getLastError(), Theme::Highlight) << "\n\n";
        }

        std::cout << "\n";
    }

    void handleOutput(const std::string &message)
    {
        if (message.find("Error:") == 0 || message.find("[✗]") != std::string::npos)
        {
            std::cout << "\n  " << Styled("[✗]", Theme::Error) << "  "
                      << Styled(message.find("Error:") == 0 ? message.substr(6) : message,
                                Theme::Highlight)
                      << "\n\n";
        }
        else
        {
            // Executor handles its own styled output for most commands.
            // This path handles plain informational strings.
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
        }
#else
        // Unix/Linux interactive input with history + autocomplete.
        if (!isatty(STDIN_FILENO))
        {
            std::string input;
            std::getline(std::cin, input);
            return input;
        }

        struct TerminalModeGuard
        {
            termios original{};
            bool active = false;

            TerminalModeGuard()
            {
                if (tcgetattr(STDIN_FILENO, &original) != 0)
                    return;

                termios raw = original;
                raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
                raw.c_iflag &= static_cast<tcflag_t>(~(IXON | ICRNL));
                raw.c_cc[VMIN] = 1;
                raw.c_cc[VTIME] = 0;

                if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0)
                    active = true;
            }

            ~TerminalModeGuard()
            {
                if (active)
                {
                    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
                }
            }
        } termGuard;

        if (!termGuard.active)
        {
            std::string input;
            std::getline(std::cin, input);
            return input;
        }

        auto readByte = []() -> int
        {
            unsigned char c = 0;
            ssize_t n = ::read(STDIN_FILENO, &c, 1);
            return (n == 1) ? static_cast<int>(c) : -1;
        };

        std::string input;
        std::vector<std::string> suggestions;
        int historyPos = static_cast<int>(history.size());
        size_t cursorPos = 0;
        int displayedSuggestionCount = 0;

        std::cout << "\n\n\n\n";
        std::cout << "\033[4A";
        std::cout << "\033[2K";
        std::cout << Prompt();
        std::cout.flush();

        auto refreshSuggestions = [&](const std::vector<std::string> &lines)
        {
            int newCount = static_cast<int>(lines.size());
            int oldCount = displayedSuggestionCount;
            int maxLines = (newCount > oldCount) ? newCount : oldCount;

            if (maxLines == 0)
            {
                displayedSuggestionCount = 0;
                return;
            }

            for (int i = 0; i < maxLines; ++i)
            {
                std::cout << "\033[1E";
                std::cout << "\033[2K";

                if (i < newCount)
                {
                    std::cout << lines[i];
                    std::cout << "\033[2m";
                }
            }

            std::cout << "\033[" << maxLines << "F";
            std::cout << "\033[2K";
            std::cout << Prompt();
            std::cout << Styled(input, Theme::Primary);

            size_t charsAfterCursor = input.length() - cursorPos;
            if (charsAfterCursor > 0)
            {
                std::cout << "\033[" << charsAfterCursor << "D";
            }

            std::cout.flush();
            displayedSuggestionCount = newCount;
        };

        auto redrawSuggestionsForInput = [&]()
        {
            suggestions = GetSuggestions(input);
            if (!suggestions.empty() && !input.empty())
            {
                std::vector<std::string> lines;
                int show = std::min<int>(4, static_cast<int>(suggestions.size()));
                for (int i = 0; i < show; ++i)
                {
                    lines.push_back(Styled(suggestions[i], Color::Dim));
                }
                refreshSuggestions(lines);
            }
            else
            {
                refreshSuggestions({});
            }
        };

        while (true)
        {
            int code = readByte();
            if (code < 0)
                continue;

            char ch = static_cast<char>(code);

            if (ch == 3)
            {
                running = false;
                refreshSuggestions({});
                std::cout << "ctrl + c\n"
                          << Styled("Goodbye!\n", Theme::Success);
                exit(0);
            }

            if (ch == '\r' || ch == '\n')
            {
                refreshSuggestions({});
                return input;
            }
            else if (ch == '\b' || ch == 127)
            {
                if (!input.empty() && cursorPos > 0)
                {
                    if (cursorPos == input.length())
                    {
                        input.pop_back();
                        cursorPos = input.length();
                        std::cout << "\b \b";
                    }
                    else
                    {
                        input.erase(cursorPos - 1, 1);
                        std::cout << "\033[D";
                        cursorPos--;
                        std::cout << "\033[K";
                        std::cout << input.substr(cursorPos);
                        for (size_t m = 0; m < input.length() - cursorPos; ++m)
                            std::cout << "\033[D";
                    }

                    redrawSuggestionsForInput();
                }
            }
            else if (ch == '\t')
            {
                suggestions = GetSuggestions(input);
                if (!suggestions.empty())
                {
                    if (cursorPos < input.length())
                    {
                        for (size_t mv = 0; mv < input.length() - cursorPos; ++mv)
                            std::cout << "\033[C";
                        cursorPos = input.length();
                    }

                    for (size_t i = 0; i < input.length(); ++i)
                        std::cout << "\b \b";

                    std::string suggestion = suggestions[0];
                    size_t parenPos = suggestion.find('(');
                    std::string fill = (parenPos != std::string::npos) ? suggestion.substr(0, parenPos + 1) : suggestion;

                    input = fill;
                    std::cout << input;
                    cursorPos = input.length();

                    redrawSuggestionsForInput();
                }
            }
            else if (ch == 27)
            {
                int seq1 = readByte();
                if (seq1 != '[')
                    continue;

                int seq2 = readByte();
                if (seq2 < 0)
                    continue;

                // Up arrow
                if (seq2 == 'A' && !history.empty())
                {
                    if (historyPos > 0)
                        historyPos--;

                    if (cursorPos < input.length())
                    {
                        for (size_t mv = 0; mv < input.length() - cursorPos; ++mv)
                            std::cout << "\033[C";
                    }

                    for (size_t i = 0; i < input.length(); ++i)
                        std::cout << "\b \b";

                    if (historyPos >= 0 && historyPos < static_cast<int>(history.size()))
                    {
                        input = history[static_cast<size_t>(historyPos)];
                        std::cout << input;
                        cursorPos = input.length();
                        redrawSuggestionsForInput();
                    }
                }
                // Down arrow
                else if (seq2 == 'B' && !history.empty())
                {
                    if (historyPos < static_cast<int>(history.size()))
                        historyPos++;

                    if (cursorPos < input.length())
                    {
                        for (size_t mv = 0; mv < input.length() - cursorPos; ++mv)
                            std::cout << "\033[C";
                    }

                    for (size_t i = 0; i < input.length(); ++i)
                        std::cout << "\b \b";

                    if (historyPos >= 0 && historyPos < static_cast<int>(history.size()))
                    {
                        input = history[static_cast<size_t>(historyPos)];
                        std::cout << input;
                        cursorPos = input.length();
                        redrawSuggestionsForInput();
                    }
                    else
                    {
                        input.clear();
                        cursorPos = 0;
                        refreshSuggestions({});
                    }
                }
                // Left arrow
                else if (seq2 == 'D')
                {
                    if (cursorPos > 0)
                    {
                        std::cout << "\033[D";
                        cursorPos--;
                    }
                }
                // Right arrow
                else if (seq2 == 'C')
                {
                    if (cursorPos < input.length())
                    {
                        std::cout << "\033[C";
                        cursorPos++;
                    }
                }
            }
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
                    input.insert(input.begin() + static_cast<std::string::difference_type>(cursorPos), ch);
                    std::cout << "\033[K";
                    std::cout << Styled(input.substr(cursorPos), Theme::Primary);

                    size_t tailLen = input.length() - cursorPos - 1;
                    for (size_t m = 0; m < tailLen; ++m)
                        std::cout << "\033[D";
                    cursorPos++;
                }

                redrawSuggestionsForInput();
            }
        }
#endif
    }
};