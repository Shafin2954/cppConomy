#pragma once
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <thread>
#include <chrono>

#include "consumer.h"
#include "laborer.h"
#include "farmer.h"
#include "firm.h"
#include "market.h"
#include "world.h"
#include "cmd.h"
#include "style.h"

using namespace styledTerminal;
using namespace Box;
class cli; // Forward declaration

class cmdExec
{
public:
    using OutputCallback = std::function<void(const std::string &)>;
    using RefreshHeaderCallback = std::function<void()>;

    cmdExec(world &simulation, OutputCallback outputFunc, RefreshHeaderCallback refreshHeaderFunc = {}, int screenWidth = 93)
        : simulation(simulation), outputCallback(outputFunc), refreshHeaderCallback(refreshHeaderFunc), sw(screenWidth) {};

    // Execute a parsed command
    bool execute(const Command &cmd)
    {
        if (!cmd.valid)
        {
            lastError = cmd.errorMessage;
            output("Error: " + lastError);
            return false;
        }

        try
        {
            // Handle property assignments first
            if (cmd.commandType == Command::Type::Assignment)
            {
                return executeAssignment(cmd);
            }

            if (cmd.name == "consumers")
                cmdConsumers(cmd);
            else if (cmd.name == "laborers")
                cmdLaborers(cmd);
            else if (cmd.name == "farmers")
                cmdFarmers(cmd);
            else if (cmd.name == "firms")
                cmdFirms(cmd);
            else if (cmd.name == "markets")
                cmdMarkets(cmd);
            else if (cmd.name == "products")
                cmdProducts(cmd);
            else if (cmd.name == "add_consumer")
                cmdAddConsumer(cmd);
            else if (cmd.name == "add_laborer")
                cmdAddLaborer(cmd);
            else if (cmd.name == "add_farmer")
                cmdAddFarmer(cmd);
            else if (cmd.name == "add_firm")
                cmdAddFirm(cmd);
            else if (cmd.name == "select_consumer")
                cmdSelectConsumer(cmd);
            else if (cmd.name == "select_laborer")
                cmdSelectLaborer(cmd);
            else if (cmd.name == "select_farmer")
                cmdSelectFarmer(cmd);
            else if (cmd.name == "select_market")
                cmdSelectMarket(cmd);
            else if (cmd.name == "clear_selection")
                cmdClearSelection(cmd);
            else if (cmd.name == "consumer_mu")
                cmdConsumerMU(cmd);
            else if (cmd.name == "consumer_surplus")
                cmdConsumerSurplus(cmd);
            else if (cmd.name == "consumer_details")
                simulation.selected_consumer != nullptr ? output(simulation.selected_consumer->getStyledDetails()) : output("No consumer selected");
            else if (cmd.name == "consumer_substitution")
                cmdConsumerSubstitution(cmd);
            else if (cmd.name == "consumer_needs")
                cmdConsumerNeeds(cmd);
            else if (cmd.name == "consumer_demand_curve")
                cmdConsumerDemandCurve(cmd);
            else if (cmd.name == "kill_consumer")
                cmdKillConsumer(cmd);
            else if (cmd.name == "farmer_supply")
                cmdFarmerSupply(cmd);
            else if (cmd.name == "farmer_details")
                simulation.selected_farmer != nullptr ? output(simulation.selected_farmer->getStyledDetails()) : output("No farmer selected");
            else if (cmd.name == "farmer_crops")
                cmdFarmerCrops(cmd);
            else if (cmd.name == "farmer_upgrade")
                cmdFarmerUpgrade(cmd);
            else if (cmd.name == "farmer_tax")
                cmdFarmerTax(cmd);
            else if (cmd.name == "farmer_weather")
                cmdFarmerWeather(cmd);
            else if (cmd.name == "kill_farmer")
                cmdKillFarmer(cmd);
            else if (cmd.name == "laborer_details")
                simulation.selected_laborer != nullptr ? output(simulation.selected_laborer->getStyledDetails()) : output("No laborer selected");
            else if (cmd.name == "kill_laborer")
                cmdKillLaborer(cmd);
            else if (cmd.name == "firm_costs")
                cmdFirmCosts(cmd);
            else if (cmd.name == "firm_output")
                cmdFirmOutput(cmd);
            else if (cmd.name == "firm_mp")
                cmdFirmMP(cmd);
            else if (cmd.name == "firm_efficiency")
                cmdFirmEfficiency(cmd);
            else if (cmd.name == "firm_details")
                simulation.selected_firm != nullptr ? output(simulation.selected_firm->getStyledDetails()) : output("No firm selected");
            else if (cmd.name == "firm_hire")
                cmdFirmHire(cmd);
            else if (cmd.name == "firm_fire")
                cmdFirmFire(cmd);
            else if (cmd.name == "firm_capital")
                cmdFirmCapital(cmd);
            else if (cmd.name == "market_details")
                simulation.selected_market != nullptr ? output(simulation.selected_market->getStyledDetails()) : output("No market selected");
            else if (cmd.name == "market_history")
                cmdMarketHistory(cmd);
            else if (cmd.name == "pass_day")
                cmdPassDay(cmd);
            else if (cmd.name == "set_income")
                cmdSetIncome(cmd);
            else if (cmd.name == "status")
                cmdStatus(cmd);
            else if (cmd.name == "help")
                cmdHelp(cmd);
            else if (cmd.name == "clear")
                cmdClear();
            else if (cmd.name == "exit")
            {
                output("Exiting simulation...");
                return true;
            }
            else
            {
                lastError = "Unknown command: " + cmd.name;
                output("Error: " + lastError);
                return false;
            }
            return true;
        }
        catch (const std::bad_variant_access &e)
        {
            lastError = std::string("Parameter type mismatch: ") + cmd.name + " " + e.what();
            output("Error: " + lastError);
            return false;
        }
        catch (const std::out_of_range &e)
        {
            lastError = std::string("Parameter access error: ") + e.what();
            output("Error: " + lastError);
            return false;
        }
        catch (const std::exception &e)
        {
            lastError = e.what();
            output("Error: " + lastError);
            return false;
        }
        catch (...)
        {
            lastError = "Unknown error occurred";
            output("Error: " + lastError);
            return false;
        }
    }

    // Execute from std::string input
    bool execute(const std::string &input)
    {
        Command cmd = parser.parse(input);
        return execute(cmd);
    }

    // Set output callback for results
    void setOutputCallback(OutputCallback callback) { outputCallback = callback; }
    void setRefreshHeaderCallback(RefreshHeaderCallback callback) { refreshHeaderCallback = callback; }

    // Get last error message
    const std::string &getLastError() const { return lastError; }

    // Get parser for command info
    const CommandParser &getParser() const { return parser; }

    // Property assignment handler
    bool executeAssignment(const Command &cmd)
    {
        return assignSystemProperty(cmd);
    }

    // Assignment handlers
    bool assignSystemProperty(const Command &cmd)
    {
        if (cmd.assignmentProperty == "gdp")
        {
            double gdpValue = 0.0;
            bool ok = true;
            if (auto pv = std::get_if<double>(&cmd.assignmentValue))
                gdpValue = *pv;
            else if (auto pi = std::get_if<int>(&cmd.assignmentValue))
                gdpValue = static_cast<double>(*pi);
            else if (auto ps = std::get_if<std::string>(&cmd.assignmentValue))
            {
                try
                {
                    gdpValue = std::stod(*ps);
                }
                catch (...)
                {
                    ok = false;
                }
            }
            else if (auto pb = std::get_if<bool>(&cmd.assignmentValue))
                gdpValue = *pb ? 1.0 : 0.0;
            else
                ok = false;

            if (!ok)
            {
                output(Styled("[✗]", Theme::Error) + " Invalid gdp value");
                return false;
            }
            simulation.currentStats.gdp = gdpValue;
            std::cout << "\n  " << Styled("[✓]", Theme::Success)
                      << "  GDP set to  " << Styled("Tk " + fmtD(gdpValue), Theme::Highlight) << "\n\n";
            return true;
        }
        output(Styled("[✗]", Theme::Error) + " Unknown property: " + cmd.assignmentProperty);
        return false;
    }

    bool hasParam(const Command &cmd, const std::string &name) const
    {
        return cmd.params.find(name) != cmd.params.end();
    }

    void output(const std::string &message)
    {
        if (outputCallback)
            outputCallback(message);
    }

    void requestHeaderRefresh()
    {
        if (refreshHeaderCallback)
            refreshHeaderCallback();
    }

    world &simulation;
    CommandParser parser;
    OutputCallback outputCallback;
    RefreshHeaderCallback refreshHeaderCallback;
    int sw = 93; // screen width — set from cli::screen_width via constructor
    std::string lastError;

    // ── Shared visual helpers ────────────────────────────────────────────────

    // Format double to fixed decimal string
    std::string fmtD(double v, int precision = 2) const
    {
        std::ostringstream os;
        os << std::fixed << std::setprecision(precision) << v;
        return os.str();
    }

    // Pad or truncate string to fixed width
    std::string padStr(const std::string &s, int w) const
    {
        if ((int)s.size() >= w)
            return s;
        return s + std::string(w - (int)s.size(), ' ');
    }

    // Section header  "  TITLE  ·  subtitle"  followed by a thin rule
    void sH(const std::string &title, const std::string &sub = "") const
    {
        std::cout << "\n  " << Styled(title, Theme::BoldPrimary);
        if (!sub.empty())
            std::cout << Styled("  ·  ", Theme::Muted)
                      << Styled(sub, Theme::Warning);
        std::cout << "\n  " << Styled(Repeat(Horizontal, sw - 4), Theme::Muted) << "\n";
    }

    // Key-value row:  "    Key                 Value"
    void kv(const std::string &key, const std::string &val, int kw = 24) const
    {
        std::cout << "    "
                  << Styled(padStr(key, kw), Theme::Info)
                  << Styled(val, Theme::Highlight) << "\n";
    }

    // Key-value with trailing muted note
    void kvNote(const std::string &key, const std::string &val,
                const std::string &note_text, int kw = 24) const
    {
        std::cout << "    "
                  << Styled(padStr(key, kw), Theme::Info)
                  << Styled(val, Theme::Highlight)
                  << "  " << Styled(note_text, Theme::Muted) << "\n";
    }

