#pragma once

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <sstream>
#include <iomanip>

enum class LogLevel
{
    INFO,
    WARNING,
    ALERT,
    SUCCESS
};

struct LogEntry
{
    LogLevel level;
    std::string message;
    std::string timestamp;
    int tick;
};

class Logger
{
public:
    using LogCallback = std::function<void(const LogEntry &)>;

    static Logger &getInstance()
    {
        static Logger instance;
        return instance;
    }

    void setCallback(LogCallback callback)
    {
        m_callback = callback;
    }

    void setCurrentTick(int tick)
    {
        m_currentTick = tick;
    }

    void log(LogLevel level, const std::string &message)
    {
        LogEntry entry;
        entry.level = level;
        entry.message = message;
        entry.timestamp = getCurrentTimestamp();
        entry.tick = m_currentTick;

        m_entries.push_back(entry);

        if (m_callback)
        {
            m_callback(entry);
        }
    }

    void info(const std::string &message)
    {
        log(LogLevel::INFO, message);
    }

    void warning(const std::string &message)
    {
        log(LogLevel::WARNING, message);
    }

    void alert(const std::string &message)
    {
        log(LogLevel::ALERT, message);
    }

    void success(const std::string &message)
    {
        log(LogLevel::SUCCESS, message);
    }

    const std::vector<LogEntry> &getEntries() const
    {
        return m_entries;
    }

    void clear()
    {
        m_entries.clear();
    }

private:
    Logger() : m_currentTick(0) {}
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    std::string getCurrentTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%H:%M:%S");
        return ss.str();
    }

    std::vector<LogEntry> m_entries;
    LogCallback m_callback;
    int m_currentTick;
};

// Convenience macros
#define LOG_INFO(msg) Logger::getInstance().info(msg)
#define LOG_WARNING(msg) Logger::getInstance().warning(msg)
#define LOG_ALERT(msg) Logger::getInstance().alert(msg)
#define LOG_SUCCESS(msg) Logger::getInstance().success(msg)
