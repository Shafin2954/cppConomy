#pragma once
#include "laborer.h"
#include "capital.h"
#include "consumer.h"
#include <iostream>
#include <cmath>

struct productionFunction
{
    double tech;
    virtual double output(double L, double K) = 0; // pure virtual function makes this an abstract class
    virtual ~productionFunction() = default;
};

struct cobbDouglas : productionFunction
{
    double alpha, beta, tech;
    cobbDouglas(double alpha, double beta, double tech = 1.0) : alpha(alpha), beta(beta), tech(tech) {}
    double output(double L, double K) override
    {
        return pow(L, alpha) * pow(K, beta) * tech;
    }
};

struct ces : productionFunction
{
    double rho;
    ces(double rho) : rho(rho) {}
    double output(double L, double K) override
    {
        return pow(pow(L, rho) + pow(K, rho), 1.0 / rho);
    }
};

class firm
{
public:
    enum class ProdType
    {
        CobbDouglas,
        Ces
    };

    double cash; // how much money they have to invest

    int ownerId; // for simplicity, each firm has one owner (a consumer)
    std::vector<laborer> workers;
    std::vector<capital> capitals;

    std::vector<product> products; // goods produced (n)

    double wage;
    double fixed_overhead;

    double totalFixedCost;      // TFC
    double totalVariableCost;   // TVC
    double totalCost;           // TC
    double averageFixedCost;    // AFC
    double averageVariableCost; // AVC
    double averageCost;         // AC
    double marginalCost;        // MC
    double currentOutput;       // Q

    cobbDouglas cdProd;
    ces cesProd;
    productionFunction *prodFunc;
    ProdType prodType;

    firm(int id, double cash, cobbDouglas cd)
        : cash(cash), ownerId(id), wage(0.0), fixed_overhead(0.0),
          totalFixedCost(0.0), totalVariableCost(0.0), totalCost(0.0),
          averageFixedCost(0.0), averageVariableCost(0.0), averageCost(0.0),
          marginalCost(0.0), currentOutput(0.0),
          cdProd(cd), cesProd(0), prodFunc(&cdProd), prodType(ProdType::CobbDouglas)
    {
    }
    firm(int id, double cash, ces c)
        : cash(cash), ownerId(id), wage(0.0), fixed_overhead(0.0),
          totalFixedCost(0.0), totalVariableCost(0.0), totalCost(0.0),
          averageFixedCost(0.0), averageVariableCost(0.0), averageCost(0.0),
          marginalCost(0.0), currentOutput(0.0),
          cdProd(0, 0), cesProd(c), prodFunc(&cesProd), prodType(ProdType::Ces)
    {
    }

    firm(const firm &other)
        : cash(other.cash),
          ownerId(other.ownerId),
          workers(other.workers),
          capitals(other.capitals),
          products(other.products),
          wage(other.wage),
          fixed_overhead(other.fixed_overhead),
          totalFixedCost(other.totalFixedCost),
          totalVariableCost(other.totalVariableCost),
          totalCost(other.totalCost),
          averageFixedCost(other.averageFixedCost),
          averageVariableCost(other.averageVariableCost),
          averageCost(other.averageCost),
          marginalCost(other.marginalCost),
          currentOutput(other.currentOutput),
          cdProd(other.cdProd),
          cesProd(other.cesProd),
          prodFunc(nullptr),
          prodType(other.prodType)
    {
        bindProdFunc();
    }

    firm &operator=(const firm &other)
    {
        if (this == &other)
            return *this;
        cash = other.cash;
        ownerId = other.ownerId;
        workers = other.workers;
        capitals = other.capitals;
        products = other.products;
        wage = other.wage;
        fixed_overhead = other.fixed_overhead;
        totalFixedCost = other.totalFixedCost;
        totalVariableCost = other.totalVariableCost;
        totalCost = other.totalCost;
        averageFixedCost = other.averageFixedCost;
        averageVariableCost = other.averageVariableCost;
        averageCost = other.averageCost;
        marginalCost = other.marginalCost;
        currentOutput = other.currentOutput;
        cdProd = other.cdProd;
        cesProd = other.cesProd;
        prodType = other.prodType;
        bindProdFunc();
        return *this;
    }

    firm(firm &&other) noexcept
        : cash(other.cash),
          ownerId(other.ownerId),
          workers(std::move(other.workers)),
          capitals(std::move(other.capitals)),
          products(std::move(other.products)),
          wage(other.wage),
          fixed_overhead(other.fixed_overhead),
          totalFixedCost(other.totalFixedCost),
          totalVariableCost(other.totalVariableCost),
          totalCost(other.totalCost),
          averageFixedCost(other.averageFixedCost),
          averageVariableCost(other.averageVariableCost),
          averageCost(other.averageCost),
          marginalCost(other.marginalCost),
          currentOutput(other.currentOutput),
          cdProd(other.cdProd),
          cesProd(other.cesProd),
          prodFunc(nullptr),
          prodType(other.prodType)
    {
        bindProdFunc();
    }

    firm &operator=(firm &&other) noexcept
    {
        if (this == &other)
            return *this;
        cash = other.cash;
        ownerId = other.ownerId;
        workers = std::move(other.workers);
        capitals = std::move(other.capitals);
        products = std::move(other.products);
        wage = other.wage;
        fixed_overhead = other.fixed_overhead;
        totalFixedCost = other.totalFixedCost;
        totalVariableCost = other.totalVariableCost;
        totalCost = other.totalCost;
        averageFixedCost = other.averageFixedCost;
        averageVariableCost = other.averageVariableCost;
        averageCost = other.averageCost;
        marginalCost = other.marginalCost;
        currentOutput = other.currentOutput;
        cdProd = other.cdProd;
        cesProd = other.cesProd;
        prodType = other.prodType;
        bindProdFunc();
        return *this;
    }

