#pragma once

#include <string>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#endif

// ============================================================================
// TermColors.h - Terminal color and styling utilities
//
// Provides ANSI color codes and styling utilities for creating a beautiful CLI
// Similar to modern CLIs like Claude Code
// ============================================================================

namespace TermColors
{
    // Initialize terminal for color support (Windows)
    inline void Init()
    {
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
        constexpr const char *Reset = "\033[0m";
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

        // Bright foreground colors
        constexpr const char *BrightBlack = "\033[90m";
        constexpr const char *BrightRed = "\033[91m";
        constexpr const char *BrightGreen = "\033[92m";
        constexpr const char *BrightYellow = "\033[93m";
        constexpr const char *BrightBlue = "\033[94m";
        constexpr const char *BrightMagenta = "\033[95m";
        constexpr const char *BrightCyan = "\033[96m";
        constexpr const char *BrightWhite = "\033[97m";
    }

    // Themed color palette (similar to Claude CLI)
    namespace Theme
    {
        constexpr const char *Primary = Color::BrightCyan;
        constexpr const char *Secondary = Color::BrightBlue;
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

    // Utility functions for styled output
    inline string Styled(const string &text, const char *style)
    {
        return string(style) + text + Color::Reset;
    }

    // Helper to repeat a string n times
    inline string Repeat(const string &str, size_t count)
    {
        string result;
        result.reserve(str.length() * count);
        for (size_t i = 0; i < count; ++i)
            result += str;
        return result;
    }

    inline string Success(const string &text)
    {
        return Styled("[OK] " + text, Theme::Success);
    }

    inline string Error(const string &text)
    {
        return Styled("[X] " + text, Theme::Error);
    }

    inline string Warning(const string &text)
    {
        return Styled("[!] " + text, Theme::Warning);
    }

    inline string Info(const string &text)
    {
        return Styled("[i] " + text, Theme::Info);
    }

    inline string Prompt()
    {
        return Styled("❯", Theme::Primary) + " ";
    }

    inline string Header(const string &text)
    {
        return Styled(text, string(Color::Bold).append(Theme::Primary).c_str());
    }

    // Create a styled box
    inline string BoxedText(const string &content, const string &title = "")
    {
        stringstream ss;
        size_t width = 70;

        // Top border
        ss << Styled(Box::TopLeft, Theme::Primary);
        if (!title.empty())
        {
            string titleStr = " " + title + " ";
            ss << Styled(Repeat(Box::Horizontal, 2), Theme::Primary);
            ss << Styled(titleStr, Theme::Highlight);
            ss << Styled(Repeat(Box::Horizontal, width - titleStr.length() - 4), Theme::Primary);
        }
        else
        {
            ss << Styled(Repeat(Box::Horizontal, width - 2), Theme::Primary);
        }
        ss << Styled(Box::TopRight, Theme::Primary) << "\n";

        // Content
        istringstream contentStream(content);
        string line;
        while (getline(contentStream, line))
        {
            ss << Styled(Box::Vertical, Theme::Primary) << " "
               << line
               << string(width - line.length() - 3, ' ')
               << Styled(Box::Vertical, Theme::Primary) << "\n";
        }

        // Bottom border
        ss << Styled(Box::BottomLeft, Theme::Primary)
           << Styled(Repeat(Box::Horizontal, width - 2), Theme::Primary)
           << Styled(Box::BottomRight, Theme::Primary) << "\n";

        return ss.str();
    }

    // Create a horizontal separator
    inline string Separator(size_t width = 70)
    {
        return Styled(Repeat(Box::Horizontal, width), Theme::Muted);
    }

    // Format a key-value pair
    inline string KeyValue(const string &key, const string &value)
    {
        return Styled(key, Theme::Info) + Styled(": ", Theme::Muted) + value;
    }
}