    // Sub-entity label inside a section  "  ▸ Name"
    void entLabel(const std::string &name) const
    {
        std::cout << "\n  " << Styled("  ▸ " + name, Theme::Warning) << "\n";
    }

    // Thin horizontal rule
    void hline(int w = 62) const
    {
        std::cout << "  " << Styled(Repeat(Horizontal, w), Theme::Muted) << "\n";
    }

    // Blank line
    void bln() const { std::cout << "\n"; }

    // Muted note line
    void noteText(const std::string &t) const
    {
        std::cout << "    " << Styled(t, Theme::Muted) << "\n";
    }

    // Success confirmation  "    ✓  text"
    void successNote(const std::string &t) const
    {
        std::cout << "\n  " << Styled("  ✓  " + t, Theme::Success) << "\n\n";
    }

    // Warning / recommendation line  "    !  text"
    void warnNote(const std::string &t) const
    {
        std::cout << "    " << Styled("!  " + t, Theme::Warning) << "\n";
    }

    // Economic formula row  "    Label                P = c - mQ"
    void eqRow(const std::string &label, const std::string &eq, int kw = 24) const
    {
        std::cout << "    "
                  << Styled(padStr(label, kw), Theme::Info)
                  << Styled(eq, Theme::Secondary) << "\n";
    }

    // ── Parameter retrieval (robust conversion) ──────────────────────────────
    template <typename T>
    T getParam(const Command &cmd, const std::string &name, T defaultValue) const
    {
        auto it = cmd.params.find(name);
        if (it == cmd.params.end())
            return defaultValue;

        const ParamValue &v = it->second;

        if constexpr (std::is_same_v<T, double>)
        {
            if (auto pv = std::get_if<double>(&v))
                return *pv;
            if (auto pi = std::get_if<int>(&v))
                return static_cast<double>(*pi);
            if (auto ps = std::get_if<std::string>(&v))
            {
                try
                {
                    return std::stod(*ps);
                }
                catch (...)
                {
                    return defaultValue;
                }
            }
            if (auto pb = std::get_if<bool>(&v))
                return *pb ? 1.0 : 0.0;
        }
        if constexpr (std::is_same_v<T, int>)
        {
            if (auto pi = std::get_if<int>(&v))
                return *pi;
            if (auto pd = std::get_if<double>(&v))
                return static_cast<int>(*pd);
            if (auto ps = std::get_if<std::string>(&v))
            {
                try
                {
                    return static_cast<int>(std::stoi(*ps));
                }
                catch (...)
                {
                    return defaultValue;
                }
            }
            if (auto pb = std::get_if<bool>(&v))
                return *pb ? 1 : 0;
        }
        if constexpr (std::is_same_v<T, std::string>)
        {
            if (auto ps = std::get_if<std::string>(&v))
                return *ps;
            if (auto pi = std::get_if<int>(&v))
                return std::to_string(*pi);
            if (auto pd = std::get_if<double>(&v))
                return std::to_string(*pd);
            if (auto pb = std::get_if<bool>(&v))
                return *pb ? "true" : "false";
        }
        if constexpr (std::is_same_v<T, bool>)
        {
            if (auto pb = std::get_if<bool>(&v))
                return *pb;
            if (auto pi = std::get_if<int>(&v))
                return *pi != 0;
            if (auto pd = std::get_if<double>(&v))
                return *pd != 0.0;
            if (auto ps = std::get_if<std::string>(&v))
                return (*ps == "true" || *ps == "1");
        }

        return defaultValue;
    }

    // ========== COMMAND IMPLEMENTATIONS ==========

    // ── LIST COMMANDS ──────────────────────────────────────────────────────

    void cmdConsumers(const Command &cmd)
    {
        sH("CONSUMERS", std::to_string(simulation.consumers.size()) + " entities");
        for (auto &c : simulation.consumers)
        {
            std::cout << "  " << Styled("▸", Theme::Primary) << "  "
                      << Styled(padStr(c.name, 14), Theme::Highlight)
                      << Styled("id ", Theme::Muted) << Styled(padStr(std::to_string(c.id), 5), Theme::Warning)
                      << Styled("age ", Theme::Muted) << Styled(padStr(std::to_string(c.ageInDays / 365) + "y", 6), Theme::Highlight)
                      << Styled("savings ", Theme::Muted) << Styled("Tk " + fmtD(c.savings), Theme::Highlight)
                      << "  "
                      << (c.isAlive ? Styled("● alive", Theme::Success) : Styled("● dead", Theme::Error))
                      << "\n";
        }
        bln();
    }

    void cmdLaborers(const Command &cmd)
    {
        sH("LABORERS", std::to_string(simulation.laborers.size()) + " entities");
        for (auto &l : simulation.laborers)
        {
            std::cout << "  " << Styled("▸", Theme::Primary) << "  "
                      << Styled(padStr(l.name, 14), Theme::Highlight)
                      << Styled("id ", Theme::Muted) << Styled(padStr(std::to_string(l.id), 5), Theme::Warning)
                      << Styled("skill ", Theme::Muted) << Styled(padStr(std::to_string((int)(l.skillLevel * 100)) + "%", 6), Theme::Highlight)
                      << Styled("min wage ", Theme::Muted) << Styled("Tk " + fmtD(l.minWage) + "/day", Theme::Highlight)
                      << "\n";
        }
        bln();
    }

    void cmdFarmers(const Command &cmd)
    {
        sH("FARMERS", std::to_string(simulation.farmers.size()) + " entities");
        for (auto &f : simulation.farmers)
        {
            std::string cropList;
            for (size_t i = 0; i < f.crops.size(); i++)
                cropList += (i ? ", " : "") + f.crops[i].name;

            std::cout << "  " << Styled("▸", Theme::Primary) << "  "
                      << Styled(padStr(f.name, 14), Theme::Highlight)
                      << Styled("id ", Theme::Muted) << Styled(padStr(std::to_string(f.id), 5), Theme::Warning)
                      << Styled("land ", Theme::Muted) << Styled(padStr(fmtD(f.land) + " ac", 9), Theme::Highlight)
                      << Styled("tech ", Theme::Muted) << Styled(padStr(std::to_string((int)(f.techLevel * 100)) + "%", 6), Theme::Highlight)
                      << Styled("crops ", Theme::Muted) << Styled(cropList, Theme::Secondary)
                      << "\n";
        }
        bln();
    }

    void cmdFirms(const Command &cmd)
    {
        sH("FIRMS", std::to_string(simulation.firms.size()) + " firms");
        for (auto &f : simulation.firms)
        {
            // Find owner name
            std::string ownerName = "#" + std::to_string(f.ownerId);
            for (auto &c : simulation.consumers)
                if (c.id == f.ownerId)
                {
                    ownerName = c.name;
                    break;
                }

            std::cout << "  " << Styled("▸", Theme::Primary) << "  "
                      << Styled(padStr(ownerName + "'s firm", 18), Theme::Highlight)
                      << Styled("cash ", Theme::Muted) << Styled(padStr("Tk " + fmtD(f.cash), 14), Theme::Highlight)
                      << Styled("workers ", Theme::Muted) << Styled(padStr(std::to_string(f.workers.size()), 4), Theme::Warning)
                      << Styled("capital ", Theme::Muted) << Styled(padStr(std::to_string(f.capitals.size()), 4), Theme::Warning)
                      << Styled("Q ", Theme::Muted) << Styled(fmtD(f.currentOutput), Theme::Secondary)
                      << "\n";
        }
        bln();
    }

    void cmdMarkets(const Command &cmd)
    {
        sH("MARKETS", std::to_string(simulation.markets.size()) + " active");
        for (auto &m : simulation.markets)
        {
            std::cout << "  " << Styled("▸", Theme::Primary) << "  "
                      << Styled(padStr(m.prod->name, 14), Theme::Highlight)
                      << Styled("P ", Theme::Muted) << Styled(padStr("Tk " + fmtD(m.price), 12), Theme::Warning)
                      << Styled("D: ", Theme::Muted) << Styled("p = " + fmtD(m.aggregateDemand.c) + " − " + fmtD(m.aggregateDemand.m) + "Q", Theme::Secondary)
                      << "  "
                      << Styled("S: ", Theme::Muted) << Styled("p = " + fmtD(m.aggregateSupply.c) + " + " + fmtD(m.aggregateSupply.m) + "Q", Theme::Secondary)
                      << "\n";
        }
        bln();
    }

    void cmdProducts(const Command &cmd)
    {
        std::vector<product *> prods = {&rice, &cloth, &computer, &phone, &car, &steel, &potato, &banana, &corn, &jute};
        sH("PRODUCTS", std::to_string(prods.size()) + " goods");
        for (auto *p : prods)
        {
            std::cout << "  " << Styled("▸", Theme::Primary) << "  "
                      << Styled(padStr(p->name, 14), Theme::Highlight)
                      << Styled("decay ", Theme::Muted) << Styled(padStr(fmtD(p->decayRate) + "/day", 11), Theme::Warning)
                      << Styled("η ", Theme::Muted) << Styled(padStr(fmtD(p->eta), 7), Theme::Info)
                      << Styled("base ", Theme::Muted) << Styled(fmtD(p->baseConsumption) + " units", Theme::Secondary)
                      << "\n";
        }
        bln();
    }

    // ── ADD / CREATE COMMANDS ──────────────────────────────────────────────

    void cmdAddConsumer(const Command &cmd)
    {
        if (!hasParam(cmd, "name") || !hasParam(cmd, "age"))
        {
            output(Styled("[✗]", Theme::Error) + " Missing params  →  add_consumer(name, age)");
            return;
        }
        std::string name = getParam<std::string>(cmd, "name", std::string());
        int age = getParam<int>(cmd, "age", 0);
        if (name.empty())
        {
            output(Styled("[✗]", Theme::Error) + " Invalid name");
            return;
        }
        simulation.addConsumer(name, age);
        successNote("Consumer added  →  " + name + "  age " + std::to_string(age));
    }