    void bindProdFunc()
    {
        prodFunc = (prodType == ProdType::CobbDouglas) ? static_cast<productionFunction *>(&cdProd)
                                                       : static_cast<productionFunction *>(&cesProd);
    }

    double getCapitalCost()
    {
        double capitalCost = 0.0;
        for (const auto &cap : capitals)
        {
            capitalCost += cap.rentalRate;
        }
        return capitalCost;
    }

    double MPofLabor()
    {
        int L = workers.size();
        int K = capitals.size();
        double currentQ = prodFunc->output(L, K);
        double newQ = prodFunc->output(L + 1, K);
        return newQ - currentQ;
    }

    // "How much does adding 1 more machine help me right now?"
    double MPofCapital()
    {
        int L = workers.size();
        int K = capitals.size();
        double currentQ = prodFunc->output(L, K);
        double newQ = prodFunc->output(L, K + 1);
        return newQ - currentQ;
    }

    // 3. THE OPTIMIZER (Finding the Tangency Point)
    // This function runs every turn to re-balance the firm.
    std::vector<double> marginalCosts()
    {
        double mpL = MPofLabor();
        double mpK = MPofCapital();

        // Calculate "Bang for Buck" (Marginal Product per Dollar)
        double laborRatio = mpL / wage;
        double capitalRatio = mpK / getCapitalCost();

        // cout << "Efficiency Check:\n";
        // cout << "Labor  (MPL/w): " << laborRatio << endl;
        // cout << "Capital(MPK/r): " << capitalRatio << endl;

        // // DECISION LOGIC
        // double tolerance = 0.05;

        // if (abs(laborRatio - capitalRatio) < tolerance) {
        //     cout << "STATUS: Optimal Mix (Isoquant Tangent to Isocost)" << endl;
        // }
        // else if (laborRatio > capitalRatio) {
        //     cout << "ACTION: Labor is cheaper relative to output. HIRE WORKER." << endl;
        //     // logic to call market.hireWorker()
        // }
        // else {
        //     cout << "ACTION: Capital is cheaper relative to output. BUY MACHINE." << endl;
        //     // logic to buy machine
        // }
        return {laborRatio, capitalRatio};
    }

    void calculateCosts()
    {
        double L = workers.size();
        double K = capitals.size();

        // Q
        currentOutput = prodFunc->output(L, K);

        // TFC = Overhead (Building) + Machine Rentals
        totalFixedCost = fixed_overhead + getCapitalCost();

        // TVC = Workers * Wage
        totalVariableCost = L * wage;

        // TC
        totalCost = totalFixedCost + totalVariableCost;

        // C. Averages (Watch out for divide by zero!)
        if (currentOutput > 0.0001)
        {
            averageFixedCost = totalFixedCost / currentOutput;       // TFC/Q (lessens as output increases)
            averageVariableCost = totalVariableCost / currentOutput; // TVC/Q
            averageCost = totalCost / currentOutput;                 // AC = TC/Q (U-shaped curve due to fixed costs spreading out and variable costs eventually dominating)
        }
        else
        {
            // Infinite costs if producing nothing but paying rent
            averageFixedCost = std::numeric_limits<double>::infinity();
            averageVariableCost = 0;
            averageCost = std::numeric_limits<double>::infinity();
        }

        // D. Marginal Cost (The Economic Shortcut)
        // MC = Price of Input / Marginal Product of Input
        // We use Labor for this calculation as it's the primary variable input
        double mpL = MPofLabor();

        if (mpL > 0.0001)
        {
            marginalCost = wage / mpL; // MC same in optimized state
        }
        else
        {
            // If adding a worker produces 0 output, cost is infinite
            marginalCost = std::numeric_limits<double>::infinity();
        }
    }

    std::string getStyledDetails() const
    {
        using namespace styledTerminal;
        std::stringstream ss;

        ss << Header("FIRM (Owner ID: " + std::to_string(ownerId) + ")") << "\n";
        ss << KeyValue("Cash", "$" + std::to_string(twoDecimal(cash))) << "\n";
        ss << KeyValue("Workers", std::to_string(workers.size())) << "\n";
        ss << KeyValue("Capital Units", std::to_string(capitals.size())) << "\n";
        ss << KeyValue("Wage Rate", "$" + std::to_string(twoDecimal(wage))) << "\n\n";

        ss << Styled("PRODUCTION:\n", Theme::Primary);
        ss << KeyValue("Current Output", std::to_string(twoDecimal(currentOutput))) << "\n";
        ss << KeyValue("Total Cost", "$" + std::to_string(twoDecimal(totalCost))) << "\n";
        ss << KeyValue("Average Cost", "$" + std::to_string(twoDecimal(averageCost))) << "\n";
        ss << KeyValue("Marginal Cost", "$" + std::to_string(twoDecimal(marginalCost))) << "\n\n";

        auto ratios = const_cast<firm *>(this)->marginalCosts();
        ss << Styled("EFFICIENCY:\n", Theme::Info);
        ss << "  MPL/w: " << twoDecimal(ratios[0]) << "\n";
        ss << "  MPK/r: " << twoDecimal(ratios[1]) << "\n";

        if (std::abs(ratios[0] - ratios[1]) < 0.05)
            ss << Styled("  Status: Optimal", Theme::Success) << "\n";
        else if (ratios[0] > ratios[1])
            ss << Styled("  Recommendation: Hire more labor", Theme::Warning) << "\n";
        else
            ss << Styled("  Recommendation: Add more capital", Theme::Warning) << "\n";

        return ss.str();
    }
};