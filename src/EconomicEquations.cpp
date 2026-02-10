#include "EconomicEquations.h"
#include <sstream>
#include <iomanip>
using namespace std;

// ========== VariableChange::ToString ==========
string EconomicEquations::VariableChange::ToString() const
{
    ostringstream oss;
    oss << fixed << setprecision(2);

    oss << "📊 " << variableName << "\n";
    oss << "   " << oldValue << " → " << newValue;

    if (percentChange > 0)
        oss << " (↑ +" << percentChange << "%)";
    else if (percentChange < 0)
        oss << " (↓ " << percentChange << "%)";
    else
        oss << " (→ No change)";

    oss << "\n";

    if (!description.empty())
    {
        oss << "   Description: " << description << "\n";
    }

    if (!affectedVariables.empty())
    {
        oss << "   Affects: ";
        for (size_t i = 0; i < affectedVariables.size(); i++)
        {
            if (i > 0)
                oss << ", ";
            oss << affectedVariables[i];
        }
        oss << "\n";
    }

    return oss.str();
}

// ========== CostAnalysis::ToString ==========
string EconomicEquations::CostAnalysis::ToString(double price) const
{
    ostringstream oss;
    oss << fixed << setprecision(2);

    oss << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    oss << "         Cost Analysis (Q = " << quantity << ")\n";
    oss << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    oss << left << setw(30) << "Total Fixed Cost (TFC)" << ": $" << totalFixedCost << "\n";
    oss << left << setw(30) << "Total Variable Cost (TVC)" << ": $" << totalVariableCost << "\n";
    oss << left << setw(30) << "Total Cost (TC)" << ": $" << totalCost << "\n";
    oss << "─────────────────────────────────────────────\n";
    oss << left << setw(30) << "AFC (Fixed Cost / Q)" << ": $" << (quantity > 0 ? totalFixedCost / quantity : 0) << "\n";
    oss << left << setw(30) << "AVC (Variable Cost / Q)" << ": $" << averageVariableCost << "\n";
    oss << left << setw(30) << "ATC (Total Cost / Q)" << ": $" << averageTotalCost << "\n";
    oss << "─────────────────────────────────────────────\n";

    if (price > 0)
    {
        double profit = (price * quantity) - totalCost;
        double profitMargin = (profit / (price * quantity)) * 100;

        oss << left << setw(30) << "Price" << ": $" << price << "\n";
        oss << left << setw(30) << "Revenue (P × Q)" << ": $" << (price * quantity) << "\n";
        oss << left << setw(30) << "Profit" << ": $" << profit;

        if (profit > 0)
            oss << " ✓ (Profitable)";
        else if (profit < 0)
            oss << " ✗ (Loss-making)";

        oss << "\n";
        oss << left << setw(30) << "Profit Margin" << ": " << profitMargin << "%\n";

        // Shutdown condition: if P < AVC, firm should shut down
        oss << "─────────────────────────────────────────────\n";
        if (price < averageVariableCost)
        {
            oss << "⚠️  SHUTDOWN CONDITION MET!\n";
            oss << "Price ($" << price << ") < AVC ($" << averageVariableCost << ")\n";
            oss << "Firm is losing money on every unit. Should shut down.\n";
        }
        else if (price < averageTotalCost)
        {
            oss << "⚠️  BREAKING EVEN (Short Run)\n";
            oss << "Price ($" << price << ") > AVC but < ATC\n";
            oss << "Firm is covering variable costs. Can operate in short run.\n";
        }
        else
        {
            oss << "✓ PROFITABLE\n";
            oss << "Price ($" << price << ") > ATC ($" << averageTotalCost << ")\n";
            oss << "Firm is making economic profit.\n";
        }
    }

    oss << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";

    return oss.str();
}
