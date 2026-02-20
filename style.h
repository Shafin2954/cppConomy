#pragma once

#include <cmath>
#include <string>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace styledTerminal
{
    // Initialize terminal for color support (Windows)
    inline void Init()
    {
// todo: remove and run on wsl
#ifdef _WIN32
        // Enable ANSI colors and UTF-8 on Windows 10+
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);

        // Set console to UTF-8
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
#endif
    }

    // ANSI Color codes
    namespace Color
    {
        // constexpr for compile-time constants (no memory allocation)
        constexpr const char *Reset = "\033[0m"; // \033 to escape and start ansi command
        constexpr const char *Bold = "\033[1m";
        constexpr const char *Dim = "\033[2m";
        constexpr const char *Italic = "\033[3m";
        constexpr const char *Underline = "\033[4m";

        // Foreground colors
        constexpr const char *Black = "\033[30m";
        constexpr const char *Red = "\033[31m";
        constexpr const char *Green = "\033[32m";
        constexpr const char *Yellow = "\033[33m";
        constexpr const char *Blue = "\033[34m";
        constexpr const char *Magenta = "\033[35m";
        constexpr const char *Cyan = "\033[36m";
        constexpr const char *White = "\033[37m";
        constexpr const char *Gray = "\033[90m";

        // Bright foreground colors
        constexpr const char *BrightBlack = "\033[90m";
        constexpr const char *BrightRed = "\033[91m";
        constexpr const char *BrightGreen = "\033[92m";
        constexpr const char *BrightYellow = "\033[93m";
        constexpr const char *BrightBlue = "\033[94m";
        constexpr const char *BrightMagenta = "\033[95m";
        constexpr const char *BrightCyan = "\033[96m";
        constexpr const char *BoldBrightCyan = "\033[1m\033[96m";
        constexpr const char *BrightWhite = "\033[97m";
    }

    // Themed color palette (similar to Claude CLI)
    namespace Theme
    {
        constexpr const char *Primary = Color::BrightGreen;
        constexpr const char *BoldPrimary = Color::BoldBrightCyan;
        constexpr const char *Secondary = Color::Green;
        constexpr const char *Success = Color::BrightGreen;
        constexpr const char *Warning = Color::BrightYellow;
        constexpr const char *Error = Color::BrightRed;
        constexpr const char *Info = Color::Cyan;
        constexpr const char *Muted = Color::BrightBlack;
        constexpr const char *Highlight = Color::BrightWhite;
    }

    // Box drawing characters (Unicode rounded corners like Claude)
    namespace Box
    {
        constexpr const char *TopLeft = "╭";
        constexpr const char *TopRight = "╮";
        constexpr const char *BottomLeft = "╰";
        constexpr const char *BottomRight = "╯";
        constexpr const char *Horizontal = "─";
        constexpr const char *Vertical = "│";
        constexpr const char *VerticalRight = "├";
        constexpr const char *VerticalLeft = "┤";
        constexpr const char *HorizontalDown = "┬";
        constexpr const char *HorizontalUp = "┴";
        constexpr const char *Cross = "┼";
        constexpr const char *Separator = "─";
    }

    // Utility functions for styled output                               ╭ convert char to std::string
    inline std::string Styled(const std::string &text, const char *style) { return std::string(style) + text + Color::Reset; } // ansiCode + text + reset

    // Helper to repeat a std::string n times
    inline std::string Repeat(const std::string &str, size_t count)
    {
        std::string result;
        result.reserve(str.length() * count);
        for (size_t i = 0; i < count; ++i)
            result += str;
        return result;
    }

    inline std::string Success(const std::string &text) { return Styled("[OK] " + text, Theme::Success); }
    inline std::string Error(const std::string &text) { return Styled("[X] " + text, Theme::Error); }
    inline std::string Warning(const std::string &text) { return Styled("[!] " + text, Theme::Warning); }
    inline std::string Info(const std::string &text) { return Styled("[i] " + text, Theme::Info); }
    inline std::string Prompt() { return Styled("❯", Theme::Primary) + " "; }
    inline std::string Header(const std::string &text) { return Styled(text, std::string(Color::Bold).append(Theme::Primary).c_str()); }

    // Create a styled box
    inline std::string BoxedText(const std::string &content, const std::string &title = "")
    {
        std::stringstream ss;
        size_t width = 70;

        // Top border
        ss << Styled(Box::TopLeft, Theme::Primary);
        if (!title.empty())
        {
            std::string titleStr = " " + title + " ";
            ss << Styled(Repeat(Box::Horizontal, 2), Theme::Primary);
            ss << Styled(titleStr, Theme::Highlight);
            ss << Styled(Repeat(Box::Horizontal, width - titleStr.length() - 4), Theme::Primary);
        }
        else
        {
            ss << Styled(Repeat(Box::Horizontal, width - 2), Theme::Primary);
        }
        ss << Styled(Box::TopRight, Theme::Primary) << "\n";

        // Content | line |
        std::istringstream contentStream(content);
        std::string line;
        while (getline(contentStream, line))
        {
            ss << Styled(Box::Vertical, Theme::Primary) << " "
               << line
               << std::string(width - line.length() - 3, ' ')
               << Styled(Box::Vertical, Theme::Primary) << "\n";
        }

        // Bottom border
        ss << Styled(Box::BottomLeft, Theme::Primary)
           << Styled(Repeat(Box::Horizontal, width - 2), Theme::Primary)
           << Styled(Box::BottomRight, Theme::Primary) << "\n";

        return ss.str();
    }

    // Create a horizontal separator
    inline std::string Separator(size_t width = 93) { return Styled(Repeat(Box::Horizontal, width), Theme::Primary); }

    // Format a key-value pair
    inline std::string KeyValue(const std::string &key, const std::string &value) { return Styled(key, Theme::Info) + Styled(": ", Theme::Muted) + value; }

    inline double twoDecimal(double value) { return std::round(value * 100.0) / 100.0; }
}
