#include "rnd.h"

#include <cctype>
#include <chrono>
#include <iostream>
#include <limits>
#include <thread>

constexpr bool ABOVE_ZERO = true;
constexpr bool ANY_INTEGER = false;

void promptInt(int* value, bool aboveZero)
{
    while (true)
    {
        if (std::cin >> *value)
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (!aboveZero || *value > 0)
                return;
        }
        else
        {
            if (std::cin.eof())
                return;

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        std::cout << (aboveZero ? "Enter a number greater than zero: " : "Enter a valid number: ");
    }
}

void promptBool(bool* result)
{
    while (true)
    {
        char c;

        if (!(std::cin >> c))
        {
            if (std::cin.eof())
            {
                *result = false;
                return;
            }

            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            continue;
        }

        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        if (c == 'y')
        {
            *result = true;
            return;
        }

        if (c == 'n')
        {
            *result = false;
            return;
        }

        std::cout << "Enter y or n: ";
    }
}

struct SimConfig
{
    int prior;
    int costOffset;
    int numRolls;
    bool keepGoing;
};

SimConfig promptConfig()
{
    SimConfig config;

    std::cout << "Prior: ";
    promptInt(&config.prior, ANY_INTEGER);

    std::cout << "Cost Offset: ";
    promptInt(&config.costOffset, ANY_INTEGER);

    std::cout << "Number of Rolls: ";
    promptInt(&config.numRolls, ABOVE_ZERO);

    std::cout << "Keep Going if failed? (y/n): ";
    promptBool(&config.keepGoing);

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

        default:
            std::cout << " failed.\n\n";
            break;
    }

    return outcome;
}

bool runIteration(rnd& simulator)
{
    constexpr int MAX_RETRIES = 100;
    int retryCount = 0;

    bool retry;

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

int main()
{
    while (true)
    {
        SimConfig config = promptConfig();

        if (!runSession(config))
            return 1;

        std::cout << "Run another simulation? (y/n): ";

        bool continueProgram;
        promptBool(&continueProgram);

        if (!continueProgram)
            break;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
