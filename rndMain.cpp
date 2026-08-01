#include "rndCli.h"

#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    while (true)
    {
        SimConfig config = promptConfig();

        if (!runSession(config))
            return 1;

        std::cout << "Run another simulation? (y/n): ";

        bool continueProgram;
        promptBool(continueProgram);

        if (!continueProgram)
            break;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
