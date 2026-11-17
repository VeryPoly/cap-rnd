#include "rndCli.h"

#include <iostream>

int main()
{
    std::ios::sync_with_stdio(false);

    while (true)
    {
        auto config = promptConfig();
        if (!config)
            return 0;

        if (runSession(*config) != RunResult::Completed)
            return 1;

        std::cout << "Run another simulation? (y/n): ";

        bool continueProgram = false;
        promptBool(continueProgram);

        if (!continueProgram)
            break;
    }

    return 0;
}
