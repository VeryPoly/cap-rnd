#include "rndCli.h"

#include <iostream>

int main()
{
    std::ios::sync_with_stdio(false);

    while (true)
    {
        SimConfig config = promptConfig();

        if (!runSession(config))
            return 1;

        std::cout << "Run another simulation? (y/n): ";

        bool continueProgram = false;
        promptBool(continueProgram);

        if (!continueProgram)
            break;
    }

    return 0;
}
