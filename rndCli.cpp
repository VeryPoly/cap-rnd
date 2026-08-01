#include "rndCli.h"

#include <cctype>
#include <iostream>
#include <limits>

constexpr bool ABOVE_ZERO = true;
constexpr bool ANY_INTEGER = false;

namespace
{
void discardLine()
{
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void warnIfClamped(int value, int min, int max, const char* label)
{
    if (value < min || value > max)
    {
        std::cout << label << " is clamped to the range [" << min << ", " << max << "].\n";
    }
}
}  // namespace

void promptInt(int& value, bool aboveZero)
{
    while (true)
    {
        if (std::cin >> value)
        {
            discardLine();

            if (!aboveZero || value > 0)
                return;
        }
        else
        {
            if (std::cin.eof())
                return;

            std::cin.clear();
            discardLine();
        }

        std::cout << (aboveZero ? "Enter a number greater than zero: " : "Enter a valid number: ");
    }
}

void promptBool(bool& result)
{
    while (true)
    {
        char c;

        if (!(std::cin >> c))
        {
            if (std::cin.eof())
            {
                result = false;
                return;
            }

            std::cin.clear();
            discardLine();

            continue;
        }

        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        if (c == 'y')
        {
            result = true;
            return;
        }

        if (c == 'n')
        {
            result = false;
            return;
        }

        std::cout << "Enter y or n: ";
    }
}

SimConfig promptConfig()
{
    SimConfig config;

    std::cout << "Prior: ";
    promptInt(config.prior, ANY_INTEGER);
    warnIfClamped(config.prior, rnd::PRIOR_MIN, rnd::PRIOR_MAX, "Prior");

    std::cout << "Cost Offset: ";
    promptInt(config.costOffset, ANY_INTEGER);
    warnIfClamped(config.costOffset, rnd::COST_OFFSET_MIN, rnd::COST_OFFSET_MAX, "Cost Offset");

    std::cout << "Number of Rolls: ";
    promptInt(config.numRolls, ABOVE_ZERO);

    std::cout << "Keep Going if failed? (y/n): ";
    promptBool(config.keepGoing);

    return config;
}

bool runRollSequence(rnd& simulator)
{
    int safetyCounter = 1000;

    for (;;)
    {
        bool complete = simulator.roll();

        std::cout << simulator.getLastRoll() << ' ';

        if (complete)
            return true;

        if (--safetyCounter <= 0)
            return false;
    }
}

rnd::CostOutcome reportCost(rnd& simulator)
{
    rnd::CostOutcome outcome = simulator.cost();

    bool succeeded =
        outcome == rnd::CostOutcome::RetrySuccess || outcome == rnd::CostOutcome::Success;

    if (succeeded)
    {
        std::cout << "\nFaults: " << simulator.getFaults() << ", " << simulator.getFails()
                  << " crit faults\n";
    }

    std::cout << "Total cost: " << simulator.getScaledCost();

    switch (outcome)
    {
        case rnd::CostOutcome::RetrySuccess:
            std::cout << " success, redoing to remove fault.\n\n";
            break;

        case rnd::CostOutcome::Success:
            std::cout << " success.\n\n";
            break;

        case rnd::CostOutcome::RetryFailure:
        case rnd::CostOutcome::GiveUpFailure:
            std::cout << " failed.\n\n";
            break;
    }

    return outcome;
}

bool runIteration(rnd& simulator)
{
    constexpr int MAX_RETRIES = 100;
    int retryCount = 0;

    bool retry = false;

    do
    {
        if (!runRollSequence(simulator))
        {
            std::cerr << "Safety limit exceeded.\n";
            return false;
        }

        rnd::CostOutcome outcome = reportCost(simulator);

        retry =
            outcome == rnd::CostOutcome::RetrySuccess || outcome == rnd::CostOutcome::RetryFailure;

        simulator.reset();

        if (retry)
        {
            ++retryCount;

            if (retryCount >= MAX_RETRIES)
            {
                std::cerr << "Retry limit exceeded.\n";
                return false;
            }
        }

    } while (retry);

    simulator.incPrior();

    return true;
}

bool runSession(const SimConfig& config)
{
    rnd simulator(config.prior, config.costOffset, config.keepGoing);

    int completed = 0;

    while (completed < config.numRolls)
    {
        if (!runIteration(simulator))
            return false;

        ++completed;
    }

    std::cout << "Grand total cost across all iterations: " << simulator.getGrandTotalCost()
              << "\n\n";

    return true;
}
