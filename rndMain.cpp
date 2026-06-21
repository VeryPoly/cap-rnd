#include "rnd.h"

#include <cctype>
#include <chrono>
#include <iostream>
#include <limits>
#include <thread>

const bool ABOVE_ZERO = true;
const bool ANY_INTEGER = false;

void promptInt(int* numInput, bool aboveZero)
{
    if (numInput == nullptr) {
        return;
    }

    while (true)
    {
        if (std::cin >> *numInput) {
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(),
                '\n');

            if (!aboveZero || *numInput > 0) {
                break;
            }
        }
        else
        {
            if (std::cin.eof()) {
                return;
            }

            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(),
                '\n');
        }

        if (aboveZero) {
            std::cout << "Invalid input. Enter a number greater than zero: ";
        } else {
            std::cout << "Invalid input. Enter a number: ";
        }
    }
}

void promptBool(bool* result)
{
    if (result == nullptr) {
        return;
    }

    char inputCharacter;
    bool resultSet = false;

    while (true)
    {
        if (!(std::cin >> inputCharacter)) {
            if (std::cin.eof()) {
                *result = false;
                return;
            }

            std::cin.clear();
            std::cin.ignore(
                std::numeric_limits<std::streamsize>::max(),
                '\n');
            std::cout << "Invalid input, try again (y/n): ";
            continue;
        }

        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(),
            '\n');

        switch (static_cast<char>(std::tolower(static_cast<unsigned char>(inputCharacter))))
        {
            case 'y':
                *result = true;
                resultSet = true;
                break;
            case 'n':
                *result = false;
                resultSet = true;
                break;
        }

        if (!resultSet) {
            std::cout << "Invalid input, try again (y/n): ";
            continue;
        }

        break;
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

		bool endChoice;

        std::cout << "Prior: ";
        promptInt(&prior, ANY_INTEGER);

        std::cout << std::endl << "Cost Offset: ";
		promptInt(&costOffset, ANY_INTEGER);

        std::cout << std::endl << "Number of Rolls: ";
		promptInt(&numRolls, ABOVE_ZERO);

		std::cout << std::endl << "Keep Going if failed? (y/n): ";
        promptBool(&keepGoing);

        std::cout << std::endl;

        rnd simulator(prior, costOffset, keepGoing);

        int completed = 0;
        while (completed < numRolls)
        {
            int safetyCounter = 1000;

            while (!simulator.roll() && --safetyCounter > 0)
				continue;

            if (safetyCounter <= 0)
            {
                std::cout << "Safety limit reached.\n";
                break;
            }

            std::cout << std::endl;

            bool retry = simulator.cost();

            simulator.reset();
            simulator.incPrior();

            if (!retry)
                ++completed;
        }

        std::cout
            << "Grand total cost across all iterations: "
            << simulator.getGrandTotalCost()
            << std::endl << std::endl;

        std::cout
            << "All completed." << std::endl
            << "Wish to do more rolls? (y/n):" << std::endl;
        promptBool(&endChoice);

        std::cout << std::endl << std::endl;

        if (!endChoice)
            break;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