    void cmdAddLaborer(const Command &cmd)
    {
        if (!hasParam(cmd, "name") || !hasParam(cmd, "age") || !hasParam(cmd, "skill") || !hasParam(cmd, "minwage"))
        {
            output(Styled("[✗]", Theme::Error) + " Missing params  →  add_laborer(name, age, skill, minwage)");
            return;
        }
        std::string name = getParam<std::string>(cmd, "name", std::string());
        int age = getParam<int>(cmd, "age", 0);
        double skill = getParam<double>(cmd, "skill", 0.0);
        double minwage = getParam<double>(cmd, "minwage", 0.0);
        if (name.empty())
        {
            output(Styled("[✗]", Theme::Error) + " Invalid name");
            return;
        }
        simulation.addlaborer(name, age, skill, minwage);
        successNote("Laborer added  →  " + name + "  skill " + std::to_string((int)(skill * 100)) + "%  min $" + fmtD(minwage) + "/day");
    }

    void cmdAddFarmer(const Command &cmd)
    {
        if (!hasParam(cmd, "name") || !hasParam(cmd, "age") || !hasParam(cmd, "land") || !hasParam(cmd, "tech"))
        {
            output(Styled("[✗]", Theme::Error) + " Missing params  →  add_farmer(name, age, land, tech)");
            return;
        }
        std::string name = getParam<std::string>(cmd, "name", std::string());
        int age = getParam<int>(cmd, "age", 0);
        double land = getParam<double>(cmd, "land", 0.0);
        double tech = getParam<double>(cmd, "tech", 0.0);
        if (name.empty())
        {
            output(Styled("[✗]", Theme::Error) + " Invalid name");
            return;
        }
        simulation.addFarmer(name, age, land, tech);
        successNote("Farmer added  →  " + name + "  land " + fmtD(land) + " ac  tech " + std::to_string((int)(tech * 100)) + "%");
    }

    void cmdAddFirm(const Command &cmd)
    {
        if (!hasParam(cmd, "ownerid") || !hasParam(cmd, "cash") || !hasParam(cmd, "alpha") || !hasParam(cmd, "beta"))
        {
            output(Styled("[✗]", Theme::Error) + " Missing params  →  add_firm(ownerid, cash, alpha, beta)");
            return;
        }
        int ownerId = getParam<int>(cmd, "ownerid", 0);
        double cash = getParam<double>(cmd, "cash", 0.0);
        double alpha = getParam<double>(cmd, "alpha", 0.0);
        double beta = getParam<double>(cmd, "beta", 0.0);
        simulation.addFirm(ownerId, cash, cobbDouglas(alpha, beta, 1.0));
        successNote("Firm added  →  owner #" + std::to_string(ownerId) + "  α=" + fmtD(alpha) + "  β=" + fmtD(beta));
    }

    // ── KILL COMMANDS ──────────────────────────────────────────────────────

    void cmdKillConsumer(const Command &cmd)
    {
        if (!simulation.selected_consumer)
        {
            output(Styled("[✗]", Theme::Error) + " No consumer selected");
            return;
        }
        std::string name = simulation.selected_consumer->name;
        simulation.selected_consumer->die();
        simulation.selected_consumer = nullptr;
        simulation.consumers.erase(
            std::remove_if(simulation.consumers.begin(), simulation.consumers.end(),
                           [](const consumer &c)
                           { return !c.isAlive; }),
            simulation.consumers.end());
        requestHeaderRefresh();
        std::cout << "\n  " << Styled("  ✗  " + name + " was killed and removed from simulation", Theme::Error) << "\n\n";
    }

    void cmdKillFarmer(const Command &cmd)
    {
        if (!simulation.selected_farmer)
        {
            output(Styled("[✗]", Theme::Error) + " No farmer selected");
            return;
        }
        std::string name = simulation.selected_farmer->name;
        simulation.selected_farmer->die();
        simulation.selected_farmer = nullptr;
        simulation.farmers.erase(
            std::remove_if(simulation.farmers.begin(), simulation.farmers.end(),
                           [](const farmer &f)
                           { return !f.isAlive; }),
            simulation.farmers.end());
        requestHeaderRefresh();
        std::cout << "\n  " << Styled("  ✗  " + name + " was killed and removed from simulation", Theme::Error) << "\n\n";
    }

    void cmdKillLaborer(const Command &cmd)
    {
        if (!simulation.selected_laborer)
        {
            output(Styled("[✗]", Theme::Error) + " No laborer selected");
            return;
        }
        std::string name = simulation.selected_laborer->name;
        simulation.selected_laborer->die();
        simulation.selected_laborer = nullptr;
        simulation.laborers.erase(
            std::remove_if(simulation.laborers.begin(), simulation.laborers.end(),
                           [](const laborer &l)
                           { return !l.isAlive; }),
            simulation.laborers.end());
        requestHeaderRefresh();
        std::cout << "\n  " << Styled("  ✗  " + name + " was killed and removed from simulation", Theme::Error) << "\n\n";
    }

    // ── SELECTION COMMANDS ─────────────────────────────────────────────────

