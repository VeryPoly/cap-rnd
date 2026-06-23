#include "rnd.h"

#include <chrono>
#include <cctype>
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
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(),
                '\n');

            if (!aboveZero || *value > 0)
                return;
        }
        else
        {
            if (std::cin.eof())
                return;

            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(),
                '\n');
        }

        std::cout
            << (aboveZero
                    ? "Enter a number greater than zero: "
                    : "Enter a valid number: ");
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
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(),
                '\n');

            continue;
        }

        c = static_cast<char>(
            std::tolower(
                static_cast<unsigned char>(c)));

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

        std::cout
            << "Enter y or n: ";
    }
}

int main()
{
    while (true)
    {
        int prior;
        int costOffset;
        int numRolls;

        bool keepGoing;
        bool continueProgram;

        std::cout << "Prior: ";
        promptInt(&prior, ANY_INTEGER);

        std::cout << "Cost Offset: ";
        promptInt(&costOffset, ANY_INTEGER);

        std::cout << "Number of Rolls: ";
        promptInt(&numRolls, ABOVE_ZERO);

        std::cout
            << "Keep Going if failed? (y/n): ";
        promptBool(&keepGoing);

        rnd simulator(
            prior,
            costOffset,
            keepGoing);

        int completed = 0;

        while (completed < numRolls)
        {
            constexpr int MAX_RETRIES = 100;
            int retryCount = 0;

            bool retry;

            do
            {
                int safetyCounter = 1000;

                while (!simulator.roll()
                       && --safetyCounter > 0)
                {
                }

                if (safetyCounter <= 0)
                {
                    std::cerr
                        << "Safety limit exceeded.\n";
                    return 1;
                }

                retry = simulator.cost();

                simulator.reset();

                if (retry)
                {
                    ++retryCount;

                    if (retryCount >= MAX_RETRIES)
                    {
                        std::cerr
                            << "Retry limit exceeded.\n";
                        return 1;
                    }
                }

            } while (retry);

            simulator.incPrior();
            ++completed;
        }

        std::cout
            << "Grand total cost across all iterations: "
            << simulator.getGrandTotalCost()
            << "\n\n";

        std::cout
            << "Run another simulation? (y/n): ";

        promptBool(&continueProgram);

        if (!continueProgram)
            break;

        std::this_thread::sleep_for(
            std::chrono::seconds(1));
    }

    return 0;
}