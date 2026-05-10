#include "rnd.h"

#include <chrono>
#include <iostream>
#include <limits>
#include <thread>

const bool ABOVE_ZERO = true;
const bool ANY_INTEGER = false;

void promptInt(int* numInput, bool aboveZero)
{	
	while (true)
	{
		if (std::cin >> *numInput) {
			if (!aboveZero) {
				break;
			}
			
			if (!(*numInput <= 0)) {
				break;
			}
		}
		
		std::cin.clear();
		
		std::cin.ignore(
			std::numeric_limits<std::streamsize>::max(),
			'\n' // This is a spot where \n should be used instead of std::endl so I kept it.
		);
		
		if (aboveZero) {
			std::cout << "Invalid input. Enter a number greater than zero: ";
			continue;
		}
		
		std::cout << "Invalid input. Enter a number: ";
	}
}

void promptBool(bool* result)
{
	char inputCharacter;
	bool resultSet = false;
	
	while (true)
    {
		std::cin >> inputCharacter;

		switch ((char) tolower(inputCharacter))
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
			std::cout << "Invalid input, try again (y/n): " << std::endl;
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