    void cmdSelectConsumer(const Command &cmd)
    {
        if (!hasParam(cmd, "name"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: select_consumer(name)");
            return;
        }
        std::string name = getParam<std::string>(cmd, "name", std::string());
        for (auto &c : simulation.consumers)
        {
            if (c.name == name)
            {
                simulation.selected_consumer = &c;
                successNote("Selected consumer  →  " + name);
                return;
            }
        }
        output(Styled("[✗]", Theme::Error) + " Consumer not found: " + name);
    }

    void cmdSelectLaborer(const Command &cmd)
    {
        if (!hasParam(cmd, "name"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: select_laborer(name)");
            return;
        }
        std::string name = getParam<std::string>(cmd, "name", std::string());
        for (auto &l : simulation.laborers)
        {
            if (l.name == name)
            {
                simulation.selected_laborer = &l;
                successNote("Selected laborer  →  " + name);
                return;
            }
        }
        output(Styled("[✗]", Theme::Error) + " Laborer not found: " + name);
    }

    void cmdSelectFarmer(const Command &cmd)
    {
        if (!hasParam(cmd, "name"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: select_farmer(name)");
            return;
        }
        std::string name = getParam<std::string>(cmd, "name", std::string());
        for (auto &f : simulation.farmers)
        {
            if (f.name == name)
            {
                simulation.selected_farmer = &f;
                successNote("Selected farmer  →  " + name);
                return;
            }
        }
        output(Styled("[✗]", Theme::Error) + " Farmer not found: " + name);
    }

    void cmdSelectMarket(const Command &cmd)
    {
        if (!hasParam(cmd, "product"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: select_market(product)");
            return;
        }
        std::string prodName = getParam<std::string>(cmd, "product", std::string());
        for (auto &m : simulation.markets)
        {
            if (m.prod->name == prodName)
            {
                simulation.selected_market = &m;
                successNote("Selected market  →  " + prodName);
                return;
            }
        }
        output(Styled("[✗]", Theme::Error) + " Market not found: " + prodName);
    }

    void cmdClearSelection(const Command &cmd)
    {
        simulation.selected_consumer = nullptr;
        simulation.selected_laborer = nullptr;
        simulation.selected_farmer = nullptr;
        simulation.selected_market = nullptr;
        successNote("All selections cleared");
    }

    // ── MARKET ANALYSIS (legacy, kept for internal use) ───────────────────

    void cmdMarketEquilibrium(const Command &cmd)
    {
        market *m = getMarketFromCmd(cmd);
        if (!m)
        {
            output(Styled("[✗]", Theme::Error) + " No market selected");
            return;
        }

        m->calculateAggregateDemand(simulation.consumers, simulation.farmers, simulation.laborers);
        m->calculateAggregateSupply(simulation.farmers);
        auto eq = m->findEquilibrium();
        m->price = eq.price;

        sH("MARKET EQUILIBRIUM", m->prod->name);
        eqRow("Demand curve", "P = " + fmtD(m->aggregateDemand.c) + " − " + fmtD(m->aggregateDemand.m) + "Q");
        eqRow("Supply curve", "P = " + fmtD(m->aggregateSupply.c) + " + " + fmtD(m->aggregateSupply.m) + "Q");
        hline();
        kv("Equilibrium price", "Tk " + fmtD(eq.price));
        kv("Equilibrium qty", fmtD(eq.quantity) + " units");
        kv("Market value", "Tk " + fmtD(eq.price * eq.quantity));
        bln();
    }

    void cmdMarketDemand(const Command &cmd)
    {
        market *m = getMarketFromCmd(cmd);
        if (!m)
        {
            output(Styled("[✗]", Theme::Error) + " No market selected");
            return;
        }

        m->calculateAggregateDemand(simulation.consumers, simulation.farmers, simulation.laborers);

        sH("AGGREGATE DEMAND", m->prod->name);
        eqRow("Aggregate curve", "P = " + fmtD(m->aggregateDemand.c) + " − " + fmtD(m->aggregateDemand.m) + "Q");
        hline();
        noteText("Individual demand curves (consumers, farmers, laborers):");

        auto printDemandCurve = [&](const std::string &label, const consumer &agent)
        {
            auto it = agent.dd.find(m->prod);
            if (it != agent.dd.end())
            {
                std::cout << "      " << Styled(padStr(label, 18), Theme::Warning)
                          << Styled("P = " + fmtD(it->second.c) + " − " + fmtD(it->second.m) + "Q",
                                    Theme::Secondary)
                          << "\n";
            }
        };

        for (const auto &c : simulation.consumers)
            printDemandCurve("[C] " + c.name, c);

        for (const auto &f : simulation.farmers)
            printDemandCurve("[F] " + f.name, static_cast<const consumer &>(f));

        for (const auto &l : simulation.laborers)
            printDemandCurve("[L] " + l.name, static_cast<const consumer &>(l));

        if (m->aggregateDemand.m <= 0.000001)
        {
            noteText("No valid demand curve found for this product.");
        }

        bln();
    }

    void cmdMarketSupply(const Command &cmd)
    {
        market *m = getMarketFromCmd(cmd);
        if (!m)
        {
            output(Styled("[✗]", Theme::Error) + " No market selected");
            return;
        }

        m->calculateAggregateSupply(simulation.farmers);

        sH("AGGREGATE SUPPLY", m->prod->name);
        eqRow("Aggregate curve", "P = " + fmtD(m->aggregateSupply.c) + " + " + fmtD(m->aggregateSupply.m) + "Q");
        hline();
        noteText("Individual supply curves:");
        for (auto &f : simulation.farmers)
        {
            if (f.ss.find(m->prod) != f.ss.end())
            {
                std::cout << "      " << Styled(padStr(f.name, 14), Theme::Warning)
                          << Styled("P = " + fmtD(f.ss[m->prod].c) + " + " + fmtD(f.ss[m->prod].m) + "Q", Theme::Secondary)
                          << "  " << Styled("max " + fmtD(f.maxOutput[m->prod]) + " u", Theme::Muted)
                          << "\n";
            }
        }
        bln();
    }

    // ── CONSUMER ANALYSIS ──────────────────────────────────────────────────

    void cmdConsumerMU(const Command &cmd)
    {
        consumer *c = simulation.selected_consumer;
        if (!c)
        {
            output(Styled("[✗]", Theme::Error) + " No consumer selected");
            return;
        }
        if (!hasParam(cmd, "product"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: consumer_mu(product)");
            return;
        }

        std::string prodName = getParam<std::string>(cmd, "product", std::string());
        product *p = getProductByName(prodName);
        if (!p)
        {
            output(Styled("[✗]", Theme::Error) + " Unknown product: " + prodName);
            return;
        }

        double mu = c->getMarginalUtility(*p);
        double muPerTk = c->getMUperTk();
        double wtp = c->dd[p].c - c->dd[p].m * c->consumed[p];

        sH("MARGINAL UTILITY", c->name + "  →  " + prodName);
        kv("MU per Tk", fmtD(muPerTk, 7) + "  (= 1 / wealth)");
        kv("Willingness to pay", "Tk " + fmtD(wtp));
        kv("Consumed so far", fmtD(c->consumed[p]) + " units");
        hline();
        kv("Marginal utility", fmtD(mu, 7));
        bln();
    }

    void cmdConsumerSurplus(const Command &cmd)
    {
        consumer *c = simulation.selected_consumer;
        if (!c)
        {
            output(Styled("[✗]", Theme::Error) + " No consumer selected");
            return;
        }
        if (!hasParam(cmd, "product"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: consumer_surplus(product)");
            return;
        }

        std::string prodName = getParam<std::string>(cmd, "product", std::string());
        product *p = getProductByName(prodName);
        market *m = getMarketByProduct(p);
        if (!p || !m)
        {
            output(Styled("[✗]", Theme::Error) + " Unknown product or market");
            return;
        }

        // findKey() returns the real global pointer used as the map key.
        // Using &(*p) directly would point at a local copy and miss the map entry.
        product *key = c->findKey(prodName);
        if (!key)
        {
            output(Styled("[✗]", Theme::Error) + " Consumer has no demand curve for " + prodName);
            return;
        }

        double intercept = c->dd[key].c; // P-intercept of individual demand curve
        double slope = c->dd[key].m;     // slope m
        double marketPrice = m->price;

        // Consumer surplus = ½ × (c − P) × Q*   where Q* = (c−P)/m
        double qStar = (slope > 0.001) ? std::max(0.0, (intercept - marketPrice) / slope) : 0.0;
        double surplus = 0.5 * (intercept - marketPrice) * qStar;
        surplus = std::max(0.0, surplus);

        double wtp = intercept - slope * c->consumed[key]; // WTP at current consumed qty

        sH("CONSUMER SURPLUS", c->name + "  →  " + prodName);
        kv("Market price", "Tk " + fmtD(marketPrice));
        kv("Max WTP (intercept)", "Tk " + fmtD(intercept));
        kv("WTP at current qty", "Tk " + fmtD(wtp));
        kv("Qty at market price", fmtD(qStar, 3) + " units");
        hline();
        kv("Consumer surplus", "Tk " + fmtD(surplus));
        noteText("½ × (c − P) × Q*  =  ½ × (max WTP − price) × qty demanded");
        if (surplus > 0)
            noteText("This consumer gains Tk " + fmtD(surplus) + " of value above what they pay");
        bln();
    }

    void cmdConsumerSubstitution(const Command &cmd)
    {
        consumer *c = simulation.selected_consumer;
        if (!c)
        {
            output(Styled("[✗]", Theme::Error) + " No consumer selected");
            return;
        }

        sH("SUBSTITUTION RATIOS", c->name);
        noteText("MRS relative to Rice  (MU_good / MU_rice)");
        hline();
        for (auto &need : c->needs)
        {
            double ratio = c->updateSubRatio(need);
            std::string bar = "";
            int barLen = std::min((int)(ratio * 20), 30);
            for (int i = 0; i < barLen; i++)
                bar += "▪";

            std::cout << "    "
                      << Styled(padStr(need.name, 14), Theme::Info)
                      << Styled(padStr(fmtD(ratio, 3), 9), Theme::Highlight)
                      << Styled(bar, Theme::Secondary) << "\n";
        }
        bln();
    }

    void cmdConsumerNeeds(const Command &cmd)
    {
        consumer *c = simulation.selected_consumer;
        if (!c)
        {
            output(Styled("[✗]", Theme::Error) + " No consumer selected");
            return;
        }

        sH("NEEDS & CONSUMPTION", c->name);
        for (auto &need : c->needs)
        {
            product *p = &need;
            entLabel(need.name);
            eqRow("Demand curve", "P = " + fmtD(c->dd[p].c) + " − " + fmtD(c->dd[p].m) + "Q");
            kv("Consumed", fmtD(c->consumed[p]) + " units");
        }
        bln();
    }

    void cmdConsumerDemandCurve(const Command &cmd)
    {
        consumer *c = simulation.selected_consumer;
        if (!c)
        {
            output(Styled("[✗]", Theme::Error) + " No consumer selected");
            return;
        }
        if (!hasParam(cmd, "product"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: consumer_demand_curve(product)");
            return;
        }

        std::string prodName = getParam<std::string>(cmd, "product", std::string());
        product *p = getProductByName(prodName);
        if (!p)
        {
            output(Styled("[✗]", Theme::Error) + " Unknown product: " + prodName);
            return;
        }

        sH("DEMAND CURVE", c->name + "  →  " + prodName);
        eqRow("Individual curve", "P = " + fmtD(c->dd[p].c) + " − " + fmtD(c->dd[p].m) + "Q");
        hline();
        kv("Intercept (c)", fmtD(c->dd[p].c) + "  (max WTP at Q=0)");
        kv("Slope (m)", fmtD(c->dd[p].m) + "  (WTP falls by this per unit)");
        bln();
    }

    // ── FARMER ANALYSIS ────────────────────────────────────────────────────

    void cmdFarmerSupply(const Command &cmd)
    {
        farmer *f = simulation.selected_farmer;
        if (!f)
        {
            output(Styled("[✗]", Theme::Error) + " No farmer selected");
            return;
        }
        if (!hasParam(cmd, "product") || !hasParam(cmd, "price"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: farmer_supply(product, price)");
            return;
        }

        std::string prodName = getParam<std::string>(cmd, "product", std::string());
        double price = getParam<double>(cmd, "price", 0.0);
        product *p = getProductByName(prodName);
        if (!p)
        {
            output(Styled("[✗]", Theme::Error) + " Unknown product: " + prodName);
            return;
        }

        double qty = f->calculateSupply(p, price);

        sH("FARMER SUPPLY", f->name + "  →  " + prodName);
        kv("Query price", "Tk " + fmtD(price));
        kv("MC intercept", "Tk " + fmtD(f->ss[p].c));
        kv("Slope", fmtD(f->ss[p].m, 4));
        hline();
        kv("Supply at P", fmtD(qty) + " units");
        if (price <= f->ss[p].c)
            noteText("Price is below marginal cost — farmer will not produce");
        bln();
    }

    void cmdFarmerCrops(const Command &cmd)
    {
        farmer *f = simulation.selected_farmer;
        if (!f)
        {
            output(Styled("[✗]", Theme::Error) + " No farmer selected");
            return;
        }

        sH("CROPS", f->name);
        for (auto &crop : f->crops)
        {
            product *p = &crop;
            entLabel(crop.name);
            eqRow("Supply curve", "P = " + fmtD(f->ss[p].c) + " + " + fmtD(f->ss[p].m) + "Q");
            kv("Growth rate", fmtD(f->growthRate[p]) + " units/day");
            kv("Decay rate", fmtD(f->decay[p]) + " units/day");
            kv("Max output", fmtD(f->maxOutput[p]) + " units");
        }
        bln();
    }

    void cmdFarmerUpgrade(const Command &cmd)
    {
        farmer *f = simulation.selected_farmer;
        if (!f)
        {
            output(Styled("[✗]", Theme::Error) + " No farmer selected");
            return;
        }
        if (!hasParam(cmd, "level"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: farmer_upgrade(level)");
            return;
        }

        double newLevel = getParam<double>(cmd, "level", f->techLevel);
        double oldLevel = f->techLevel;

        // Snapshot supply curves BEFORE upgrade
        std::vector<std::tuple<std::string, double, double>> before; // name, c, m
        for (auto &crop : f->crops)
            before.emplace_back(crop.name, f->ss[&crop].c, f->ss[&crop].m);

        f->upgradeTech(newLevel);

        // Apply new supply curves immediately so we can show the shift
        for (auto &crop : f->crops)
            f->updateSupplyCurve(&crop);

        sH("TECH UPGRADE", f->name);

        // Tech level change bar
        int barOld = (int)(oldLevel * 30);
        int barNew = (int)(newLevel * 30);
        std::string barStr;
        for (int i = 0; i < 30; i++)
        {
            if (i < barOld)
                barStr += Styled("█", Theme::Muted);
            else if (i < barNew)
                barStr += Styled("█", Theme::Success);
            else
                barStr += Styled("░", Theme::Muted);
        }
        std::cout << "    " << barStr << "\n";
        std::cout << "    "
                  << Styled(padStr("Tech level", 24), Theme::Info)
                  << Styled(fmtD(oldLevel * 100, 0) + "%", Theme::Muted)
                  << Styled("  →  ", Theme::Info)
                  << Styled(fmtD(newLevel * 100, 0) + "%", Theme::Success)
                  << "\n";

        // Show supply curve shifts for each crop
        if (!f->crops.empty())
        {
            hline();
            std::cout << "\n  " << Styled("SUPPLY CURVE EFFECTS  (higher tech → lower cost floor → more supply)", Theme::Warning) << "\n\n";

            for (size_t i = 0; i < f->crops.size() && i < before.size(); i++)
            {
                auto &[cname, oldC, oldM] = before[i];
                double newC = f->ss[&f->crops[i]].c;
                double newM = f->ss[&f->crops[i]].m;

                std::cout << "    " << Styled(padStr(cname, 12), Theme::Warning) << "\n";
                eqRow("  Before", "P = " + fmtD(oldC) + " + " + fmtD(oldM, 3) + "Q  (cost floor Tk " + fmtD(oldC) + ")");
                eqRow("  After", "P = " + fmtD(newC) + " + " + fmtD(newM, 3) + "Q  (cost floor Tk " + fmtD(newC) + ")");

                double costDrop = oldC - newC;
                if (costDrop > 0.01)
                    noteText("  Cost floor fell by Tk " + fmtD(costDrop) + "  →  supply shifts right (more at same price)");
                else if (costDrop < -0.01)
                    noteText("  Cost floor rose by Tk " + fmtD(-costDrop) + "  →  supply shifts left");
                else
                    noteText("  Cost floor unchanged (weather/tax may be offsetting tech gain)");
            }
        }
        bln();
    }

    void cmdFarmerWeather(const Command &cmd)
    {
        farmer *f = simulation.selected_farmer;
        if (!f)
        {
            output(Styled("[✗]", Theme::Error) + " No farmer selected");
            return;
        }

        // Build a simple bar indicator
        int barLen = (int)(f->weather * 30);
        std::string bar;
        for (int i = 0; i < 30; i++)
            bar += (i < barLen) ? "█" : "░";

        const char *weatherColor = f->weather > 0.65 ? Theme::Success : f->weather > 0.4 ? Theme::Warning
                                                                                         : Theme::Error;
        std::string weatherLabel = f->weather > 0.65 ? "Good" : f->weather > 0.4 ? "Moderate"
                                                                                 : "Poor";

        sH("WEATHER INDEX", f->name);
        kv("Weather factor", fmtD(f->weather, 2) + "  " + Styled("(" + weatherLabel + ")", weatherColor));
        std::cout << "    " << Styled(bar, weatherColor) << "\n";
        hline();
        noteText("< 0.4 raises marginal cost  ·  > 0.65 boosts output");
        bln();
    }

    void cmdFarmerSupplyCurve(const Command &cmd)
    {
        farmer *f = simulation.selected_farmer;
        if (!f)
        {
            output(Styled("[✗]", Theme::Error) + " No farmer selected");
            return;
        }
        if (!hasParam(cmd, "product"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: farmer_supply_curve(product)");
            return;
        }

        std::string prodName = getParam<std::string>(cmd, "product", std::string());
        product *p = getProductByName(prodName);
        if (!p)
        {
            output(Styled("[✗]", Theme::Error) + " Unknown product: " + prodName);
            return;
        }

        sH("SUPPLY CURVE", f->name + "  →  " + prodName);
        eqRow("Curve", "P = " + fmtD(f->ss[p].c) + " + " + fmtD(f->ss[p].m) + "Q");
        kv("Max output", fmtD(f->maxOutput[p]) + " units");
        bln();
    }

    // ── FIRM ANALYSIS ──────────────────────────────────────────────────────

    void cmdFirmCosts(const Command &cmd)
    {
        if (simulation.selected_firm == nullptr)
        {
            output(Styled("[✗]", Theme::Error) + " No firm selected");
            return;
        }
        firm *f = simulation.selected_firm;
        f->calculateCosts();

        std::string ownerName = "Owner #" + std::to_string(f->ownerId);
        for (auto &c : simulation.consumers)
            if (c.id == f->ownerId)
            {
                ownerName = c.name + "'s firm";
                break;
            }

        sH("COST ANALYSIS", ownerName);
        kv("Labor (L)", std::to_string(f->workers.size()) + " workers");
        kv("Capital (K)", std::to_string(f->capitals.size()) + " units");
        kv("Output (Q)", fmtD(f->currentOutput) + " units");
        hline();

        // Fixed & Variable
        std::cout << "    " << Styled(padStr("TFC", 24), Theme::Info)
                  << Styled("Tk " + fmtD(f->totalFixedCost), Theme::Highlight)
                  << Styled("  (overhead + capital rental)", Theme::Muted) << "\n";
        std::cout << "    " << Styled(padStr("TVC", 24), Theme::Info)
                  << Styled("Tk " + fmtD(f->totalVariableCost), Theme::Highlight)
                  << Styled("  (L × wage)", Theme::Muted) << "\n";
        std::cout << "    " << Styled(padStr("TC", 24), Theme::Info)
                  << Styled("Tk " + fmtD(f->totalCost), Theme::Highlight) << "\n";
        hline();

        // Averages
        kv("AFC  (TFC/Q)", "Tk " + fmtD(f->averageFixedCost));
        kv("AVC  (TVC/Q)", "Tk " + fmtD(f->averageVariableCost));
        kv("AC   (TC/Q)", "Tk " + fmtD(f->averageCost));
        hline();

        // Marginal
        kv("MC  (w / MPL)", "Tk " + fmtD(f->marginalCost));

        if (f->marginalCost < f->averageCost)
            noteText("MC < AC  →  Economies of scale (AC is still falling)");
        else
            noteText("MC > AC  →  Diminishing returns (AC is rising)");
        bln();
    }

    void cmdFirmOutput(const Command &cmd)
    {
        if (simulation.selected_firm == nullptr)
        {
            output(Styled("[✗]", Theme::Error) + " No firm selected");
            return;
        }
        firm *f = simulation.selected_firm;
        double L = f->workers.size();
        double K = f->capitals.size();
        double Q = f->prodFunc->output(L, K);

        std::string ownerName = "Owner #" + std::to_string(f->ownerId);
        for (auto &c : simulation.consumers)
            if (c.id == f->ownerId)
            {
                ownerName = c.name + "'s firm";
                break;
            }

        sH("PRODUCTION OUTPUT", ownerName);
        kv("Function type", "Cobb-Douglas");
        eqRow("Q(L,K)", "A · L^α · K^β  =  " +
                            fmtD(f->cdProd.tech) + " · L^" + fmtD(f->cdProd.alpha) + " · K^" + fmtD(f->cdProd.beta));
        hline();
        kv("Labor (L)", std::to_string((int)L) + " workers");
        kv("Capital (K)", std::to_string((int)K) + " units");
        kv("Output (Q)", fmtD(Q) + " units");
        bln();
    }

    void cmdFirmMP(const Command &cmd)
    {
        if (simulation.selected_firm == nullptr)
        {
            output(Styled("[✗]", Theme::Error) + " No firm selected");
            return;
        }
        firm *f = simulation.selected_firm;
        double mpL = f->MPofLabor();
        double mpK = f->MPofCapital();

        std::string ownerName = "Owner #" + std::to_string(f->ownerId);
        for (auto &c : simulation.consumers)
            if (c.id == f->ownerId)
            {
                ownerName = c.name + "'s firm";
                break;
            }

        sH("MARGINAL PRODUCTS", ownerName);
        kv("MPL  (adding 1 worker)", "+" + fmtD(mpL, 4) + " units");
        kv("MPK  (adding 1 machine)", "+" + fmtD(mpK, 4) + " units");
        hline();
        if (mpL > mpK)
            noteText("Labor has higher marginal product right now");
        else if (mpK > mpL)
            noteText("Capital has higher marginal product right now");
        else
            noteText("Balanced marginal products");
        bln();
    }

    void cmdFirmEfficiency(const Command &cmd)
    {
        if (simulation.selected_firm == nullptr)
        {
            output(Styled("[✗]", Theme::Error) + " No firm selected");
            return;
        }
        firm *f = simulation.selected_firm;
        auto ratios = f->marginalCosts();
        double diff = ratios[0] - ratios[1];

        std::string ownerName = "Owner #" + std::to_string(f->ownerId);
        for (auto &c : simulation.consumers)
            if (c.id == f->ownerId)
            {
                ownerName = c.name + "'s firm";
                break;
            }

        sH("FACTOR EFFICIENCY", ownerName);
        kv("Labor  MPL/w", fmtD(ratios[0], 4) + "  (output per $ of wage)");
        kv("Capital  MPK/r", fmtD(ratios[1], 4) + "  (output per $ of rental)");
        hline();

        if (std::abs(diff) < 0.05)
            successNote("Optimal input mix  —  isoquant tangent to isocost");
        else if (diff > 0)
            warnNote("Labor is more efficient  →  consider hiring another worker");
        else
            warnNote("Capital is more efficient  →  consider adding a machine");
        bln();
    }

    // ── FIRM MUTATION COMMANDS ─────────────────────────────────────────────

    void cmdFirmHire(const Command &cmd)
    {
        if (simulation.selected_firm == nullptr)
        {
            output(Styled("[✗]", Theme::Error) + " No firm selected");
            return;
        }
        firm *f = simulation.selected_firm;
        if (!hasParam(cmd, "laborer"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: firm_hire(laborer)");
            return;
        }

        std::string name = getParam<std::string>(cmd, "laborer", std::string());
        for (auto &l : simulation.laborers)
        {
            if (l.name == name)
            {
                f->workers.push_back(l);
                f->calculateCosts();
                successNote("Hired " + name + "  →  Q = " + fmtD(f->currentOutput) + " units");
                return;
            }
        }
        output(Styled("[✗]", Theme::Error) + " Laborer not found: " + name);
    }

    void cmdFirmFire(const Command &cmd)
    {
        if (simulation.selected_firm == nullptr)
        {
            output(Styled("[✗]", Theme::Error) + " No firm selected");
            return;
        }
        firm *f = simulation.selected_firm;
        if (!hasParam(cmd, "laborer"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: firm_fire(laborer)");
            return;
        }

        std::string name = getParam<std::string>(cmd, "laborer", std::string());
        for (auto it = f->workers.begin(); it != f->workers.end(); ++it)
        {
            if (it->name == name)
            {
                f->workers.erase(it);
                f->calculateCosts();
                std::cout << "\n  " << Styled("  ✗  Fired " + name + "  →  Q = " + fmtD(f->currentOutput) + " units", Theme::Warning) << "\n\n";
                return;
            }
        }
        output(Styled("[✗]", Theme::Error) + " Laborer not found in firm: " + name);
    }

    void cmdFirmCapital(const Command &cmd)
    {
        if (simulation.selected_firm == nullptr)
        {
            output(Styled("[✗]", Theme::Error) + " No firm selected");
            return;
        }
        firm *f = simulation.selected_firm;
        if (!hasParam(cmd, "rental") || !hasParam(cmd, "eff"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: firm_capital(rental, eff)");
            return;
        }
        double rental = getParam<double>(cmd, "rental", 0.0);
        double eff = getParam<double>(cmd, "eff", 0.0);
        f->capitals.emplace_back(rental, eff);
        f->calculateCosts();
        successNote("Capital added  r=$" + fmtD(rental) + " eff=" + fmtD(eff) + "  →  Q = " + fmtD(f->currentOutput) + " units");
    }

    // ── FARMER TAX ────────────────────────────────────────────────────────
    void cmdFarmerTax(const Command &cmd)
    {
        farmer *f = simulation.selected_farmer;
        if (!f)
        {
            output(Styled("[✗]", Theme::Error) + " No farmer selected");
            return;
        }
        if (!hasParam(cmd, "rate"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: farmer_tax(rate)  e.g. farmer_tax(0.15)");
            return;
        }

        double newRate = getParam<double>(cmd, "rate", f->tax);
        if (newRate < 0.0 || newRate > 1.0)
        {
            output(Styled("[✗]", Theme::Error) + " Tax rate must be 0.0 – 1.0");
            return;
        }

        double oldRate = f->tax;

        // Snapshot curves before
        std::vector<std::tuple<std::string, double, double>> before;
        for (auto &crop : f->crops)
            before.emplace_back(crop.name, f->ss[&crop].c, f->ss[&crop].m);

        // Apply tax and immediately recalculate supply curves
        f->tax = newRate;
        for (auto &crop : f->crops)
            f->updateSupplyCurve(&crop);

        sH("TAX POLICY CHANGE", f->name);

        const char *dir = newRate > oldRate ? Theme::Error : Theme::Success;
        std::string arrow = newRate > oldRate ? "▲ INCREASE" : "▼ DECREASE";
        std::cout << "    "
                  << Styled(padStr("Tax rate", 24), Theme::Info)
                  << Styled(fmtD(oldRate * 100, 1) + "%", Theme::Muted)
                  << Styled("  →  ", Theme::Info)
                  << Styled(fmtD(newRate * 100, 1) + "%  " + arrow, dir)
                  << "\n";

        hline();
        std::cout << "\n  " << Styled("HOW TAX AFFECTS SUPPLY  (tax raises marginal cost → supply shifts left)", Theme::Warning) << "\n\n";

        for (size_t i = 0; i < f->crops.size() && i < before.size(); i++)
        {
            auto &[cname, oldC, oldM] = before[i];
            double newC = f->ss[&f->crops[i]].c;
            double newM = f->ss[&f->crops[i]].m;
            double costRise = newC - oldC;

            std::cout << "    " << Styled(padStr(cname, 12), Theme::Warning) << "\n";
            eqRow("  Before", "P = " + fmtD(oldC) + " + " + fmtD(oldM, 3) + "Q");
            eqRow("  After", "P = " + fmtD(newC) + " + " + fmtD(newM, 3) + "Q");

            if (costRise > 0.01)
            {
                noteText("  Cost floor ▲ Tk " + fmtD(costRise) +
                         "  →  farmer won't supply unless price ≥ Tk " + fmtD(newC));
                noteText("  Less food reaches market → consumers face higher prices");
            }
            else if (costRise < -0.01)
            {
                noteText("  Cost floor ▼ Tk " + fmtD(-costRise) + "  →  supply expands, prices may fall");
            }
        }

        hline();
        noteText("Run pass_day to propagate full effects through the market");
        bln();
    }

    // ── MARKET HISTORY ────────────────────────────────────────────────────
    void cmdMarketHistory(const Command &cmd)
    {
        market *m = simulation.selected_market;
        if (!m)
        {
            output(Styled("[✗]", Theme::Error) + " No market selected");
            return;
        }

        const auto &hist = m->priceHistory;
        if (hist.empty())
        {
            output(Styled("[i]", Theme::Info) + " No history yet — run pass_day first");
            return;
        }

        sH("PRICE HISTORY", m->prod->name + "  (last " + std::to_string(hist.size()) + " days)");

        // Chart bounds
        double minP = *std::min_element(hist.begin(), hist.end());
        double maxP = *std::max_element(hist.begin(), hist.end());
        double range = maxP - minP;
        if (range < 0.01)
            range = 1.0; // avoid div-by-zero on flat history

        const int chartH = 8;                              // rows
        const int chartW = std::min((int)hist.size(), 50); // max 50 cols

        // Build grid: chartH rows × chartW cols
        std::vector<std::string> rows(chartH, "");

        for (int col = 0; col < chartW; col++)
        {
            // Map history index to column (rightmost = most recent)
            int histIdx = (int)hist.size() - chartW + col;
            double p = hist[histIdx];
            double norm = (p - minP) / range;              // 0..1
            int filled = (int)(norm * (chartH - 1) + 0.5); // which row lights up (bottom = 0)

            for (int row = 0; row < chartH; row++)
            {
                int rowFromBottom = chartH - 1 - row;
                if (rowFromBottom == filled)
                    rows[row] += Styled("█", rowFromBottom > chartH / 2 ? Theme::Success : Theme::Warning);
                else if (rowFromBottom < filled)
                    rows[row] += Styled("▒", Theme::Muted);
                else
                    rows[row] += " ";
            }
        }

        // Print y-axis labels + chart
        for (int row = 0; row < chartH; row++)
        {
            double yVal = maxP - (row / (double)(chartH - 1)) * range;
            std::cout << "  " << Styled(padStr("Tk " + fmtD(yVal, 0), 9), Theme::Info)
                      << Styled("│", Theme::Muted) << rows[row] << "\n";
        }

        // X-axis
        std::cout << "           " << Styled("└" + std::string(chartW, '-'), Theme::Muted) << "\n";
        std::cout << "           " << Styled("Day 1" + std::string(std::max(0, chartW - 10), ' ') + "Today", Theme::Muted) << "\n";

        hline();
        kv("Current price", "Tk " + fmtD(m->price));
        kv("Min (period)", "Tk " + fmtD(minP));
        kv("Max (period)", "Tk " + fmtD(maxP));
        double trend = hist.size() > 1 ? hist.back() - hist.front() : 0.0;
        const char *tColor = trend > 0.5 ? Theme::Error : trend < -0.5 ? Theme::Success
                                                                       : Theme::Muted;
        std::cout << "    " << Styled(padStr("Trend", 24), Theme::Info)
                  << Styled((trend >= 0 ? "▲ +" : "▼ ") + fmtD(trend) + " Tk over period", tColor) << "\n";
        bln();
    }

    // ── SET INCOME ────────────────────────────────────────────────────────
    void cmdSetIncome(const Command &cmd)
    {
        consumer *c = simulation.selected_consumer;
        if (!c)
        {
            output(Styled("[✗]", Theme::Error) + " No consumer selected");
            return;
        }
        if (!hasParam(cmd, "value"))
        {
            output(Styled("[✗]", Theme::Error) + " Usage: set_income(value)");
            return;
        }

        double newIncome = getParam<double>(cmd, "value", c->incomePerDay);
        if (newIncome < 0)
        {
            output(Styled("[✗]", Theme::Error) + " Income cannot be negative");
            return;
        }

        double oldIncome = c->incomePerDay;
        double oldWealth = c->savings + oldIncome * 30;
        double newWealth = c->savings + newIncome * 30;
        double incomeChange = newIncome - oldIncome;

        c->incomePerDay = newIncome;

        // Update MU per Tk (richer = values each Tk less)
        double oldMU = c->muPerTk;
        c->muPerTk = c->getMUperTk();

        // Shift demand curves for all needs (Engel curve effect)
        for (auto &need : c->needs)
        {
            product *key = c->findKey(need.name);
            if (!key)
                continue;
            // Normal goods: demand shifts out; inferior goods: demand shifts in
            if (need.eta > 0)
                c->dd[key].c += incomeChange * 0.05 * need.eta;
            else
                c->dd[key].c = std::max(0.5, c->dd[key].c + incomeChange * 0.02 * need.eta);
        }

        sH("INCOME CHANGE", c->name);

        const char *dir = incomeChange > 0 ? Theme::Success : Theme::Error;
        std::string arrow = incomeChange > 0 ? "▲ RAISE" : "▼ CUT";
        std::cout << "    "
                  << Styled(padStr("Daily income", 24), Theme::Info)
                  << Styled("Tk " + fmtD(oldIncome), Theme::Muted)
                  << Styled("  →  ", Theme::Info)
                  << Styled("Tk " + fmtD(newIncome) + "  " + arrow, dir)
                  << "\n";
        kv("Effective wealth", "Tk " + fmtD(newWealth) + "  (savings + 30d income)");
        kv("MU per Tk", fmtD(oldMU, 4) + "  →  " + fmtD(c->muPerTk, 4) +
                            "  " + Styled((newIncome > oldIncome ? "(values money less)" : "(values money more)"), Theme::Muted));

        hline();
        std::cout << "\n  " << Styled("DEMAND SHIFTS  (Engel curve effect)", Theme::Warning) << "\n\n";

        for (auto &need : c->needs)
        {
            product *key = c->findKey(need.name);
            if (!key)
                continue;

            std::string elasticityTag = need.eta > 1.0 ? "luxury" : need.eta > 0.0 ? "normal"
                                                                : need.eta == 0.0  ? "neutral"
                                                                                   : "inferior";
            const char *eColor = need.eta > 0 ? Theme::Success : Theme::Warning;

            std::cout << "    " << Styled(padStr(need.name, 12), Theme::Warning)
                      << Styled("η = " + fmtD(need.eta, 2) + "  (" + elasticityTag + ")", eColor) << "\n";

            if (incomeChange > 0 && need.eta > 0)
                noteText("  WTP intercept ▲  →  demand shifts right (buys more at same price)");
            else if (incomeChange > 0 && need.eta < 0)
                noteText("  WTP intercept ▼  →  inferior good: buys less as income rises");
            else if (incomeChange < 0 && need.eta > 0)
                noteText("  WTP intercept ▼  →  income fell, can afford less of this good");
            else if (incomeChange < 0 && need.eta < 0)
                noteText("  WTP intercept ▲  →  inferior good: buys more when poorer");

            eqRow("  New demand", "P = " + fmtD(c->dd[key].c) + " − " + fmtD(c->dd[key].m) + "Q");
        }

        hline();
        noteText("Run pass_day to see full market equilibrium effects");
        bln();
    }

    // ── HELP ──────────────────────────────────────────────────────────────

    void cmdHelp(const Command &cmd)
    {
        const int COL1 = 40;

        struct Group
        {
            std::string label;
            std::string prefix;
        };
        std::vector<Group> groups = {
            {"LIST", "consumers|laborers|farmers|firms|markets|products"},
            {"ADD", "add_"},
            {"SELECT", "select_|clear_"},
            {"CONSUMER", "consumer_"},
            {"FARMER", "farmer_"},
            {"LABORER", "laborer_"},
            {"FIRM", "firm_"},
            {"MARKET", "market_"},
            {"SIMULATION", "pass_day|status|help|clear|exit"},
        };

        auto inGroup = [](const std::string &name, const std::string &pattern) -> bool
        {
            size_t pos = 0;
            while (pos < pattern.size())
            {
                size_t bar = pattern.find('|', pos);
                std::string tok = (bar == std::string::npos) ? pattern.substr(pos) : pattern.substr(pos, bar - pos);
                pos = (bar == std::string::npos) ? pattern.size() : bar + 1;
                if (tok.back() == '_')
                {
                    if (name.rfind(tok, 0) == 0)
                        return true;
                }
                else
                {
                    if (name == tok)
                        return true;
                }
            }
            return false;
        };

        std::stringstream ss;
        const auto &cmds = parser.getAvailableCommands();

        ss << "\n"
           << "  " << Styled(padStr("COMMAND", COL1), Theme::Muted)
           << Styled("DESCRIPTION", Theme::Muted) << "\n\n";

        auto divider = [&](const std::string &label)
        {
            ss << "  " << Styled(label, Theme::Warning) << "\n";
        };

        auto dataRow = [&](const CommandInfo &info)
        {
            int pad = std::max(1, COL1 - (int)info.name.size());
            ss << "    " << Styled(info.name, Theme::Highlight)
               << std::string(pad, ' ')
               << Styled(info.description, Theme::Secondary) << "\n";
        };

        std::vector<bool> printed(cmds.size(), false);
        for (auto &group : groups)
        {
            bool headerPrinted = false;
            for (size_t i = 0; i < cmds.size(); i++)
            {
                if (printed[i])
                    continue;
                if (inGroup(cmds[i].name, group.prefix))
                {
                    if (!headerPrinted)
                    {
                        divider(group.label);
                        headerPrinted = true;
                    }
                    dataRow(cmds[i]);
                    printed[i] = true;
                }
            }
            if (headerPrinted)
                ss << "\n";
        }

        bool miscPrinted = false;
        for (size_t i = 0; i < cmds.size(); i++)
        {
            if (!printed[i])
            {
                if (!miscPrinted)
                {
                    divider("MISC");
                    miscPrinted = true;
                }
                dataRow(cmds[i]);
            }
        }

        ss << "  " << Styled("Tab", Theme::Warning) << Styled(" to autocomplete  ·  ", Theme::Muted)
           << Styled("↑↓", Theme::Warning) << Styled(" for history\n", Theme::Muted);

        output(ss.str());
    }

    // ── STATUS ────────────────────────────────────────────────────────────

    void cmdStatus(const Command &cmd)
    {
        auto stats = simulation.getStats();

        sH("ECONOMIC STATUS");

        // GDP from markets
        std::cout << "  " << Styled("OUTPUT", Theme::Warning) << "\n";
        kv("GDP (market sum)", "Tk " + fmtD(stats.gdp));
        double gdpPerCapita = stats.population > 0 ? stats.gdp / stats.population : 0.0;
        kv("GDP per capita", "Tk " + fmtD(gdpPerCapita));
        hline();

        // Market prices snapshot
        std::cout << "  " << Styled("MARKET PRICES", Theme::Warning) << "\n";
        for (auto &m : simulation.markets)
        {
            if (m.price > 0.1)
            {
                std::string trendStr;
                if (m.priceHistory.size() > 1)
                {
                    double delta = m.price - m.priceHistory[m.priceHistory.size() - 2];
                    trendStr = delta > 0.5 ? Styled("  ▲", Theme::Warning) : delta < -0.5 ? Styled("  ▼", Theme::Info)
                                                                                          : Styled("  ─", Theme::Muted);
                }
                kv(m.prod->name, "Tk " + fmtD(m.price) + trendStr);
            }
        }
        hline();

        // Labor
        std::cout << "  " << Styled("LABOR MARKET", Theme::Warning) << "\n";
        kv("Population", std::to_string(stats.population));
        kv("Employed", std::to_string(stats.employed));
        double uPct = stats.unemployment * 100.0;
        const char *uColor = uPct < 5.0 ? Theme::Success : uPct < 10.0 ? Theme::Warning
                                                                       : Theme::Error;
        std::cout << "    " << Styled(padStr("Unemployment", 24), Theme::Info)
                  << Styled(fmtD(uPct) + "%", uColor) << "\n";
        kv("Total firms", std::to_string(stats.firms));
        hline();

        // Wealth
        std::cout << "  " << Styled("WEALTH", Theme::Warning) << "\n";
        kv("Total money supply", "Tk " + fmtD(stats.moneySupply));
        bln();
    }

    // ── PASS DAY  ────────────────────────────────────────────────────────
    void cmdPassDay(const Command &cmd)
    {
        int n = getParam<int>(cmd, "n", 1);
        if (n < 1)
            n = 1;
        if (n > 365)
        {
            output(Styled("[✗]", Theme::Error) + " Max 365 days at once");
            return;
        }

        if (n > 1)
        {
            double gdpBefore = simulation.currentStats.gdp;
            std::map<std::string, double> pricesBefore;
            for (auto &m : simulation.markets)
                pricesBefore[m.prod->name] = m.price;

            std::cout << "\n"
                      << Styled("  ◆ SIMULATING " + std::to_string(n) + " DAYS", Theme::BoldPrimary)
                      << " " << std::flush;

            for (int i = 0; i < n; i++)
            {
                simulation.pass_day();
                std::cout << Styled(".", Theme::Primary) << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(80));
            }
            std::cout << "\n\n";

            std::cout << "  " << Styled("AFTER " + std::to_string(n) + " DAYS:", Theme::Warning) << "\n";
            hline();

            double gdpAfter = simulation.currentStats.gdp;
            std::cout << "    " << Styled(padStr("GDP", 20), Theme::Info)
                      << Styled("Tk " + fmtD(gdpBefore), Theme::Muted)
                      << Styled("  →  ", Theme::Info)
                      << Styled("Tk " + fmtD(gdpAfter), Theme::Highlight) << "\n";

            for (auto &m : simulation.markets)
            {
                auto pit = pricesBefore.find(m.prod->name);
                double prevPrice = (pit != pricesBefore.end()) ? pit->second : 0.0;
                if (m.price < 0.1 && prevPrice < 0.1)
                    continue;
                double diff = m.price - prevPrice;
                std::string diffStr = diff > 0.005 ? Styled("  ▲ +" + fmtD(diff), Theme::Error) : diff < -0.005 ? Styled("  ▼ " + fmtD(diff), Theme::Success)
                                                                                                                : Styled("  ─ stable", Theme::Muted);
                std::cout << "    " << Styled(padStr(m.prod->name + " price", 20), Theme::Info)
                          << Styled("Tk " + fmtD(prevPrice), Theme::Muted)
                          << Styled("  →  ", Theme::Info)
                          << Styled("Tk " + fmtD(m.price), Theme::Highlight)
                          << diffStr << "\n";
            }
            hline();
            noteText("Run market_history to see trends   |   status for macro view");
            bln();
            return;
        }

        // n == 1 → full animated pass
        using namespace styledTerminal;

        auto pause = [](int ms)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        };

        auto dots = [&pause](int count, int delayMs)
        {
            for (int i = 0; i < count; i++)
            {
                std::cout << Styled(".", Theme::Primary) << std::flush;
                pause(delayMs);
            }
        };

        auto pad = [](const std::string &s, int width) -> std::string
        {
            if ((int)s.size() >= width)
                return s.substr(0, width);
            return s + std::string(width - (int)s.size(), ' ');
        };

        auto fmt = [](double v) -> std::string
        {
            std::ostringstream os;
            os << std::fixed << std::setprecision(2) << v;
            return os.str();
        };

        auto showChange = [&](double before, double after, const std::string &unit = "") -> std::string
        {
            double diff = after - before;
            std::ostringstream os;
            os << std::fixed << std::setprecision(2);
            std::string arrow;
            if (diff > 0.005)
            {
                os << "▲ +" << diff << unit;
                arrow = Styled(os.str(), Theme::Success);
            }
            else if (diff < -0.005)
            {
                os << "▼ " << diff << unit;
                arrow = Styled(os.str(), Theme::Error);
            }
            else
            {
                arrow = Styled("─  (unchanged)", Theme::Muted);
            }
            std::ostringstream full;
            full << std::fixed << std::setprecision(2)
                 << Styled(fmt(before), Theme::Muted)
                 << Styled(" → ", Theme::Info)
                 << Styled(fmt(after), Theme::Highlight)
                 << "  " << arrow;
            return full.str();
        };

        auto phaseHeader = [&](const std::string &title)
        {
            pause(350);
            std::cout << "\n"
                      << Styled("  ◆ " + title, Theme::Info) << "\n"
                      << "  " << Styled(Repeat(Horizontal, sw - 4), Theme::Muted) << "\n";
            pause(100);
        };

        auto row = [&](const std::string &label, double before, double after,
                       const std::string &unit = "")
        {
            std::cout << "    "
                      << Styled(pad(label, 22), Theme::Info)
                      << showChange(before, after, unit)
                      << "\n";
            pause(60);
        };

        auto entityLabel = [&](const std::string &name)
        {
            std::cout << "\n  " << Styled("  → " + name, Theme::Warning) << "\n";
            pause(80);
        };

        // ── SNAPSHOTS BEFORE pass_day() ───────────────────────────────────

        struct MktSnap
        {
            std::string name;
            double price;
            double qd;
            double qs;
        };
        struct ConsSnap
        {
            std::string name;
            double savings;
            double expenses;
            double income;
        };
        struct FarmSnap
        {
            std::string name;
            double savings;
            double weather;
            double tax;
            std::vector<std::pair<std::string, double>> cropMax;
        };
        struct LabSnap
        {
            std::string name;
            double savings;
            double income;
        };
        struct FirmSnap
        {
            int ownerId;
            double output;
            double tc;
            double mc;
            double ac;
            int workers;
            int capital;
            double wage;
        };

        std::vector<MktSnap> mktSnap;
        for (auto &m : simulation.markets)
        {
            mktSnap.push_back({m.prod->name, m.price,
                               m.getQuantityDemanded(m.price), m.getQuantitySupplied(m.price)});
        }

        std::vector<ConsSnap> consSnap;
        for (auto &c : simulation.consumers)
            consSnap.push_back({c.name, c.savings, c.expenses, c.incomePerDay});

        // crops vector stores copies — look up maxOutput by name via the ss map keys
        auto getMaxOutput = [](const farmer &f, const std::string &cropName) -> double
        {
            for (auto &[ptr, val] : f.maxOutput)
                if (ptr && ptr->name == cropName)
                    return val;
            return 0.0;
        };

        std::vector<FarmSnap> farmSnap;
        for (auto &f : simulation.farmers)
        {
            FarmSnap s;
            s.name = f.name;
            s.savings = f.savings;
            s.weather = f.weather;
            s.tax = f.tax;
            for (auto &crop : f.crops)
                s.cropMax.push_back({crop.name, getMaxOutput(f, crop.name)});
            farmSnap.push_back(s);
        }

        std::vector<LabSnap> labSnap;
        for (auto &l : simulation.laborers)
            labSnap.push_back({l.name, l.savings, l.incomePerDay});

        std::vector<FirmSnap> firmSnap;
        for (auto &f : simulation.firms)
        {
            const_cast<firm &>(f).calculateCosts();
            firmSnap.push_back({f.ownerId,
                                f.currentOutput, f.totalCost, f.marginalCost, f.averageCost,
                                (int)f.workers.size(), (int)f.capitals.size(), f.wage});
        }

        double gdpBefore = simulation.currentStats.gdp;
        double moneyBefore = simulation.currentStats.moneySupply;
        double unempBefore = simulation.currentStats.unemployment;

        // ── INTRO BANNER ──────────────────────────────────────────────────

        std::cout << "\n"
                  << Styled(Repeat(Horizontal, 3) +
                                " ADVANCING ONE DAY " +
                                Repeat(Horizontal, sw - 22),
                            Theme::Primary)
                  << "\n"
                  << "  " << Styled("Simulating world dynamics", Theme::Warning) << std::flush;
        dots(8, 220);
        std::cout << "\n"
                  << Styled(Separator(sw - 2), Theme::Primary) << "\n";

        // ── RUN SIMULATION ────────────────────────────────────────────────
        simulation.pass_day();

        // ── PHASE 1: MARKETS ──────────────────────────────────────────────
        phaseHeader("PHASE 1 — MARKETS CLEARING & PRICES ADJUSTING");

        bool anyActiveMarket = false;
        for (size_t i = 0; i < simulation.markets.size() && i < mktSnap.size(); i++)
        {
            auto &m = simulation.markets[i];
            auto &snap = mktSnap[i];
            if (snap.price < 0.01 && m.price < 0.01)
                continue;
            anyActiveMarket = true;

            double qdAfter = m.getQuantityDemanded(m.price);
            double qsAfter = m.getQuantitySupplied(m.price);

            entityLabel(snap.name + " Market");
            row("Price ($)", snap.price, m.price);
            row("Qty Demanded", snap.qd, qdAfter);
            row("Qty Supplied", snap.qs, qsAfter);
            row("Excess Demand", snap.qd - snap.qs, qdAfter - qsAfter);
        }
        if (!anyActiveMarket)
            std::cout << "    " << Styled("No active markets yet.", Theme::Muted) << "\n";

        // ── PHASE 2: CONSUMERS ────────────────────────────────────────────
        phaseHeader("PHASE 2 — CONSUMERS RESPONDING TO PRICES");

        for (size_t i = 0; i < simulation.consumers.size() && i < consSnap.size(); i++)
        {
            auto &c = simulation.consumers[i];
            auto &snap = consSnap[i];
            entityLabel(snap.name);
            row("Savings ($)", snap.savings, c.savings);
            row("Expenses ($)", snap.expenses, c.expenses);
            row("Income/day ($)", snap.income, c.incomePerDay);
        }

        // ── PHASE 3: FARMERS ──────────────────────────────────────────────
        phaseHeader("PHASE 3 — FARMERS UPDATING SUPPLY");

        for (size_t i = 0; i < simulation.farmers.size() && i < farmSnap.size(); i++)
        {
            auto &f = simulation.farmers[i];
            auto &snap = farmSnap[i];
            entityLabel(snap.name + "  (Tech: " + fmt(f.techLevel * 100) + "%)");
            row("Savings ($)", snap.savings, f.savings);
            row("Weather index", snap.weather, f.weather);
            row("Tax rate", snap.tax, f.tax);

            for (size_t ci = 0; ci < snap.cropMax.size(); ci++)
            {
                const std::string &cropName = snap.cropMax[ci].first;
                double maxAfter = getMaxOutput(f, cropName);
                row("  " + cropName + " max output",
                    snap.cropMax[ci].second, maxAfter, " units");
            }
        }

        // ── PHASE 4: LABORERS ─────────────────────────────────────────────
        phaseHeader("PHASE 4 — LABORERS & WAGES");

        for (size_t i = 0; i < simulation.laborers.size() && i < labSnap.size(); i++)
        {
            auto &l = simulation.laborers[i];
            auto &snap = labSnap[i];
            entityLabel(snap.name + "  (Skill: " + fmt(l.skillLevel * 100) + "%)");
            row("Savings ($)", snap.savings, l.savings);
            row("Income/day ($)", snap.income, l.incomePerDay);
        }

        // ── PHASE 5: FIRMS ────────────────────────────────────────────────
        phaseHeader("PHASE 5 — FIRMS OPTIMIZING PRODUCTION");

        for (size_t i = 0; i < simulation.firms.size() && i < firmSnap.size(); i++)
        {
            auto &f = simulation.firms[i];
            auto &snap = firmSnap[i];
            std::string ownerName = "Owner #" + std::to_string(snap.ownerId);
            for (auto &c : simulation.consumers)
                if (c.id == snap.ownerId)
                {
                    ownerName = c.name + "'s Firm";
                    break;
                }

            entityLabel(ownerName + "  [L=" + std::to_string((int)f.workers.size()) +
                        "  K=" + std::to_string((int)f.capitals.size()) + "]");
            row("Wage ($/worker)", snap.wage, f.wage);
            row("Output (Q)", snap.output, f.currentOutput, " units");
            row("Total Cost (TC) ($)", snap.tc, f.totalCost);
            row("Avg Cost  (AC) ($)", snap.ac, f.averageCost);
            row("Marginal Cost  ($)", snap.mc, f.marginalCost);

            auto ratios = f.marginalCosts();
            double diff = ratios[0] - ratios[1];
            if (std::abs(diff) < 0.05)
                std::cout << "    " << Styled("  ✓ Optimal input mix", Theme::Success) << "\n";
            else if (diff > 0)
                std::cout << "    " << Styled("  ! Hire more labor (MPL/w > MPK/r)", Theme::Warning) << "\n";
            else
                std::cout << "    " << Styled("  ! Add more capital (MPK/r > MPL/w)", Theme::Warning) << "\n";
            pause(80);
        }

        // ── PHASE 6: MACROECONOMICS ───────────────────────────────────────
        phaseHeader("PHASE 6 — MACROECONOMIC INDICATORS");
        pause(200);

        auto &st = simulation.currentStats;
        row("GDP ($)", gdpBefore, st.gdp);
        row("Money Supply ($)", moneyBefore, st.moneySupply);
        row("Unemployment rate", unempBefore, st.unemployment);

        // ── SUMMARY BANNER ────────────────────────────────────────────────
        pause(400);
        std::cout << "\n"
                  << Styled(Separator(sw - 2), Theme::Primary) << "\n"
                  << Styled("  ✦ DAY COMPLETE", Theme::BoldPrimary)
                  << Styled("  — all " +
                                std::to_string(simulation.consumers.size() +
                                               simulation.farmers.size() +
                                               simulation.laborers.size()) +
                                " entities updated  |  " +
                                std::to_string(simulation.markets.size()) + " markets cleared  |  " +
                                std::to_string(simulation.firms.size()) + " firms optimized",
                            Theme::Muted)
                  << "\n"
                  << Styled(Separator(sw - 2), Theme::Primary) << "\n\n";
    }

    void cmdClear()
    {
        output("Screen cleared");
    }

    // ── Helper functions ───────────────────────────────────────────────────

    market *getMarketFromCmd(const Command &cmd)
    {
        if (cmd.params.find("product") != cmd.params.end())
        {
            std::string name = getParam<std::string>(cmd, "product", std::string());
            if (name.empty())
                return simulation.selected_market;
            for (auto &m : simulation.markets)
                if (m.prod->name == name)
                    return &m;
        }
        return simulation.selected_market;
    }

    product *getProductByName(const std::string &name)
    {
        std::vector<product *> prods = {&rice, &cloth, &computer, &phone, &car, &steel, &potato, &banana, &corn, &jute};
        for (auto *p : prods)
            if (p->name == name)
                return p;
        return nullptr;
    }

    market *getMarketByProduct(product *p)
    {
        for (auto &m : simulation.markets)
            if (m.prod == p)
                return &m;
        return nullptr;
    }
};