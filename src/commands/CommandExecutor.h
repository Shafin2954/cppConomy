#pragma once

#include "CommandParser.h"
#include "../core/Economy.h"
#include <functional>
#include <sstream>

class CommandExecutor
{
public:
    using OutputCallback = std::function<void(const std::string &)>;

    CommandExecutor(Economy &economy);

    // Execute a parsed command
    bool execute(const Command &cmd);

    // Execute from string input
    bool execute(const std::string &input);

    // Set output callback for results
    void setOutputCallback(OutputCallback callback) { m_outputCallback = callback; }

    // Get last error message
    const std::string &getLastError() const { return m_lastError; }

private:
    // Command handlers
    void cmdAdjustTax(const Command &cmd);
    void cmdSetInterest(const Command &cmd);
    void cmdInjectMoney(const Command &cmd);
    void cmdGrantStimulus(const Command &cmd);
    void cmdPrintStats(const Command &cmd);
    void cmdSimulate(const Command &cmd);
    void cmdPause(const Command &cmd);
    void cmdResume(const Command &cmd);
    void cmdTriggerShock(const Command &cmd);
    void cmdAddFirms(const Command &cmd);
    void cmdAddCitizens(const Command &cmd);
    void cmdExportData(const Command &cmd);
    void cmdHelp(const Command &cmd);
    void cmdClear(const Command &cmd);
    void cmdReset(const Command &cmd);
    void cmdStatus(const Command &cmd);

    // Helper to get parameter values
    template <typename T>
    T getParam(const Command &cmd, const std::string &name, T defaultValue) const;

    void output(const std::string &message);

    Economy &m_economy;
    CommandParser m_parser;
    OutputCallback m_outputCallback;
    std::string m_lastError;
};
